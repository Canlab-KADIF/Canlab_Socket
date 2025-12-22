#ifndef NETWORK_H
#define NETWORK_H

#include "util.h"

#define ADS_IPADDR "127.0.0.1" // 서버 주소 설정

#define ADS_PORT 12345
#define GUI_PORT 12346

#define MAX_STR 1024

int connect_to_server(const char* ip, int port);
void* receive_chat(void* arg);
void* send_chat(void* arg);

int read_field(int sockfd, const char *field_name, char *str_result, int *int_result, bool *bool_result);
int read_field_special(int sockfd, const char *field_name, char *str_result, char *array);

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
                 int *triggered_cause);

int receive_route(int sockfd,
                  int *pedestrian_density,
                  int *traffic_density,
                  char *special_vehicles,
                  int *zones,
                  int *road_types,
                  int *intersections,
                  bool *roundabouts,
                  char *special_structures);
#endif
