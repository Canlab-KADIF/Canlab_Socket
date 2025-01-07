#ifndef TRIGGER_H
#define TRIGGER_H

#include "util.h"

#define BUFSIZE 1024

typedef struct {
    char MakeDirectoryBuf[BUFSIZE];
    int Trigger;  // 트리거 체크
    int Check;    // 클라이언트 로깅 체크
    int Count;    // 트리거 된 파일 체크
    int Finish;   // 트리거 된 파일 전송 체크
} TRIGGER_CLIENT;

void display_banner(const char* message);
void handle_trigger(const char* message);

#endif

