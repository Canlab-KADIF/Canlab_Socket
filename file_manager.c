#include "file_manager.h"

extern TRIGGER_CLIENT T;

extern volatile int running;  
extern char trigger_time[BUFSIZE];
extern char trigger_time_30ago[BUFSIZE];
extern char trigger_time_60ago[BUFSIZE];
extern char meta_name[BUFSIZE];
extern char route_name[BUFSIZE];
extern char thumbnail_name[BUFSIZE];
extern char clip_name[BUFSIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

char *files[MAX_FILES];

int create_directory(char* dest, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    // 타임스탬프 형식 (YYYYMMDD_HHMMSS)
    char timestamp[BUFSIZE];
    snprintf(timestamp, sizeof(timestamp), "%04d%02d%02d_%02d%02d%02d", 
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, 
             t->tm_hour, t->tm_min, t->tm_sec);

    if (snprintf(dest, size, "%s_TRG_%s/", path, timestamp) >= size) {
        log_message("Error: Path is too long.", NULL);
        return -1;
    }

    if (mkdir(dest, S_IFDIR | S_IRWXU | S_IRWXG | S_IXOTH | S_IROTH) == -1) {
        if (errno == EEXIST) {
		log_message("Error: Directory already exists.", NULL);
		perror("mkdir failed");
		return -1;
        }else log_message("Directory created successfully", NULL);
    }

    log_message("Directory created successfully", NULL);
    return 0;
}

int find_files_in_directory(const char *dir_path, const char *ext, char **files, int *count) {
    struct dirent *entry;
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        log_message("Failed to open directory", NULL);
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[BUFSIZE];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            // type이 폴더일 경우 
            find_files_in_directory(full_path, ext, files, count);
        } else {
            // type이 파일일 경우
            if (ext == NULL || strstr(entry->d_name, ext) != NULL) {
                files[*count] = strdup(full_path);
                (*count)++;
                //log_message("Found file: ", NULL);
                //log_message(full_path); 
            }
        }
    }

    // 파일 경로 정렬
    if (count > 0) {
        qsort(files, *count, sizeof(char*), compare_files_by_number);
    }
            
    closedir(dir);
    return 0;
}

void* check_file_count(void *arg) {
    T.Finish = 0;

    while (running) {
    	// 파일 수 확인
        int file_count = 0;
        find_files_in_directory(path, NULL, files, &file_count);
        
        if (file_count < 0) {
            log_message("Error occurred while fetching files.", NULL);
            continue;
        }

        // 파일 이동이 끝났 거나 파일 수가 한계를 초과할 때 전체 파일을 삭제
        if (T.Finish == 1 || file_count >= MAX_FILES) {
            if (T.Finish) {
                log_message(".db3 Move Finish, Deleting all files.", NULL);
            } else {
                log_message("File Count limit exceeded, Deleting all files.", NULL);
            }

            // 전체 파일 삭제
            for (int i = 0; i < file_count; i++) {
                if (remove(files[i]) == 0) {
                    log_message("Deleted file:", files[i]);
                } else {
                    perror("remove failed");
                }
                free(files[i]);
            }

            pthread_mutex_lock(&mutex);
            T.Finish = 0;
            pthread_mutex_unlock(&mutex);
        }
        sleep(1);
    }

    return NULL;
}

void restart_rosbag() {
    int ret = 0;
    
    pthread_mutex_lock(&mutex);
    T.Trigger = 0;
    T.Check = 0;
    T.Count = 1;
    T.Finish = 1;
    pthread_mutex_unlock(&mutex);

    sleep(1);

    ret = run_ros();
    if (ret == -1) {
        log_message("Ros Run Failed", NULL);
        display_banner("DASHBOARD END");
        running = 0;
    }
    
    ret = run_rosbag();
    if (ret == -1) {
        log_message("Rosbag Run Failed", NULL);
        display_banner("DASHBOARD END");
        running = 0;
    }

    if (create_directory(T.MakeDirectoryBuf, sizeof(T.MakeDirectoryBuf)) != 0) {
        log_message("Make Directory Failed", NULL);
        display_banner("DASHBOARD END");
        running = 0;
    }
}


void* move_bag_files(void *arg) {
    char cmd_buffer[BUFSIZE];
    int target_index = 0, start_index = 0, end_index = 0, ret = 0;

    int c_socket = -1;
    pthread_t threads = 0;
    
    char abnormal_discerned_timestamp[MAX_STR];
    char scenario_description[MAX_STR];
    char dynamic_elements[MAX_STR];
    char scenery[MAX_STR];
    char image[MAX_STR];
    char travel_path[MAX_STR];
    char video[MAX_STR];
    char triggered_timestamp[MAX_STR];
    char record_date[MAX_STR];
    char special_vehicles[MAX_STR];
    char special_structures[MAX_STR];

    int abnormal_cause = 0;
    int scenario_causative_object = 0;
    int scenario_id = 0;
    int date_time = 0;
    int driving_mode = 0;
    int illuminance = 0;
    int rainfall = 0;
    int cloudness = 0;
    int snowfall = 0;
    int wind = 0;
    int triggered_cause = 0;
    double duration = 0.0;

    int pedestrian_density = 0;
    int traffic_density = 0;
    int zones = 0;
    int road_types = 0;
    int intersections = 0;
    bool roundabouts = false;
    
    T.Count = 1;
    T.Trigger = 0;
    int backup_count = 0;

    while (running) {
        if (T.Trigger == 1) {
            log_message("Start moving .db3 files", NULL);

	    // 파일 수 확인
            int file_count = 0;
            if (find_files_in_directory(path, ".db3", files, &file_count) < 0) {
                log_message("Error occurred while fetching .db3 files.", NULL);
                continue;
            }
            
            if (file_count < 2) {
                start_index = file_count - 1;
                end_index = file_count - 1;
                log_message("File Count < 2. Selecting target file", NULL);
                T.Count = 0;
                backup_count = file_count;
            }

            if (T.Count == 1) {
                target_index = file_count - 1;
                if (file_count == 2) {
                    start_index = target_index - 1;
                } else {
                    start_index = target_index - 2;
                }
                end_index = target_index;
                log_message("Selecting target file", NULL);
                T.Count = 0;
                backup_count = file_count;
            }

	    // 3개의 파일이 생성 된 후 파일 이동
            if (file_count >= target_index + 2) {
                char bag_path[128];
                char first_bag[128], second_bag[128], third_bag[128];
                const char *trigger_times[] = {trigger_time, trigger_time_30ago, trigger_time_60ago};
                
                for (int i = start_index; i <= end_index; i++) {
                    // backup bagfile
                    snprintf(cmd_buffer, sizeof(cmd_buffer), "cp %s %s", files[i], T.MakeDirectoryBuf);
                    if (execute_command(cmd_buffer) == -1) {
                        log_message("Error copying file.", NULL);
                    } else {
                        log_message("Successfully copied file: ", files[i]);
                    }

                    // change bagfile name
                    snprintf(cmd_buffer, sizeof(cmd_buffer), "mv %s%s %s%s", T.MakeDirectoryBuf, strrchr(files[i], '/') + 1, T.MakeDirectoryBuf, trigger_times[end_index - i]);
                    if (execute_command(cmd_buffer) == -1) {
                        log_message("Error changing file name.", NULL);
                    } else {
                        log_message("Successfully changed file name: ", trigger_times[end_index - i]);
                    }

                    // bagfile index
                    backup_name(bag_path, T.MakeDirectoryBuf, trigger_times[end_index - i]);
                    if ((i - start_index) < 1) {
                        strcpy(first_bag, bag_path);
                    } else if ((i - start_index) == 1) {
                        strcpy(second_bag, bag_path);
                    } else {
                        strcpy(third_bag, bag_path);
                    }

                    // log header - duration
                    snprintf(cmd_buffer, sizeof(cmd_buffer), "ros2 bag info %s | grep Duration: > /tmp/tmp_duration.txt", bag_path);
                    if (execute_command(cmd_buffer) == -1) {
                        log_message("Error executing bag info for duration", NULL);
                    } else {
                        FILE *fp = fopen("/tmp/tmp_duration.txt", "r");
                        if (fp != NULL) {
                            char duration_tmp1[32];
                            char duration_tmp2[32];
                            
                            if (fgets(duration_tmp1, sizeof(duration_tmp1), fp) != NULL) {
                                if (sscanf(duration_tmp1, "Duration: %[0-9.]", duration_tmp2) != 1) {
                                    log_message("Error extracting duration", NULL);
                                    return -1;
                                } else {
                                    duration += atof(duration_tmp2);
                                }
                            } else {
                                log_message("Error fgets", NULL);
                                return -1;
                            }
                        } else {
                            log_message("Error fopen tmp_duration.txt", NULL);
                            return -1;
                        }
                        fclose(fp);
                    }
                }

                ret = kill_rosbag(); //ROS 종료
	        if (ret == -1) {
	            display_banner("DASHBOARD END");
                    running = 0; // 프로그램 종료     
	        }else{
		        while (ret) {
		            snprintf(cmd_buffer, sizeof(cmd_buffer), "cp %s/metadata.yaml %s 2>/dev/null", path, T.MakeDirectoryBuf);
		            if (execute_command(cmd_buffer) == 0) {
		                log_message("Successfully copied file metadata.yaml ", NULL);
		                break;
		            } else {
		                log_message("Error copying file metadata.yaml", NULL);
		            }
		            sleep(1);
		        }
		        
                        //upload_bag_files(); // metadata 업로드

		// triggered cause - triggered timestamp
		strncpy(triggered_timestamp, strrchr(bag_path, '/') + 1, 15);

		// log header - record date
		strncpy(record_date, strrchr(first_bag, '/') + 1, 15);

		// travel_path - latitude, longitude
		ret = gps_extractor(T.MakeDirectoryBuf);

		// scene_context - dynamic_elements
		strcpy(dynamic_elements, route_name);

		// scene_context - scenery
		strcpy(scenery, route_name);

		// screen - image
		strcpy(image, thumbnail_name);

		// screen - travel_path
		strcpy(travel_path, route_name);

		// screen - video
		strcpy(video, clip_name);

		// gui socket
		c_socket = connect_to_server(ADS_IPADDR, GUI_PORT);
		if (c_socket == -1) {
		    cleanup_thread(threads, 0, c_socket);
		    display_banner("DASHBOARD END");
		    return -1;
		}

		receive_data(c_socket,
			 &abnormal_cause,
			 abnormal_discerned_timestamp,
			 &scenario_causative_object,
			 scenario_description,
			 &scenario_id,
			 &date_time,
			 &driving_mode,
			 &illuminance,
			 &rainfall,
			 &cloudness,
			 &snowfall,
			 &wind,
			 &triggered_cause);
			 
		char chat_data[] = "metadata 수신 확인";
		if (write(c_socket, chat_data, strlen(chat_data)) < 0) {
			perror("Write failed");
		}
		usleep(500000);

		receive_route(c_socket,
			 &pedestrian_density,
			 &traffic_density,
			 special_vehicles,
			 &zones,
			 &road_types,
			 &intersections,
			 &roundabouts,
			 special_structures);
			 
		char route_data[] = "routedata 수신 확인";
		if (write(c_socket, route_data, strlen(route_data)) < 0) {
			perror("Write failed");
		}

		close(c_socket);

		save_to_json(meta_name,
			 abnormal_cause,
			 abnormal_discerned_timestamp,
			 scenario_causative_object,
			 scenario_description,
			 scenario_id,
			 date_time,
			 driving_mode,
			 illuminance,
			 rainfall,
			 cloudness,
			 snowfall,
			 wind,
			 triggered_cause,
			 triggered_timestamp,
			 duration,
			 record_date,
			 dynamic_elements,
			 scenery,
			 image,
			 travel_path,
			 video,
			 T.MakeDirectoryBuf);

		route_to_json(route_name,
			 pedestrian_density,
			 traffic_density,
			 zones,
			 road_types,
			 intersections,
			 roundabouts,
			 T.MakeDirectoryBuf);

		duration = 0.0;
		// gps.yaml 삭제
		snprintf(cmd_buffer, sizeof(cmd_buffer), "rm %sgps.yaml", T.MakeDirectoryBuf);
                if (execute_command(cmd_buffer) == -1) {
                    log_message("Error deleting gps.yaml", NULL);
                } else {
                    log_message("Successfully deleted gps.yaml", NULL);
                }

		// 트리거 시점 앞 15초 뒤 5초 영상 저장
                if (backup_count < 2) {
                    ret = image_extractor( first_bag, "15" , "/clpe/cam_0/compressed", first_bag, "5" , "/clpe/cam_0/compressed",T.MakeDirectoryBuf);
                } else if (backup_count == 2) {
                    ret = image_extractor( first_bag, "15" , "/clpe/cam_0/compressed", second_bag, "5" , "/clpe/cam_0/compressed",T.MakeDirectoryBuf);
                } else {
                    ret = image_extractor( second_bag, "15" , "/clpe/cam_0/compressed", third_bag, "5" , "/clpe/cam_0/compressed",T.MakeDirectoryBuf);
                }

		// change name video clip, thumbnail
		while (ret) {
		    snprintf(cmd_buffer, sizeof(cmd_buffer), "mv %sthumbnail.jpg %s%s 2>/dev/null", T.MakeDirectoryBuf, T.MakeDirectoryBuf, thumbnail_name);
		    if (execute_command(cmd_buffer) == -1) {
                        log_message("Error changing thumbnail name", NULL);
                    } else {
                        log_message("Successfully changed thumbnail name: ", thumbnail_name);
                        break;
                    }
                    sleep(1);
		}
		while (ret) {
		    snprintf(cmd_buffer, sizeof(cmd_buffer), "mv %svideo_clip.mp4 %s%s 2>/dev/null", T.MakeDirectoryBuf, T.MakeDirectoryBuf, clip_name);
		    if (execute_command(cmd_buffer) == -1) {
                        log_message("Error changing video clip name", NULL);
                    } else {
                        log_message("Successfully changed video clip name: ", clip_name);
                        break;
                    }
                    sleep(3);
		}

                // upload bagfile
                /*if (backup_count < 2) {
                    upload_bag_files(first_bag);
                } else if (backup_count == 2) {
                    upload_bag_files(first_bag);
                    upload_bag_files(second_bag);
                } else {
                    upload_bag_files(first_bag);
                    upload_bag_files(second_bag);
                    upload_bag_files(third_bag);
                }
                upload_files(T.MakeDirectoryBuf);*/

                restart_rosbag(); 
            	}            
	    }
        }
        sleep(1);
    }
    return NULL;
}
