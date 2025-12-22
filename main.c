#include "util.h"

TRIGGER_CLIENT T;
volatile int running = 1; 

void handle_sigint(int sig) {
    kill_rosbag();
    printf("\nShutting down...\n");
    display_banner("DASHBOARD END");
    running = 0;
}

void cleanup_thread(pthread_t *threads, int thread_count, int socket_fd) {
    for (int i = 0; i < thread_count; ++i) {
        pthread_cancel(threads[i]);  // 스레드 강제 종료 요청
        pthread_join(threads[i], NULL);  
    }
    if (socket_fd >= 0) {
        shutdown(socket_fd, SHUT_RDWR);  // 소켓 종료
        close(socket_fd);
    }
}

int main() {
    int c_socket = -1;
    pthread_t threads[4];  // 스레드 배열
    int thread_count = 0, ret = 0;

    signal(SIGINT, handle_sigint);

    display_banner("DASHBOARD START");

    c_socket = connect_to_server(ADS_IPADDR, ADS_PORT);
    if (c_socket == -1) {
        cleanup_thread(threads, thread_count, c_socket);
        display_banner("DASHBOARD END");
        return -1;
    }

    ret = run_rosbag();
    if (ret == -1) {
        log_message("Ros Run Failed", NULL);
        cleanup_thread(threads, thread_count, c_socket);
        display_banner("DASHBOARD END");
        return -1; 
    }
    
    if (create_directory(T.MakeDirectoryBuf, sizeof(T.MakeDirectoryBuf)) != 0) {
        log_message("Make Directory Failed", NULL);
        display_banner("DASHBOARD END");
        return -1;
    }
    // 스레드 생성 및 오류 처리
    if (pthread_create(&threads[thread_count++], NULL, receive_chat, (void*)&c_socket) != 0) {
        log_message("Error: Failed to create receive_chat thread", NULL);
        cleanup_thread(threads, thread_count, c_socket);
        display_banner("DASHBOARD END");
        return -1;
    }

    if (pthread_create(&threads[thread_count++], NULL, send_chat, (void*)&c_socket) != 0) {
        log_message("Error: Failed to create send_chat thread", NULL);
        cleanup_thread(threads, thread_count, c_socket);
        display_banner("DASHBOARD END");	
        return -1;
    }

    if (pthread_create(&threads[thread_count++], NULL, check_file_count, NULL) != 0) {
        log_message("Error: Failed to create check_file_count thread", NULL);
        cleanup_thread(threads, thread_count, c_socket);
        display_banner("DASHBOARD END");
        return -1;
    }

    if (pthread_create(&threads[thread_count++], NULL, move_bag_files, NULL) != 0) {
        log_message("Error: Failed to create move_bag_files thread", NULL);
        cleanup_thread(threads, thread_count, c_socket);
        display_banner("DASHBOARD END");
        return -1;
    }

    // 메인 루프
    while (running) {
        sleep(1);
    }

    cleanup_thread(threads, thread_count, c_socket);
    
    return 0;
}

