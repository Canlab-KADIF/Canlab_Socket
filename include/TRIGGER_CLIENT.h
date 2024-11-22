#ifndef TRIGGER_CLIENT_H
#define TRIGGER_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>	
#include <errno.h>	
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/statvfs.h>
#include <time.h>

#define PORT 12345
#define IPADDR "127.0.0.1"
#define BUFSIZE 1024

class TRIGGER_CLIENT
{

private:

public:	

	char ssd[20] = {0, };
	int  ssd_check = 0;
	int  ssd_storage = 0;


	char DirBuf[128] = {0, };
	char MakeDirectoryBuf[128] = {0, };

	int Trigger = 0;
	int FileCount = 0;
	int bagfile = 0;
	int bagfile_t = 0;
	int count = 1;
	int check = 0;
	

};
#endif
