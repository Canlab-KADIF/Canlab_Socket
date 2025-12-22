#include "network.h"

extern TRIGGER_CLIENT T;
extern volatile int running;  

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 

int connect_to_server(const char* ip, int port) {
    int c_socket;
    struct sockaddr_in c_addr;

    c_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (c_socket == -1) {
        perror("Socket creation failed");
        return -1;
    }

    memset(&c_addr, 0, sizeof(c_addr));
    c_addr.sin_addr.s_addr = inet_addr(ip);
    c_addr.sin_family = AF_INET;
    c_addr.sin_port = htons(port);

    while (connect(c_socket, (struct sockaddr*)&c_addr, sizeof(c_addr)) == -1) {
        if (!running) {  // 종료 요청 시 연결 종료
            close(c_socket);
            return -1;
        }
        log_message("Wait Server", NULL);
        sleep(1);
    }

    log_message("Connect Success", NULL);
    return c_socket;
}


void* receive_chat(void* arg) {
    int c_socket = *((int*)arg); 
    char chat_data[BUFSIZE];
    int n;

    while (running) { 
        memset(chat_data, 0, sizeof(chat_data)); 

        // 서버로부터 데이터 읽기
        n = read(c_socket, chat_data, sizeof(chat_data) - 1);

        chat_data[n] = '\0';  // 받은 데이터의 끝에 널 종료 문자 추가

        printf("서버 메시지: %s\n", chat_data);

        // 데이터 처리 시 동기화
        pthread_mutex_lock(&lock);
        handle_trigger(chat_data); 
        pthread_mutex_unlock(&lock);
        
        usleep(100000); 
    }

    return NULL;
}


void* send_chat(void* arg) {
    int c_socket = *((int*)arg);
    char chat_data[] = "logging";

    // 초기값 설정
    T.Check = 0;

    while (running) {
        if (T.Trigger == 1 && T.Check == 0) {
            // 서버로 데이터 전송
            if (write(c_socket, chat_data, strlen(chat_data)) < 0) {
                perror("Write failed");
                break;
            }
            pthread_mutex_lock(&lock);
            T.Check = 1;  // 전송 완료 플래그 설정
            pthread_mutex_unlock(&lock);
        }

        usleep(100000); 
    }

    return NULL;
}

int read_field(int sockfd, const char *field_name, char *str_result, int *int_result, bool *bool_result) {
    char buffer[MAX_STR];
    ssize_t total_read = 0;
    char ch;

    // 라인 확인
    while (total_read < MAX_STR - 1) {
        ssize_t bytes_read = read(sockfd, &ch, 1);
        if (bytes_read <= 0) {
            char err_msg[BUFSIZE];
            snprintf(err_msg, sizeof(err_msg), "Error reading %s: connection closed or error", field_name);
            log_message(err_msg, NULL);
            return -1;
        }

        if (ch == '\n') {
            break; // 라인 끝
        }

        buffer[total_read++] = ch;
    }

    buffer[total_read] = '\0'; // null-terminate

    // 문자열
    if (str_result != NULL) {
        strncpy(str_result, buffer, MAX_STR);
    }

    // 정수
    if (int_result != NULL) {
        char *endptr;
        long val = strtol(buffer, &endptr, 10);
        if (*endptr != '\0') {
            char err_msg[BUFSIZE];
            snprintf(err_msg, sizeof(err_msg), "Invalid integer for %s: '%s'", field_name, buffer);
            log_message(err_msg, NULL);
            return -1;
        }
        *int_result = (int)val;
    }

    // bool
    if (bool_result != NULL) {
        *bool_result = (strcmp(buffer, "1") == 0);
    }

    return 0;
}

int read_field_special(int sockfd, const char *field_name, char *str_result, char *array) {
    char buffer[MAX_STR];
    ssize_t total_read = 0;
    char ch;
    memset(array, 100, sizeof(array));

    // 라인 확인
    while (total_read < MAX_STR - 1) {
        ssize_t bytes_read = read(sockfd, &ch, 1);
        if (bytes_read <= 0) {
            char err_msg[BUFSIZE];
            snprintf(err_msg, sizeof(err_msg), "Error reading %s: connection closed or error", field_name);
            log_message(err_msg, NULL);
            return -1;
        }

        if (ch == '\n') {
            break; // 라인 끝
        }

        buffer[total_read++] = ch;
    }

    buffer[total_read] = '\0'; // null-terminate

    // 문자열
    if (str_result != NULL) {
        if (strlen(buffer) == 0) {
            strcpy(str_result, "");
        } else if (strchr(buffer, '_') != NULL) {
            char buffer_tmp[MAX_STR];
            strncpy(buffer_tmp, buffer, MAX_STR);
            buffer_tmp[MAX_STR - 1] = '\0';
            
            char *token = strtok(buffer_tmp, "_");
            int index = 0;
            
            while (token != NULL && index < sizeof(array) / sizeof(array[0])) {
                array[index] = atoi(token);
                token = strtok(NULL, "_");
                index++;
            }
        } else {
            array[0] = atoi(buffer);
        }
    }
    return 0;
}

int receive_data(int sockfd,
                 int *abnormal_cause,
                 char *abnormal_discerned_timestamp,
                 int *scenario_causative_object,
                 char *scenario_description,
                 int *scenario_id,
                 int *date_time,
                 int *driving_mode,
                 int *illuminance,
                 int *rainfall,
                 int *cloudness,
                 int *snowfall,
                 int *wind,
                 int *triggered_cause) {

    if (read_field(sockfd, "abnormal_cause", NULL, abnormal_cause, NULL) < 0)
        return -1;

    if (read_field(sockfd, "abnormal_discerned_timestamp", abnormal_discerned_timestamp, NULL, NULL) < 0)
        return -1;

    if (read_field(sockfd, "scenario_causative_object", NULL, scenario_causative_object, NULL) < 0)
        return -1;

    if (read_field(sockfd, "scenario_description", scenario_description, NULL, NULL) < 0)
        return -1;

    if (read_field(sockfd, "scenario_id", NULL, scenario_id, NULL) < 0)
        return -1;

    if (read_field(sockfd, "date_time", NULL, date_time, NULL) < 0)
        return -1;

    if (read_field(sockfd, "driving_mode", NULL, driving_mode, NULL) < 0)
        return -1;

    if (read_field(sockfd, "illuminance", NULL, illuminance, NULL) < 0)
        return -1;

    if (read_field(sockfd, "rainfall", NULL, rainfall, NULL) < 0)
        return -1;

    if (read_field(sockfd, "cloudness", NULL, cloudness, NULL) < 0)
        return -1;

    if (read_field(sockfd, "snowfall", NULL, snowfall, NULL) < 0)
        return -1;

    if (read_field(sockfd, "wind", NULL, wind, NULL) < 0)
        return -1;

    if (read_field(sockfd, "triggered_cause", NULL, triggered_cause, NULL) < 0)
        return -1;

    return 0;
}

int receive_route(int sockfd,
                  int *pedestrian_density,
                  int *traffic_density,
                  char *special_vehicles,
                  int *zones,
                  int *road_types,
                  int *intersections,
                  bool *roundabouts,
                  char *special_structures) {

    if (read_field(sockfd, "pedestrian_density", NULL, pedestrian_density, NULL) < 0)
        return -1;
    if (*pedestrian_density == 3) {
        *pedestrian_density = 99;
    }

    if (read_field(sockfd, "traffic_density", NULL, traffic_density, NULL) < 0)
        return -1;
    if (*traffic_density == 3) {
        *traffic_density = 99;
    }

    if (read_field_special(sockfd, "special_vehicles", special_vehicles, vehicle_array) < 0)
        return -1;

    if (read_field(sockfd, "zones", NULL, zones, NULL) < 0)
        return -1;

    if (read_field(sockfd, "road_types", NULL, road_types, NULL) < 0)
        return -1;

    if (read_field(sockfd, "intersections", NULL, intersections, NULL) < 0)
        return -1;

    if (read_field(sockfd, "roundabouts", NULL, NULL, roundabouts) < 0)
        return -1;

    if (read_field_special(sockfd, "special_structures", special_structures, structure_array) < 0)
        return -1;

    return 0;
}
