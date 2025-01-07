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

