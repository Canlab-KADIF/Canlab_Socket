#include "file_manager.h"

extern TRIGGER_CLIENT T;

extern volatile int running;  

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

    if (snprintf(dest, size, "%s_TRG_%s/", path, timestamp) >= size) { // dest배열에 TRG폴더이름 저장
        log_message("Error: Path is too long.", NULL);
        return -1;
    }

    if (mkdir(dest, S_IFDIR | S_IRWXU | S_IRWXG | S_IXOTH | S_IROTH) == -1) { // mkdir로 TRG폴더 생성
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
    struct dirent *entry; // 디렉토리 내 항목들의 이름(d_name), 타입(d_type) 등 정보를 저장하는 구조체
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        log_message("Failed to open directory", NULL);
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        // 상위 폴더 건너뛰기 string compare, d_name == "." 이면 0을 리턴
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[BUFSIZE]; // full_path=/home/canlab/BAG/d_name
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            // type이 폴더일 경우 
            find_files_in_directory(full_path, ext, files, count);
        } else {
            // type이 파일일 경우
            if (ext == NULL || strstr(entry->d_name, ext) != NULL) { // d_name에서 ext와 일치하는 스트링이 있는지 확인
                files[*count] = strdup(full_path); // string duplicate, files의 인덱스에 파일명 저장
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

    ret = run_rosbag();
    if (ret == -1) {
        log_message("Ros Run Failed", NULL);
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
                end_index = file_count;
                log_message("File Count < 2. Selecting target file", NULL);
                T.Count = 0;
            }

            if (T.Count == 1) {
                target_index = file_count - 1;
                start_index = target_index - 1;
                end_index = target_index + 1;
                log_message("Selecting target file", NULL);
                T.Count = 0;
            }

	    // 3개의 파일이 생성 된 후 파일 이동
            if (file_count >= target_index + 3) {
                for (int i = start_index; i <= end_index; i++) {
                    snprintf(cmd_buffer, sizeof(cmd_buffer), "cp %s %s", files[i], T.MakeDirectoryBuf);
                    if (execute_command(cmd_buffer) == -1) {
                        log_message("Error copying file.", NULL);
                    } else {
                        log_message("Successfully copied file: ", files[i]);
                    }
                    //upload_bag_files(); // BAG 업로드
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
                        
                        restart_rosbag(); 
            	}            
	    }
        }
        sleep(1);
    }
    return NULL;
}
