#include "util.h"

// 시스템 명령어 실행 함수
int execute_command(const char *cmd) {
    pid_t pid = fork();
    if (pid == -1) {
        log_message("Error: Fork failed.", NULL);
        perror("fork");
        return -1;
    } else if (pid == 0) {
        // 자식 프로세스에서 명령어 실행
        if (execlp("sh", "sh", "-c", cmd, (char *)NULL) == -1) {
            log_message("Error executing command.", NULL);
            perror("execlp");
            exit(1);
        }
    } else {
        // 부모 프로세스에서 자식 프로세스의 종료를 기다림
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return 0;
        } else {
            log_message("Command execution failed.", NULL);
            return -1;
        }
    }
    
    return 0;
}

// ROS 실행 함수
int run_ros(){
    char cmd_buffer[BUFSIZE];
    snprintf(cmd_buffer, sizeof(cmd_buffer), "%s/run.sh &", ROS_PATH);
    if (execute_command(cmd_buffer) == -1) {
        log_message("Error run run.sh", NULL);
        return -1;
    } else {
        log_message("Successfully run run.sh", NULL);
    }

    return 1;
}

// ROSBAG 실행 함수
int run_rosbag(){
    char cmd_buffer[BUFSIZE];
    snprintf(cmd_buffer, sizeof(cmd_buffer), "%s/rosbag_record.sh &", ROS_PATH);
    if (execute_command(cmd_buffer) == -1) {
        log_message("Error run rosbag_record.sh", NULL);
        return -1;
    } else {
        log_message("Successfully run rosbag_record.sh", NULL);
    }

    return 1;
}


// ROS 종료 함수
int kill_rosbag() {
    char cmd_buffer[BUFSIZE];
    snprintf(cmd_buffer, sizeof(cmd_buffer), "killall rosbag_record.sh");
    if (execute_command(cmd_buffer) == -1) {
        log_message("Error kill rosbag_record.sh", NULL);
        return -1;
    } else {
        log_message("Successfully kill rosbag_record.sh", NULL);
    }

    snprintf(cmd_buffer, sizeof(cmd_buffer), "killall ros2");
    if (execute_command(cmd_buffer) == -1) {
        log_message("Error kill ros2.", NULL);
        return -1;
    } else {
        log_message("Successfully kill ros2", NULL);
    }

    return 1;
}

// image_extractor 실행 함수
int image_extractor(const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *arg7) {
    char cmd_buffer[BUFSIZE];
    char combined_arg1[BUFSIZE];
    char combined_arg4[BUFSIZE];
    
    if (arg1 != NULL && arg7 != NULL) {
        const char *filename = strrchr(arg1, '/'); 
        if (filename) {
            filename++;  
        } else {
            filename = arg1;  
        }

        snprintf(combined_arg1, sizeof(combined_arg1), "%s%s", arg7, filename);
        arg1 = combined_arg1;
    }

    if (arg4 != NULL && arg7 != NULL) {
        const char *filename = strrchr(arg4, '/'); 
        if (filename) {
            filename++;  
        } else {
            filename = arg4;  
        }

        snprintf(combined_arg4, sizeof(combined_arg4), "%s%s", arg7, filename);
        arg4 = combined_arg4;  
    }
    
    snprintf(cmd_buffer, sizeof(cmd_buffer), "%s/image_extractor.sh", ROS_PATH);

    const char *args[] = {arg1, arg2, arg3, arg4, arg5, arg6, arg7};
    for (int i = 0; i < 7; i++) {
        if (args[i] != NULL) {
            strncat(cmd_buffer, " ", sizeof(cmd_buffer) - strlen(cmd_buffer) - 1);
            strncat(cmd_buffer, args[i], sizeof(cmd_buffer) - strlen(cmd_buffer) - 1);
        }
    }
    strncat(cmd_buffer, " &", sizeof(cmd_buffer) - strlen(cmd_buffer) - 1);

    if (execute_command(cmd_buffer) == -1) {
        log_message("Error running image_extractor.sh", NULL);
        return -1;
    } else {
        log_message("Successfully ran image_extractor.sh", NULL);
    }

    return 1;
}

// gps_extractor 실행 함수
int gps_extractor(const char *arg1) {
    char cmd_buffer[BUFSIZE];

    snprintf(cmd_buffer, sizeof(cmd_buffer), "%s/gps_extractor.sh", ROS_PATH);
    
    if (arg1 != NULL) {
        strncat(cmd_buffer, " ", sizeof(cmd_buffer) - strlen(cmd_buffer) - 1);
        strncat(cmd_buffer, arg1, sizeof(cmd_buffer) - strlen(cmd_buffer) - 1);
    }
    
    strncat(cmd_buffer, " &", sizeof(cmd_buffer) - strlen(cmd_buffer) - 1);

    if (execute_command(cmd_buffer) == -1) {
        log_message("Error running gps_extractor.sh", NULL);
        return -1;
    } else {
        log_message("Successfully ran gps_extractor.sh", NULL);
    }

    return 1;
}

// 로그 저장 함수
void log_message(const char *message, const char *var) {
    char log[BUFSIZE];
    
    FILE *log_file = fopen("log.txt", "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }

    if(var != NULL)
    	snprintf(log, sizeof(log), "%s %s\n", message, var);
    else
    	snprintf(log, sizeof(log), "%s\n", message);
    
    // Get current timestamp
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(log_file, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n", 
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, 
            t->tm_hour, t->tm_min, t->tm_sec, log);
    fclose(log_file);
}

// 동기화 함수
int waitForSync() {
    FILE *fp;
    char buffer[1024];
    char command[256];
    int syncSuccess = 0;
    
    snprintf(command, sizeof(command), "echo %s | sudo -S ./sync_time.sh client %s", SUDO_PASSWORD, ADS_IPADDR);

    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen 실행 실패");
        return -1;
    }

    // 출력된 내용을 한 줄씩 읽으며 성공 메시지가 있는지 확인
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, "시간 동기화가 완료되었습니다.") != NULL) {
            printf("Chrony 동기화 완료.\n");
    	    time_t curr_time = time(NULL);
	    if (curr_time == ((time_t) -1)) {
		log_message("time() Fail", NULL);
	        return -1;
	    }
       	    printf("현재 시스템 시간: %s", ctime(&curr_time));
        }else if(strstr(buffer, "최대 시도 횟수 내에 시간 동기화가 완료되지 않았습니다.") != NULL) {
            printf("Chrony 동기화 실패.\n");
            return -1;
        }
    }

    pclose(fp);
    
    return 0;
}

// 숫자 추출 함수
int extract_number_from_filename(const char *filename) {
    const char *p = filename;
    while (*p && !isdigit(*p)) {  // 숫자가 나올 때까지 이동
        p++;
    }
    return *p ? atoi(p) : -1;  // 숫자가 있으면 변환, 없으면 -1 반환
}

// 숫자를 기준으로 정렬하는 비교 함수
int compare_files_by_number(const void *a, const void *b) {
    const char *file1 = *(const char **)a;
    const char *file2 = *(const char **)b;

    int num1 = extract_number_from_filename(file1);
    int num2 = extract_number_from_filename(file2);

    return num1 - num2;  // 숫자 기준 오름차순 정렬
}

// 백업파일 이름 저장
void backup_name(char *target, char* dest, const char* name) {
    char temp[128];
    snprintf(temp, sizeof(temp), "%s%s", dest, name);
    snprintf(target, sizeof(temp), "%s", temp);
}

// 서버에 .db3 업로드 하는 함수
int upload_bag_files(){
	return 0;
}

// JSON 파일 저장 함수
void save_to_json(const char *filename,
                  int abnormal_cause,
                  const char *abnormal_discerned_timestamp,
                  int scenario_causative_object,
                  const char *scenario_description,
                  int scenario_id,
                  int date_time,
                  int driving_mode,
                  int illuminance,
                  int rainfall,
                  int cloudness,
                  int snowfall,
                  int wind,
                  int triggered_cause,
                  const char *triggered_timestamp,
                  double duration,
                  const char *record_date,
                  const char *dynamic_elements,
                  const char *scenery,
                  const char *image,
                  const char *travel_path,
                  const char *video,
                  char *MakeDirectoryBuf) {

    char full_path[BUFSIZE];

    snprintf(full_path, sizeof(full_path), "%s%s", MakeDirectoryBuf, filename);

    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        log_message("Failed to create JSON root object", NULL);
        return;
    }
    
    cJSON *abnormal_driving_cause_obj = cJSON_CreateObject();
    if (abnormal_driving_cause_obj == NULL) {
        log_message("Failed to create JSON abnormal_driving_cause object", NULL);
        cJSON_Delete(root);
        return;
    }
    //cJSON_AddNumberToObject(abnormal_driving_cause_obj, "cause", abnormal_cause);
    cJSON_AddNullToObject(abnormal_driving_cause_obj, "cause");
    //cJSON_AddStringToObject(abnormal_driving_cause_obj, "discerned_timestamp", abnormal_discerned_timestamp);
    cJSON_AddNullToObject(abnormal_driving_cause_obj, "discerned_timestamp");
    
    cJSON *scenario_obj = cJSON_CreateObject();
    if (scenario_obj == NULL) {
        log_message("Failed to create JSON scenario object", NULL);
        cJSON_Delete(abnormal_driving_cause_obj);
        cJSON_Delete(root);
        return;
    }
    //cJSON_AddNumberToObject(scenario_obj, "causative_object", scenario_causative_object);
    cJSON_AddNullToObject(scenario_obj, "causative_object");
    cJSON_AddStringToObject(scenario_obj, "description", scenario_description);
    //cJSON_AddNumberToObject(scenario_obj, "scenario_id", scenario_id);
    cJSON_AddNullToObject(scenario_obj, "scenario_id");
    
    cJSON_AddItemToObject(abnormal_driving_cause_obj, "scenario", scenario_obj);
    cJSON_AddItemToObject(root, "abnormal_driving_cause", abnormal_driving_cause_obj);
    
    cJSON_AddNumberToObject(root, "datetime", date_time);
    cJSON_AddNumberToObject(root, "driving_mode", driving_mode);
    
    cJSON *log_header_obj = cJSON_CreateObject();
    if (log_header_obj == NULL) {
        log_message("Failed to create JSON log_header object", NULL);
        cJSON_Delete(root);
        return;
    }
    cJSON_AddNumberToObject(log_header_obj, "duration", duration);
    cJSON_AddStringToObject(log_header_obj, "record_date", record_date);
    
    cJSON *topic_list_arr = cJSON_CreateArray();
    if (topic_list_arr == NULL) {
        log_message("Failed to create JSON topic_list array", NULL);
        cJSON_Delete(log_header_obj);
        cJSON_Delete(root);
        return;
    }
    char cmd_buffer[BUFSIZE];
    int topic_max = 50;
    int topic_count = 0;
    char row[BUFSIZE];
    char topic_name[topic_max][BUFSIZE];
    char topic_type[topic_max][BUFSIZE];
    snprintf(cmd_buffer,sizeof(cmd_buffer), "ros2 bag info %s%s | grep Topic: > /tmp/tmp_topic.txt", MakeDirectoryBuf, trigger_time);
    if (execute_command(cmd_buffer) == -1) {
        log_message("Error executing bag info for topic list", NULL);
    }
    
    FILE *fp = fopen("/tmp/tmp_topic.txt", "r");
    if (fp == NULL) {
        log_message("Error open tmp_topic.txt", NULL);
        return;
    }
    while (fgets(row, sizeof(row), fp) && topic_count < topic_max) {
        char *topic_start = strstr(row, "Topic: ");
        char *type_start = strstr(row, "Type: ");

        topic_start += 7;
        type_start += 6;

        char *topic_end = strchr(topic_start, '|');
        char *type_end = strchr(type_start, '|');
        size_t topic_len = topic_end - topic_start;
        size_t type_len = type_end - type_start;
        if (topic_len >= BUFSIZE || type_len >= BUFSIZE) {
            log_message("Topic name or type too long", NULL);
            return;
        }

        memcpy(topic_name[topic_count], topic_start, topic_len);
        topic_name[topic_count][topic_len] = '\0';
        strtok(topic_name[topic_count], " ");

        memcpy(topic_type[topic_count], type_start, type_len);
        topic_type[topic_count][type_len] = '\0';
        strtok(topic_type[topic_count], " ");

        topic_count++;
    }
    fclose(fp);

    for (int i = 0; i < topic_count; i++) {
        cJSON *topic_obj = cJSON_CreateObject();
        if (topic_obj == NULL) {
            log_message("Failed to create JSON topic object", NULL);
            cJSON_Delete(topic_list_arr);
            cJSON_Delete(log_header_obj);
            cJSON_Delete(root);
            return;
        } else {
            cJSON_AddStringToObject(topic_obj, "topic_name", topic_name[i]);
            cJSON_AddStringToObject(topic_obj, "topic_type", topic_type[i]);
        }
        cJSON_AddItemToArray(topic_list_arr, topic_obj);
    }
    
    cJSON_AddItemToObject(log_header_obj, "topic_list", topic_list_arr);
    cJSON_AddItemToObject(root, "log_header", log_header_obj);
    
    cJSON *scene_context_obj = cJSON_CreateObject();
    if (scene_context_obj == NULL) {
        log_message("Failed to create JSON scene_context object", NULL);
        cJSON_Delete(root);
        return;
    }
    cJSON_AddStringToObject(scene_context_obj, "dynamic_elements", dynamic_elements);
    
    cJSON *environmental_conditions_obj = cJSON_CreateObject();
    if (environmental_conditions_obj == NULL) {
        log_message("Failed to create JSON environmental_conditions object", NULL);
        cJSON_Delete(scene_context_obj);
        cJSON_Delete(root);
        return;
    }
    cJSON_AddNumberToObject(environmental_conditions_obj, "illuminance", illuminance);
    cJSON_AddNumberToObject(environmental_conditions_obj, "rainfall", rainfall);
    cJSON_AddNumberToObject(environmental_conditions_obj, "cloudness", cloudness);
    cJSON_AddNumberToObject(environmental_conditions_obj, "snowfall", snowfall);
    cJSON_AddNumberToObject(environmental_conditions_obj, "wind", wind);
    
    cJSON_AddItemToObject(scene_context_obj, "environmental_conditions", environmental_conditions_obj);
    
    cJSON_AddStringToObject(scene_context_obj, "scenery", scenery);
    
    cJSON_AddItemToObject(root, "scene_context", scene_context_obj);
    
    cJSON *screen_obj = cJSON_CreateObject();
    if (screen_obj == NULL) {
        log_message("Failed to create JSON screen object", NULL);
        cJSON_Delete(root);
        return;
    }
    cJSON_AddStringToObject(screen_obj, "image", image);
    cJSON_AddStringToObject(screen_obj, "travel_path", travel_path);
    
    char gps_path[BUFSIZE];
    char gps_row[BUFSIZE];
    int gps_count = 0;
    int file_idx = 0;
    int trg_idx = 0;
    double latitude = 0.0;
    double longitude = 0.0;
    snprintf(gps_path, sizeof(gps_path), "%sgps.yaml", MakeDirectoryBuf);
    FILE *gps_file = fopen(gps_path, "r");
    if (gps_file != NULL) {
        while (fgets(gps_row, sizeof(gps_row), gps_file) != NULL) {
            int tmp_idx;
            if (sscanf(gps_row, " %d", &tmp_idx) == 1) {
                file_idx = tmp_idx;
                continue;
            }
            if (strstr(gps_row, "lat:") != NULL) {
                if (file_idx == 2 && trg_idx == 0) {
                    trg_idx = gps_count;
                    if (sscanf(gps_row, " - lat: %lf", &latitude) == 1) {
                        fgets(gps_row, sizeof(gps_row), gps_file);
                        sscanf(gps_row, " lon: %lf", &longitude);
                    }
                    break;
                }
                gps_count++;
            }
        }
    } else {
        log_message("Failed to open gps.yaml", NULL);
        return;
    }
    fclose(gps_file);
    
    cJSON *triggered_position_obj = cJSON_CreateObject();
    if (triggered_position_obj == NULL) {
        log_message("Failed to create JSON triggered_position object", NULL);
        cJSON_Delete(screen_obj);
        cJSON_Delete(root);
        return;
    }
    cJSON_AddNumberToObject(triggered_position_obj, "idx", trg_idx);
    cJSON_AddNumberToObject(triggered_position_obj, "latitude", latitude);
    cJSON_AddNumberToObject(triggered_position_obj, "longitude", longitude);
    
    cJSON_AddItemToObject(screen_obj, "triggered_position", triggered_position_obj);
    
    cJSON_AddStringToObject(screen_obj, "video", video);
    
    cJSON_AddItemToObject(root, "screen", screen_obj);
    
    cJSON *triggered_cause_obj = cJSON_CreateObject();
    if (triggered_cause_obj == NULL) {
        log_message("Failed to create JSON triggered_cause object", NULL);
        cJSON_Delete(root);
        return;
    }
    cJSON_AddNumberToObject(triggered_cause_obj, "cause", triggered_cause);
    cJSON_AddStringToObject(triggered_cause_obj, "triggered_timestamp", triggered_timestamp);
    
    cJSON_AddItemToObject(root, "triggered_cause", triggered_cause_obj);
    
    char *json_string = cJSON_Print(root);
    if (json_string == NULL) {
        log_message("Failed to print JSON string", NULL);
        cJSON_Delete(root);
        return;
    }
    
    FILE *file = fopen(full_path, "w");
    if (file == NULL) {
        char err_msg[BUFSIZE];
        snprintf(err_msg, sizeof(err_msg), "파일 생성 실패: %s", filename);
        log_message(err_msg, NULL);
        free(json_string);
        cJSON_Delete(root);
        return;
    }
    fprintf(file, "%s\n", json_string);
    fclose(file);
    
    // 성공 로그 기록
    {
        char success_msg[BUFSIZE];
        snprintf(success_msg, sizeof(success_msg), "JSON 파일 생성 완료: %s", filename);
        log_message(success_msg, NULL);
    }
    
    free(json_string);
    cJSON_Delete(root);
}

void route_to_json(const char *filename,
                  int pedestrian_density,
                  int traffic_density,
                  int zones,
                  int road_types,
                  int intersections,
                  bool roundabouts,
                  char *MakeDirectoryBuf) {

    char full_path[BUFSIZE];

    snprintf(full_path, sizeof(full_path), "%s%s", MakeDirectoryBuf, filename);

    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        log_message("Failed to create JSON root object", NULL);
        return;
    }
    
    cJSON *data_arr = cJSON_CreateArray();
    if (data_arr == NULL) {
        log_message("Failed to create JSON data array", NULL);
        cJSON_Delete(root);
        return;
    }
    
    char gps_path[BUFSIZE];
    char row[BUFSIZE];
    int num_idx = 0;
    snprintf(gps_path, sizeof(gps_path), "%sgps.yaml", MakeDirectoryBuf);
    FILE *gps_file = fopen(gps_path, "r");
    if (gps_file != NULL) {
        while (fgets(row, sizeof(row), gps_file) != NULL) {
            if (strstr(row, "lat:") != NULL) {
                num_idx++;
            }
        }
    } else {
        log_message("Failed to open gps.yaml", NULL);
        return;
    }
    fclose(gps_file);
    
    double latitude[num_idx], longitude[num_idx];
    unsigned long timestamp[num_idx];
    char row2[BUFSIZE];
    int tmp_idx = 0;
    FILE *gps_file2 = fopen(gps_path, "r");
    if (gps_file2 != NULL) {
        while (fgets(row2, sizeof(row2), gps_file2) && tmp_idx < num_idx) {
            if (sscanf(row2, " - lat: %lf", &latitude[tmp_idx]) == 1) {
                fgets(row2, sizeof(row2), gps_file2);
                sscanf(row2, " lon: %lf", &longitude[tmp_idx]);
                fgets(row2, sizeof(row2), gps_file2);
                sscanf(row2, " timestamp: %lu", &timestamp[tmp_idx]);
                tmp_idx++;
            }
        }
    } else {
        log_message("Failed to open gps.yaml", NULL);
        return;
    }
    fclose(gps_file2);
    
    for (int i = 0; i < num_idx; i++) {
        cJSON *data_obj = cJSON_CreateObject();
        if (data_obj == NULL) {
            log_message("Failed to create JSON data object", NULL);
            cJSON_Delete(data_arr);
            cJSON_Delete(root);
            return;
        }
        cJSON *dynamic_elements_obj = cJSON_CreateObject();
        if (dynamic_elements_obj == NULL) {
            log_message("Failed to create JSON dynamic_elements object", NULL);
            cJSON_Delete(data_obj);
            cJSON_Delete(data_arr);
            cJSON_Delete(root);
            return;
        }
        cJSON_AddNumberToObject(dynamic_elements_obj, "pedestrian_density", pedestrian_density);
        
        cJSON *special_vehicles_arr = cJSON_CreateArray();
        if (special_vehicles_arr == NULL) {
            log_message("Failed to create JSON special_vehicles array", NULL);
            cJSON_Delete(dynamic_elements_obj);
            cJSON_Delete(data_obj);
            cJSON_Delete(data_arr);
            cJSON_Delete(root);
            return;
        }
        for (int i = 0; i < sizeof(vehicle_array) / sizeof(vehicle_array[0]); i++) {
            if (vehicle_array[i] == 100) {
                break;
            } else {
                cJSON_AddItemToArray(special_vehicles_arr, cJSON_CreateNumber(vehicle_array[i]));
            }
        }
        cJSON_AddItemToObject(dynamic_elements_obj, "special_vehicles", special_vehicles_arr);
        
        cJSON_AddNumberToObject(dynamic_elements_obj, "traffic_density", traffic_density);
        cJSON_AddItemToObject(data_obj, "dynamic_elements", dynamic_elements_obj);
        
        cJSON_AddNumberToObject(data_obj, "idx", i);
        
        cJSON_AddNumberToObject(data_obj, "timestamp", timestamp[i]);
        
        cJSON *scenery_obj = cJSON_CreateObject();
        if (scenery_obj == NULL) {
            log_message("Failed to create JSON scenery object", NULL);
            cJSON_Delete(data_obj);
            cJSON_Delete(data_arr);
            cJSON_Delete(root);
            return;
        }
        cJSON *junctions_obj = cJSON_CreateObject();
        if (junctions_obj == NULL) {
            log_message("Failed to create JSON junctions object", NULL);
            cJSON_Delete(scenery_obj);
            cJSON_Delete(data_obj);
            cJSON_Delete(data_arr);
            cJSON_Delete(root);
            return;
        }
        cJSON_AddNumberToObject(junctions_obj, "intersections", intersections);
        cJSON_AddBoolToObject(junctions_obj, "roundabouts", roundabouts);
        cJSON_AddItemToObject(scenery_obj, "junctions", junctions_obj);
        cJSON_AddNumberToObject(scenery_obj, "road_types", road_types);
        
        cJSON *special_structures_arr = cJSON_CreateArray();
        if (special_structures_arr == NULL) {
            log_message("Failed to create JSON special_structures array", NULL);
            cJSON_Delete(scenery_obj);
            cJSON_Delete(data_obj);
            cJSON_Delete(data_arr);
            cJSON_Delete(root);
            return;
        }
        for (int i = 0; i < sizeof(structure_array) / sizeof(structure_array[0]); i++) {
            if (structure_array[i] == 100) {
                break;
            } else {
                cJSON_AddItemToArray(special_structures_arr, cJSON_CreateNumber(structure_array[i]));
            }
        }
        cJSON_AddItemToObject(scenery_obj, "special_structures", special_structures_arr);

        cJSON_AddNumberToObject(scenery_obj, "zones", zones);
        cJSON_AddItemToObject(data_obj, "scenery", scenery_obj);
    
        cJSON *travel_path_obj = cJSON_CreateObject();
        if (travel_path_obj == NULL) {
            log_message("Failed to create JSON travel_path object", NULL);
            cJSON_Delete(data_obj);
            cJSON_Delete(data_arr);
            cJSON_Delete(root);
            return;
        }
        cJSON_AddNumberToObject(travel_path_obj, "latitude", latitude[i]);
        cJSON_AddNumberToObject(travel_path_obj, "longitude", longitude[i]);
        cJSON_AddItemToObject(data_obj, "travel_path", travel_path_obj);
        
        cJSON_AddItemToArray(data_arr, data_obj);
    }
    cJSON_AddItemToObject(root, "data", data_arr);
    
    char *json_string = cJSON_Print(root);
    if (json_string == NULL) {
        log_message("Failed to print JSON string", NULL);
        cJSON_Delete(root);
        return;
    }
    
    FILE *file = fopen(full_path, "w");
    if (file == NULL) {
        char err_msg[BUFSIZE];
        snprintf(err_msg, sizeof(err_msg), "파일 생성 실패: %s", filename);
        log_message(err_msg, NULL);
        free(json_string);
        cJSON_Delete(root);
        return;
    }
    fprintf(file, "%s\n", json_string);
    fclose(file);
    
    // 성공 로그 기록
    {
        char success_msg[BUFSIZE];
        snprintf(success_msg, sizeof(success_msg), "JSON 파일 생성 완료: %s", filename);
        log_message(success_msg, NULL);
    }
    
    free(json_string);
    cJSON_Delete(root);
}

