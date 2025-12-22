#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "util.h"

#define path "/home/canlab/BAG" // 로깅데이터 경로
#define ROS_PATH "/home/canlab/Canlab_ROS2" // ROS2 패키지 경로
#define MAX_FILES 70  // 최대 파일 수

int create_directory(char* dest, size_t size);
int find_files_in_directory(const char *dir_path, const char *ext, char **files, int *count);

void restart_rosbag();
//void set_target_indices(int *start_index, int *end_index, int target);
void* check_file_count(void* arg);
void* move_bag_files(void* arg);

#endif

