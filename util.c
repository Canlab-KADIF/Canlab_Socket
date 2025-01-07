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
int run_rosbag(){
    char cmd_buffer[BUFSIZE];
    snprintf(cmd_buffer, sizeof(cmd_buffer), "/home/canlab/rosbag_record.sh &");
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

// 로그 저장 함수
void log_message(const char *message, const char *var) {
    char log[BUFSIZE];
    
    FILE *log_file = fopen("/home/canlab/log.txt", "a");
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

// 서버에 .db3 업로드 하는 함수
int upload_bag_files(){
	return 0;
}

