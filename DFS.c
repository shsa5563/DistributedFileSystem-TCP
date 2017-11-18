/**************************************************************************
****Author: Shekhar 
****C socket server example, handles multiple clients using threads
****reference:http://www.binarytides.com/server-client-example-c-sockets-linux/
****from client type the below command in terminal
**** (echo -en "GET /index.html HTTP/1.1\r\nHost: localhost\r\n"; sleep 10) | telnet 127.0.0.1 8888
**************************************************************************/

#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h> //write
#include <pthread.h> //for threading , link with lpthread
#include <errno.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include "util.h"


/***DataStructure Declaration ***/
typedef struct tcpRecvPacketInfo {
    char* fileName;
    char* httpVersion;
    char* hostName;
    int connectionALiveFlag;
} tcpRecvPacketInfo;

typedef struct tcpReplyPacketInfo {
    char httpMethod[MINLIMIT];
    char fileName[MINLIMIT];
    char fileType[MINLIMIT];
    char httpVersion[MINLIMIT];
    char hostName[MINLIMIT];
    int  connectionAliveFlag;
    int  contentLength;
    int  isPostMethod;
    int  postContentLength;
    char postContent[MAXLIMIT]; 
    int  errorFlag;
    char errorMessage[MINLIMIT];
    char errorReason[MINLIMIT];
} tcpReplyPacketInfo;

/***Global Variables ***/
char* _global_UserNameAndPassword[MINLIMIT][2];
char* _global_Path;
unsigned int _global_Counter=0;

/***Declaration of Functions***/
void* connection_handler(void*); //the thread function
char* getFileNameFromDirectoryString(char* token);
int readWebConfig();
int compare_filename_ext(const char* filename, char* replyfileType);
int getFileFromServer(int sock, char * filePathInput, int fileSize);
int putFileToServer( int sock , packetInfo * tcpPacketInfo);

int getFileFromServer(int sock, char * filePathInput, int fileSize)
{
    printf("The file name is :%s",filePathInput);
     packetInfo * tcpPacketInfo = (packetInfo *) malloc(sizeof(packetInfo));
    int fileSizeCounter = 0, readBytes = 0, sendBytes = 0;
    char * fileName = (char *)malloc(sizeof(strlen(filePathInput)));
    memset(fileName,'\0', strlen(filePathInput));
    FILE* fileOpen = NULL;
    fileOpen = fopen(filePathInput, "rb+"); 
    size_t length = sizeof(packetInfo);	
        memset(tcpPacketInfo, '\0', length);
        sprintf(tcpPacketInfo->command,"%s",GET_COMMAND); 
        if((fileName = fileNameFromDirectoryString(filePathInput))==NULL) 
	{ 
          printf("THe file Name dosent exists in DIrectory String\n");
	  fileName = filePathInput;
	}
	sprintf(tcpPacketInfo->fileName,"%s",fileName);
                tcpPacketInfo->fileSize = fileSize;
		send(sock, tcpPacketInfo, length, MSG_NOSIGNAL); printf("here1\n"); 
	       
		        int readLimit;  if(fileSize<MAXLIMIT)readLimit = fileSize;else readLimit=MAXLIMIT;
		fileSizeCounter = 0, readBytes = 0, sendBytes = 0;
		while (fileSizeCounter < fileSize) {
			printf("here2\n");
			memset(tcpPacketInfo, '\0', length);
		        sprintf(tcpPacketInfo->command,"%s",GET_COMMAND); 
			sprintf(tcpPacketInfo->fileName,"%s",fileName);
			tcpPacketInfo->fileSize = fileSize;
			tcpPacketInfo->dataFlag = 1;
	 		readBytes = fread(tcpPacketInfo->data, 1, readLimit, fileOpen);printf("here3\n");
		        tcpPacketInfo->readBytes= readBytes;
			//Send the message back to client
			sendBytes = send(sock, tcpPacketInfo, length, MSG_NOSIGNAL);printf("here4\n");
			fileSizeCounter += readBytes;printf("fileSizeCounter:%d fileChunkSize :%d\n", fileSizeCounter, fileSize);
	        } 
    fclose(fileOpen);
	free(tcpPacketInfo);
}
/***********************************************************************************
*** readWebConfig function : parses the ws.conf file and put the extracted values to global variables
*** the global variables are used to configure the webserver; thi fuction is called once after the main starts
***********************************************************************************/

int readWebConfig()
{
    FILE* webConfig;

    webConfig = fopen(WEBCONFIG, "rb");
    if (webConfig == NULL) {
        perror("WebConfig File error:");
	exit(-1);
    }
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char temp[MINLIMIT] = { 0 };
    _global_Counter = 0;
    while ((read = getline(&line, &len, webConfig)) != -1) {
        DEBUG(printf("Retrieved line of length %zu :\n", read));
        DEBUG(printf("%s", line));
        sscanf(line, "%s %s", (_global_UserNameAndPassword[_global_Counter][0] = (char*)malloc(strlen(line))), (_global_UserNameAndPassword[_global_Counter][1] = (char*)malloc(strlen(line))));
        printf("_global_UserNameAndPassword:%s %s\n", _global_UserNameAndPassword[_global_Counter][0], _global_UserNameAndPassword[_global_Counter][1]);
        _global_Counter++;
    }
}

/***********************************************************************************
*** main function : start point of the webserver
*** doesnot take any arguments for now, but for furture use the parameters are mentioned
***********************************************************************************/

int main(int argc, char* argv[])
{
    int socket_desc, client_sock, c, *new_sock, optval=1;
    struct sockaddr_in server, client;
    if(argc != 3)
    {
        printf("Please specify the Port\nExample: ./dfs DF1 10001\n");
        exit(0);
    }
    if (!readWebConfig()) // using NOt since it is faster that comparator operation
    {
        printf("Error in Web Config, Please restart your webserver again\n");
        exit(0);
    }

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
    }
    puts("Socket created");
    
    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, 
                   (const void *)&optval , sizeof(int)) < 0)
        return -1;

    _global_Path = (char *)malloc(sizeof(20));
    sprintf(_global_Path,"./%s", argv[1]);
    struct stat st = {0};

    if (stat(_global_Path, &st) == -1) {
	    mkdir(_global_Path, 0777);
    }


    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(strToInteger(argv[2]));

    //Bind
    if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc, 10);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while ((client_sock = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c))) {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void*)new_sock) < 0) {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL); //dont join as pthread_detach is used.
        puts("Handler assigned");
	//sem_wait(&AcceptConnection);
    }

    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }

    return 0;
}

/***********************************************************************************
*** sendFileToClient function : Reads the file in chunks and sends to the client
*** sock: the socket value from which the tcp connection is extablished - through which the file will be sent 
*** fileSize: the filesize of the file which is being sent 
*** fileName : the name of the file
***********************************************************************************/

void sendFileToClient(int sock, int fileSize, char* fileName)
{
    char buf[MAXLIMIT];
    memset(buf, '\0', MAXLIMIT);
    DEBUG(printf("HERE16\n"));
    int fileSizeCounter = 0, readBytes = 0, sendBytes = 0;
    FILE* fileOpen = NULL;
    fileOpen = fopen(fileName, "r"); //501error
    while (fileSizeCounter < fileSize) {
        readBytes = fread(buf, 1, MAXLIMIT, fileOpen);
        //Send the message back to client
        sendBytes = write(sock, buf, readBytes);
        fileSizeCounter += sendBytes;
        memset(buf, '\0', MAXLIMIT);
    }

    fclose(fileOpen);
}

int checkUserNamePassword(char * userName, char* password)
{
int counter = 0;
while(counter <_global_Counter )
{
	if((strcmp(_global_UserNameAndPassword[counter][0], userName)==0)&&(strcmp(_global_UserNameAndPassword[counter][1], password)==0))
	return 0 ;
	counter ++;
}
return -1;printf("here1.6\n");
}

int putFileToServer( int sock , packetInfo * tcpPacketInfo)
{
	char fullFilePath[MINLIMIT]; 
	int errorFlag=0;
	FILE* fileOpen = NULL;
	int length = sizeof(packetInfo);
	memset(fullFilePath, '\0', MINLIMIT);
	int fileSize = tcpPacketInfo->fileSize;
	int fileSizeCounter = 0, readBytes = 0, writtenBytes = 0;
	sprintf(fullFilePath,"%s/%s/%s",_global_Path,tcpPacketInfo->username, tcpPacketInfo->folderName);
	printf("here1.0\n");
	if(!(checkUserNamePassword(tcpPacketInfo->username, tcpPacketInfo->pasword)!=0))
        {
	   struct stat st = {0};
	
 	   if (stat(fullFilePath, &st) == -1) {
	       if(mkdir(fullFilePath, 0777)!=0)
		{
			tcpPacketInfo->errorFlag = errorFlag= 1;
			sprintf(tcpPacketInfo->errorMessage, "The folder could not be created at Server: %s\n",fullFilePath);
			printf("%s",tcpPacketInfo->errorMessage);
		}
   	 }
	sprintf(fullFilePath,"%s/%s",fullFilePath,tcpPacketInfo->fileName);
 	fileOpen = fopen(fullFilePath, "wb+"); //501error
	}else
	{printf("here1.2\n");
		tcpPacketInfo->errorFlag = errorFlag= 1;
		sprintf(tcpPacketInfo->errorMessage, "Invalid UserName and/Or Password \n");
		printf("%s",tcpPacketInfo->errorMessage);
	}
	send(sock, tcpPacketInfo, length, MSG_NOSIGNAL); printf("here1.3\n"); 
	printf("tcpPacketInfo->fileName : %s  fullFilePath: %s",tcpPacketInfo->fileName,fullFilePath);
	
	if(!(errorFlag!=0)){
		while (fileSizeCounter < fileSize) {
			memset(tcpPacketInfo,'\0', sizeof(tcpPacketInfo));
			readBytes = read( sock , tcpPacketInfo, length);
			writtenBytes = fwrite(tcpPacketInfo->data, 1, tcpPacketInfo->readBytes, fileOpen);
			printf("****** fileSizeCounter :%d   fileSize :%d\n",fileSizeCounter, fileSize);
			fflush(fileOpen);
			fileSizeCounter += writtenBytes;
		}
        fclose(fileOpen);
	}
return 0;
}
/***********************************************************************************
*** connection_handler function : handler for every thread created/forked
*** socket_desc : is the socket ID through which the connection was established
***********************************************************************************/

void* connection_handler(void* socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    pthread_detach(pthread_self());
    int read_size;
    char *message, buf[MAXLIMIT];
    memset(buf, '\0', MAXLIMIT);
    
    packetInfo * tcpPacketInfo = (packetInfo *)malloc(sizeof(packetInfo));
    int packetLength = sizeof(packetInfo);
    char * filePath = (char * )malloc(MINLIMIT);
    while(1){
            //Receive a message from client
	    memset(filePath,'\0',MINLIMIT);
            if ((read_size = recv(sock, tcpPacketInfo, packetLength, 0)) > 0) {
		 if(!tcpPacketInfo->dataFlag){
                           if (strcmp(tcpPacketInfo->command, PUT_COMMAND) == 0){printf("here1.5\n");
				  putFileToServer(sock , tcpPacketInfo);
				printf("put command completed\n");
			    }
			    else if (strcmp(tcpPacketInfo->command, GET_COMMAND) == 0) {
				sprintf(filePath,"%s/%s/%s",_global_Path,tcpPacketInfo->username, tcpPacketInfo->folderName);
				printf("get command selected filePath: %s fileNAme: %s\n",filePath,tcpPacketInfo->fileName);
				int fileExists =0;
				char * fileArry[2];
				char buffer[MINLIMIT];
				fileArry[0] = (char*)malloc(MINLIMIT);memset(fileArry[0],'\0',(MINLIMIT));
				fileArry[1] = (char*)malloc(MINLIMIT);memset(fileArry[1],'\0',(MINLIMIT));
				show_dir_content(filePath, tcpPacketInfo->fileName,  &fileExists, fileArry);
				printf("get command selected filePath: %s fileNAme: %s\n",filePath,tcpPacketInfo->fileName);
				int counter=0, errorFlag=0;
				if(checkUserNamePassword(tcpPacketInfo->username, tcpPacketInfo->pasword)!=0)
        			{
					tcpPacketInfo->errorFlag = errorFlag= 1;
					sprintf(tcpPacketInfo->errorMessage, "Invalid UserName and/Or Password \n");
					printf("%s",tcpPacketInfo->errorMessage);

				}

			        memset(tcpPacketInfo, '\0', packetLength);		
				tcpPacketInfo->readBytes= fileExists;
				sprintf(tcpPacketInfo->data, "%s *** %s",  fileArry[0], fileArry[1] );
				printf("tcpPacketInfo->username : %s tcpPacketInfo->pasword :%s\n",tcpPacketInfo->username, tcpPacketInfo->pasword);
				
				send(sock, tcpPacketInfo, packetLength, MSG_NOSIGNAL); printf("here1\n"); 

  				while((fileExists!=0)&&(counter<fileExists)&&(!(errorFlag!=0)))
				{						
					printf("The file was found:%s\n",fileArry[counter]);
					FILE* fileOpen = NULL;
					memset(buffer,'\0',MINLIMIT);printf("here1\n");
					sprintf(buffer, "%s", fileArry[counter]);printf("here2\n");
					printf("The buffer file Name is :%s", buffer);printf("here3\n");
					 fileOpen = fopen(buffer, "rb+"); //501error
			       	       printf("here4\n");
					  if (fileOpen == NULL) {
				  	    printf("Error in FIle\n");
					 }
					 fseek(fileOpen, 0, SEEK_END);
					 int fileSize = ftell(fileOpen);
					 rewind(fileOpen);
					 fclose(fileOpen);
					if(getFileFromServer(sock,fileArry[counter], fileSize)==0)
					    {
					    }counter++;
				}//else printf("The file was not found\n");
				//free(fileArry);
			    }
			    else if (strcmp(tcpPacketInfo->command, LIST_COMMAND) == 0) {
				printf("ls command selected\n");
				sprintf(filePath,"%s/%s/%s",_global_Path,tcpPacketInfo->username, tcpPacketInfo->folderName);
				printf("get command selected filePath: %s fileNAme: %s\n",filePath,tcpPacketInfo->fileName);
				int fileExists =0;
				int counter=0, errorFlag=0;
				if(checkUserNamePassword(tcpPacketInfo->username, tcpPacketInfo->pasword)!=0)
        			{
					tcpPacketInfo->errorFlag = errorFlag= 1;
					sprintf(tcpPacketInfo->errorMessage, "Invalid UserName and/Or Password \n");
					printf("%s",tcpPacketInfo->errorMessage);

				}else
				{
					sprintf(tcpPacketInfo->data, "%s", listFileInCurrentDirectory(filePath,&fileExists));
					printf("tcpPacketInfo->data :%s\n",tcpPacketInfo->data);
					if(!(fileExists!=0))
					{
						tcpPacketInfo->errorFlag = errorFlag= 1;
						sprintf(tcpPacketInfo->errorMessage, "No Files FOund in the path %s\n",filePath);
						printf("%s",tcpPacketInfo->errorMessage);
					}
				}
		
				send(sock, tcpPacketInfo, packetLength, MSG_NOSIGNAL); printf("here1\n"); 
			    }
			    else {
				printf("other command selected\n");
				//otherCommandToServer(buf, &servaddr);
			    }			
		}	
            }
    }		
    //Free the socket pointer
    free(socket_desc);
    close(sock);
    pthread_exit(0);
    return NULL;
}

