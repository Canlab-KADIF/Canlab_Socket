#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <ctype.h> 
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include "trigger.h"
#include "file_manager.h"
#include "network.h"
#include <curl/curl.h>

#define VERSION "r1.0.0"

void log_message(const char *message, const char *var);

int upload_bag_files();
int execute_command(const char *cmd);
int run_rosbag();
int kill_rosbag();
int extract_number_from_filename(const char *filename);
int compare_files_by_number(const void *a, const void *b);

#endif

