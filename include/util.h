#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
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
#include <cjson/cJSON.h>


#define VERSION "r1.1.0"
#define SUDO_PASSWORD "1"
#define FTP_ADDR "13.124.86.161/uploads/" // FTP 주소
#define FTP_ID "aqUfxwVB4A" // FTP 로그인 ID
#define FTP_PW "Q1EqIBIqlOtbXSQS" // FTP 로그인 PW
#define SESSIONID "CANL21" // SessionID
#define TYPE_RAW "raw" // Datatype-raw
#define TYPE_META "meta" // Datatype-meta
#define TYPE_ROUTE "route" // Datatype-route
#define TYPE_THUMBNAIL "thumbnail" // Datatype-thumbnail
#define TYPE_CLIP "video_clip" // Datatype-video clip
#define EXT_RAW ".db3" // ext-db3
#define EXT_JSON ".json" // ext-json
#define EXT_JPG ".jpg" // ext-jpg
#define EXT_MP4 ".mp4" // ext-mp4


void log_message(const char *message, const char *var);
void backup_name(char *target, char* dest, const char* name);

int execute_command(const char *cmd);
int run_ros();
int run_rosbag();
int kill_rosbag();
int image_extractor(const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *arg7);
int gps_extractor(const char *arg1);
int waitForSync();
int extract_number_from_filename(const char *filename);
int compare_files_by_number(const void *a, const void *b);
int upload_bag_files(const char *file);
int upload_files(char* dest);
void change_bag(char *MakeDirectoryBuf, const char *metadata_file, const char *old_txt, const char *target_txt);
void exclude_bag(char *MakeDirectoryBuf, const char *metadata_file);

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
                  char *MakeDirectoryBuf);

void route_to_json(const char *filename,
                  int pedestrian_density,
                  int traffic_density,
                  int zones,
                  int road_types,
                  int intersections,
                  bool roundabouts,
                  char *MakeDirectoryBuf);
#endif

