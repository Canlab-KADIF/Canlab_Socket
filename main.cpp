#include "include/TRIGGER_CLIENT.h"

void *do_receive_chat(void *arg);
void *do_send_chat(void *arg);
void *check_filecount(void *arg);
void *move_bagfile(void *arg);
int MakeDirectory();
void catch_sigint(int sig);

pthread_t c_thread, d_thread, s_thread, m_thread;

TRIGGER_CLIENT T;

int main(){
	struct sockaddr_in c_addr;
	int c_socket; 
	int ret;
 	int result;
	char rcvBuffer[BUFSIZE];//서버에서 보내준 메세지를 저장하는 변수
	
	signal(SIGINT, catch_sigint);
	
	int ros_source1 = system("source /opt/ros/galactic/setup.bash");
	int ros_source2 = system("source /home/canlab/Canlab_ROS2/install/setup.bash");
	
	int lidar_server = system("/home/canlab/Canlab_ROS2/server.sh &");
	sleep(3);
	int lidar_bridge = system("/home/canlab/Canlab_ROS2/bridge.sh &");
	sleep(3);
	int ros_start = system("/home/canlab/Canlab_ROS2/run.sh &");
	sleep(1);
	int bag_start = system("/home/canlab/Canlab_ROS2/rosbag_record.sh &");

	printf( "\n+--------------------------------------------+ \n");
	printf( "|                                            |\n");
	printf( "|     D A S H - B O A R D   S T A R T        |\n");	
	printf( "|                                            |\n");
	printf( "+--------------------------------------------+\n\n");
		
	printf( "\n+--------------------------------------------+ \n");
	printf( "|                                            |\n");
	printf( "|     CREATE SAVE TRIGGER FOLDER NAME        |\n");	
	printf( "|                                            |\n");
	printf( "+--------------------------------------------+\n\n");	
	
	//printf("Select Name: ");
	//fgets(T.DirBuf, BUFSIZE, stdin);
	//T.DirBuf[strlen(T.DirBuf) - 1] = '\0'; //개행 문자 제거 
	sprintf(T.MakeDirectoryBuf, "/home/canlab/%s/" , "BAGTRG");
	
	ret = MakeDirectory();	
	
	if(ret == 0){
		//1. 클라이언트 소켓 생성
		c_socket = socket(PF_INET, SOCK_STREAM, 0); //서버와 동일한 설정으로 생성
		//2.소켓 정보 초기화
		memset(&c_addr, 0, sizeof(c_addr));
		c_addr.sin_addr.s_addr = inet_addr(IPADDR); //접속할 IP 설정 (127.0.0.1)
		c_addr.sin_family = AF_INET;
		c_addr.sin_port = htons(PORT);

		while(1){
			//3. 서버에 접속
			if(connect(c_socket, (struct sockaddr *) &c_addr, sizeof(c_addr)) == -1){
				//서버 접속에 실패하면 
				printf("Wait Server\n"); //Cannot connect 메세지 출력
			}else{
			 	printf("Connect Success\n");
				//close(c_socket);
			 	break;
			}					
			sleep(1);
		}

		pthread_create(&d_thread,NULL,do_receive_chat,(void *)&c_socket);
		pthread_create(&s_thread,NULL,do_send_chat,(void *)&c_socket);
		pthread_create(&c_thread,NULL,check_filecount,NULL);
		pthread_create(&m_thread,NULL,move_bagfile,NULL);

		pthread_join(d_thread, NULL);
		pthread_join(s_thread, NULL);
		pthread_join(c_thread, NULL);
		pthread_join(m_thread, NULL);	
	}else{
		printf("Make Directory Fail\n");
	}
}

int MakeDirectory()
{
	int ret;
	
        if((ret = mkdir(T.MakeDirectoryBuf, S_IFDIR | S_IRWXU | S_IRWXG | S_IXOTH | S_IROTH)) == -1) { // 디렉토리 존재 시 스킵
       	if(errno == EEXIST){
       		//printf("ALREADY EXIST FOLDER %s\n", T.MakeDirectoryBuf);
       	 	return errno; 
       	}
       	else{
			printf( "\n+--------------------------------------------+ \n");
			printf( "|                                            |\n");
			printf( "|     D A S H - B O A R D   E N D            |\n");
			printf( "|                                            |\n");
			printf( "+--------------------------------------------+\n\n");

			return -1;       	
       	    }         		
	}	
	
	return 0;
}

void *do_receive_chat(void *arg)
{
    char   chatData[BUFSIZE];
    char*  ptr;
    char   s1[][13] = {"control", "perception", "localization", "planning"};
    int    n;
    int    c_socket = *((int *)arg);        // client socket
    while(1) {
        memset(chatData, 0, sizeof(chatData));
	n = read(c_socket, chatData, sizeof(chatData)); 
	//서버에서 보내준 메세지를 chatData 저장하고, 메세지의 길이를 리턴
	//만약 read에 실패하면, -1을 리턴
	if (n < 0){ //read() 함수 실패 
		printf("Read Failed\n");
		return 0;
	}
	chatData[n] = '\0'; //문자열 뒷부분 깨짐 방지
	printf("서버가 보낸 메세지: %s\n", chatData); //서버에서 받은 메세지 출력
	
        ptr = strtok(chatData,":");

	for(int i=0; i<sizeof(s1)/sizeof(s1[0]); i++){
		if(strcmp(ptr, s1[i]) == 0){
			T.Trigger = 1;
		}
	}	
    }
}

void * do_send_chat(void *arg)
{
	char chatData[BUFSIZE] = "logging";
	char buf[100];
	int n;
	int c_socket = *((int *) arg);	
	while(1) {
		if(T.Trigger == 1 && T.check == 0){
			memset(buf, 0, sizeof(buf));
			write(c_socket, chatData, strlen(chatData)); //서버로 채팅 메시지 전달
			T.check = 1;
		}else{ //nothing
		}
	}
}


void *check_filecount(void *arg)
{
        char  buff[BUFSIZE];
        FILE *fp;
        
	while(1){
 
        fp = popen("find /home/canlab/BAG/ -type f | wc -l", "r");
        if (NULL == fp)
        {
               perror("popen() 실패");
        }
 
        while (fgets(buff, 1024, fp)){
               T.FileCount = atoi(buff);
               //printf("FileCount %d\n", T.FileCount);
       }
 
        pclose(fp);
	
	sleep(1);
	}
}

void *move_bagfile(void *arg)
{
        char  buff[BUFSIZE];
        char  commandbuff[BUFSIZE];
        char  cmd_buffer1[BUFSIZE];
        char  cmd_buffer2[BUFSIZE];
        char  cmd_buffer3[BUFSIZE];
        char  cmd_buffer4[BUFSIZE];
        char  cmd_buffer5[BUFSIZE];
        char  cmd_buffer6[BUFSIZE];
        FILE *fp;
 	
	while(1){
	
 	sprintf(commandbuff, "find /home/canlab/BAG/ -type f -exec basename {} .db3 \\; | sort | cut -d '_' -f 2 | sed -n %dp 2>/dev/null", T.FileCount);
 	
        fp = popen(commandbuff, "r");
        if (NULL == fp)
        {
               perror("popen() 실패");
        }
 
        while (fgets(buff, 1024, fp)){
               T.bagfile = atoi(buff);
               //printf("bagfile %d\n",T.bagfile);
	}
 
        pclose(fp);
        
        if(T.Trigger == 1){
        	if(T.count == 1){ 
        		T.bagfile_t = T.bagfile;
	        	T.count = 0;
	        	//printf("bagfile_t %d\n", T.bagfile_t);
           	}
        	if(T.bagfile == (T.bagfile_t + 2)){
        		sprintf(cmd_buffer1, "find /home/canlab/BAG/ -name '*%d.db3' -exec cp {} %s \\;", T.bagfile_t-1, T.MakeDirectoryBuf);
        		system(cmd_buffer1);
        		sprintf(cmd_buffer2, "find /home/canlab/BAG/ -name '*%d.db3' -exec cp {} %s \\;", T.bagfile_t, T.MakeDirectoryBuf);
        		system(cmd_buffer2);
        		sprintf(cmd_buffer3, "find /home/canlab/BAG/ -name '*%d.db3' -exec cp {} %s \\;", T.bagfile_t+1, T.MakeDirectoryBuf);
        		system(cmd_buffer3);
        		printf("MOVE BAG FILE %d %d %d\n", T.bagfile_t-1, T.bagfile_t, T.bagfile_t+1);
        		sprintf(cmd_buffer4, "mv '/home/canlab/BAGTRG/BAG_%d.db3' '/home/canlab/BAGTRG/BAG_TRG%d.db3'", T.bagfile_t-1, T.bagfile_t-1);
        		system(cmd_buffer4);
        		sprintf(cmd_buffer5, "mv '/home/canlab/BAGTRG/BAG_%d.db3' '/home/canlab/BAGTRG/BAG_TRG%d.db3'", T.bagfile_t, T.bagfile_t);
        		system(cmd_buffer5);
        		sprintf(cmd_buffer6, "mv '/home/canlab/BAGTRG/BAG_%d.db3' '/home/canlab/BAGTRG/BAG_TRG%d.db3'", T.bagfile_t+1, T.bagfile_t+1);
        		system(cmd_buffer6);
	        	T.Trigger = 0;
	        	T.count = 1;
			T.check = 0;
        	}
        }
        sleep(1);
        }

}

void catch_sigint(int sig)
{
    char  now_time[20];
    char  copy_metadata[BUFSIZE];
    char  change_bagfolder[BUFSIZE];
    char  change_trgfolder[BUFSIZE];
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(now_time, sizeof(now_time), "%y-%m-%d_%H-%M-%S", t);
    
    sleep(3);
    
    // metadata.yaml 복사
    sprintf(copy_metadata, "find /home/canlab/BAG/ -name '*.yaml' -exec cp {} %s \\;", T.MakeDirectoryBuf);
    system(copy_metadata);
    // BAG폴더 년월일시간_BAG으로 변경
    sprintf(change_bagfolder, "mv '/home/canlab/BAG' '/home/canlab/%s_BAG'", now_time);
    system(change_bagfolder);
    // BAGTRG폴더 년월일시간_TRG으로 변경
    sprintf(change_trgfolder, "mv '/home/canlab/BAGTRG' '/home/canlab/%s_TRG'", now_time);
    system(change_trgfolder);
    
    exit(0);
}
