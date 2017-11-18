/**************************************************************************
****Author: Shekhar 
****C socket client example
****reference:http://www.binarytides.com/server-client-example-c-sockets-linux/
****to execute it, ./DFC Alice.conf
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
    int connectionAliveFlag;
    int contentLength;
    int isPostMethod;
    int postContentLength;
    char postContent[MAXLIMIT];
    int errorFlag;
    char errorMessage[MINLIMIT];
    char errorReason[MINLIMIT];
} tcpReplyPacketInfo;

/***Global Variables ***/
char _global_UserName[MINLIMIT];
char _global_Password[MINLIMIT];
char* _global_ServerIPAndPort[MINLIMIT][3];
int _global_Counter = 0;
serverALive * serverStatus;

/***Declaration of Functions***/
void* connection_handler(void*); //the thread function
char* getFileNameFromDirectoryString(char* token);
int readWebConfig();
int compare_filename_ext(const char* filename, char* replyfileType);
void connectClient( );
int putFileToServer(char * filePathInput, char * serverFolderPath, int fileSize);
int getFileFromServer(char * filePathInput, char * serverFolderPath);
int mergeFileChunks(char * filePathInput);
void listFileAtServer(char * serverFolderPath);


int mergeFileChunks(char * filePathInput)
{
  FILE * fileOpen =NULL;
  fileOpen = fopen(filePathInput, "wb+");
  char buf[MAXLIMIT];
  for(int i=0; i<_global_Counter; i++)
  {
	FILE * fileChunk =NULL;
	memset(buf, '\0', MAXLIMIT);
	sprintf(buf,"%s",filePathInput);
        if(getFileChunkName(buf,i)!=0) printf("THe file pointer requested by user is NULL\n");
        printf("fileChunk name: %s\n",buf);
	if(!(fileExist(buf)!=0))
	{ 
		return -1;
		continue;
	}
	fileChunk = fopen(buf, "rb+");
	if(!(fileChunk!=NULL))
	{
		//return -1;
	}
	int readBytes=0,writtenBytes=0;
	
        while(!feof(fileChunk) && !ferror(fileChunk))
	{
		memset(buf, '\0', MAXLIMIT);
		readBytes = fread(buf, 1, MAXLIMIT, fileChunk);printf("here3\n");
		writtenBytes = fwrite(buf, 1,readBytes, fileOpen);
		
	}       
	fclose(fileChunk);
  }
  fclose(fileOpen);
  return 0;
}

int getFileFromServer(char * filePathInput, char * serverFolderPath)
{
    connectClient();
    packetInfo * tcpPacketInfo = (packetInfo *)malloc(sizeof(packetInfo));
    int length = sizeof(packetInfo);
    int read_size =0, errorFlag=0;
    FILE* fileOpen = NULL;
    for(int i=0; i<_global_Counter; i++)
    {
        memset(tcpPacketInfo, '\0', length);
        sprintf(tcpPacketInfo->command,"%s",GET_COMMAND); 
	    sprintf(tcpPacketInfo->fileName,"%s",filePathInput);
		sprintf(tcpPacketInfo->username , "%s",_global_UserName);
		sprintf(tcpPacketInfo->pasword , "%s",_global_Password);
		sprintf(tcpPacketInfo->folderName , "%s",serverFolderPath);
printf("tcpPacketInfo->username : %s tcpPacketInfo->pasword :%s\n",tcpPacketInfo->username, tcpPacketInfo->pasword);
		if(serverStatus->status[i])
		{	
			if(send(serverStatus->sockID[i], tcpPacketInfo, length, MSG_NOSIGNAL)<0)
			 { 

				serverStatus->status[i] = 0;
				connectClient();
			 }
			else {
				memset(tcpPacketInfo, '\0', length);
				if ((read_size = recv(serverStatus->sockID[i], tcpPacketInfo, length, 0)) > 0) {
					errorFlag = tcpPacketInfo->errorFlag;
					if(errorFlag)printf("Error Message From Server: %s\n", tcpPacketInfo->errorMessage);
					if((!tcpPacketInfo->dataFlag)&&(!(errorFlag!=0))){
							int fileCounter =0, fileMaxNum = tcpPacketInfo->readBytes; 
						//printf("ReadBytes : %d %d\n", tcpPacketInfo->readBytes, fileMaxNum);
						while(fileCounter < fileMaxNum){
							memset(tcpPacketInfo, '\0', length);
							if ((read_size = recv(serverStatus->sockID[i], tcpPacketInfo, length, 0)) > 0) {
								int fileSizeCounter = 0, readBytes = 0, writtenBytes = 0;
								printf("The file name is :%s Size:%d\n",tcpPacketInfo->fileName, tcpPacketInfo->fileSize );				    
								fileOpen = fopen(tcpPacketInfo->fileName, "wb+"); //501error
								while (fileSizeCounter < tcpPacketInfo->fileSize) { 
									memset(tcpPacketInfo,'\0', length);
									readBytes = read( serverStatus->sockID[i] , tcpPacketInfo, length);
									
									encryptDecryptData(tcpPacketInfo->data,tcpPacketInfo->readBytes, _global_Password, strlen(_global_Password));
									writtenBytes = fwrite(tcpPacketInfo->data, 1, tcpPacketInfo->readBytes, fileOpen); printf("ReadBytes : %d writtenBytes:%d\n", tcpPacketInfo->readBytes, writtenBytes);
									fflush(fileOpen);
									fileSizeCounter += writtenBytes;
								}
								printf("The fie is closing: %s\n", tcpPacketInfo->fileName)	;
									fclose(fileOpen);
							}
							fileCounter++;
						}	
					}
				}	
			}
		}
	}
	free(tcpPacketInfo);
        if(mergeFileChunks(filePathInput)!=0)
	 {
            printf("The file %s is Incomplete, All the chucks could not be revtrived\n", filePathInput);
	 }
	else
	{
		printf("The file %s is succesfully received\n",filePathInput);
	}
    return 0;
}



void listFileAtServer(char * serverFolderPath)
{
     connectClient();
     packetInfo * tcpPacketInfo = (packetInfo *) malloc(sizeof(packetInfo));
     char buf[MAXLIMIT];
     int read_size =0, errorFlag=0; 
     memset(buf, '\0', MAXLIMIT);
     size_t length = sizeof(packetInfo);
     for(int i=0; i<_global_Counter; i++)
     {
        memset(tcpPacketInfo, '\0', length);
        sprintf(tcpPacketInfo->command,"%s",LIST_COMMAND); 
	sprintf(tcpPacketInfo->username , "%s",_global_UserName);
	sprintf(tcpPacketInfo->pasword , "%s",_global_Password);
	sprintf(tcpPacketInfo->folderName , "%s",serverFolderPath);
printf("tcpPacketInfo->username : %s tcpPacketInfo->pasword :%s\n",tcpPacketInfo->username, tcpPacketInfo->pasword);
		if(serverStatus->status[i])
		{	
			if(send(serverStatus->sockID[i], tcpPacketInfo, length, MSG_NOSIGNAL)<0){ 
				serverStatus->status[i] = 0;
				connectClient();
			 }else{
			memset(tcpPacketInfo, '\0', length);
			if ((read_size = recv(serverStatus->sockID[i], tcpPacketInfo, length, 0)) > 0) {
				errorFlag = tcpPacketInfo->errorFlag;
				if(errorFlag){
					printf("Error Message From Server: %s\n", tcpPacketInfo->errorMessage);
					return;
				}
				if((!tcpPacketInfo->dataFlag)&&(!(errorFlag!=0))){
					printf("tcpPacketInfo->data : %s\n",tcpPacketInfo->data);
					sprintf(buf,"%s%s%s",buf,FILE_DELIMITER,tcpPacketInfo->data);
				}
			}
			}
		}
      }	
      printf("All the files :%s\n",buf);
      
	char * ptrArry[200];
	int counter =0;
	  char * pch;
	  pch = strtok (buf,FILE_DELIMITER);
	  while (pch != NULL)
	  {
	    ptrArry[counter] = (char *)malloc(200); 
	    printf ("%s\n",pch);
	    strcpy(ptrArry[counter],pch);
	    pch = strtok (NULL, FILE_DELIMITER);
	    counter++;
	  }

	/*for(int i =0; i< counter; i++)
	{
	printf("ptrArry[%d]: %s\n",i,ptrArry[i]);
	}
	printf("*******************************************\n");*/
	char temp[200]; memset(temp,'\0',200);
	 for (int i = 0; i < counter; i++) {
	      for (int j = 0; j < counter - 1; j++) {
		 if (strcmp(ptrArry[j], ptrArry[j + 1]) > 0) {
		    strcpy(temp, ptrArry[j]);
		    strcpy(ptrArry[j], ptrArry[j + 1]);
		    strcpy(ptrArry[j + 1], temp);
		 }
	      }
	   }


	/*for(int i =0; i< counter; i++)
	{
	printf("ptrArry[%d]: %s\n",i,ptrArry[i]);
	}

	printf("*******************************************\n");*/


	  int dst = 0, i,j=0;
	   for (i = 1; i < counter; ++i) {
	       if (strcmp (ptrArry[dst], ptrArry[i]) != 0){
		   ptrArry[++dst] = ptrArry[i];
		   j++;
		   }
	   }

	for(int i =0; i<=j; i++)
	{
	printf("ptrArry[%d]: %s\n",i,ptrArry[i]);
	
	} 

	int counter1=0, inComplete=0;
for(int i =0; i<=j; i++)
{
	if(!(i^j))
	{	
		inComplete= j;
		break;
	}
	if(!(strncmp(ptrArry[i],ptrArry[i+1], (strlen(ptrArry[i])-1))!=0))
	{
		
		counter1=counter1+1;
		printf("counter in if  :%d , %s, %s\n",counter1,ptrArry[i],ptrArry[i+1]);fflush(stdout);
			
	}
	else
	{

		if(counter1==3)
		{
			printf("counter  in else:%d\n",counter1);fflush(stdout);
		 	int check = strlen(ptrArry[i])-2;
			printf("check : %d\n",check );
		 	ptrArry[i][check]='\0';
		 	printf("%s\n",ptrArry[i]);
		}
		else
		{
		 	int check = strlen(ptrArry[i])-2;
			printf("check : %d\n",check );
			ptrArry[i][check]='\0';
		 	printf("%s - Incomplete\n",ptrArry[i]);
		}
		counter1 =0;
	}
	

}
        
if(counter1<3)
{
		 	int check = strlen(ptrArry[inComplete])-2;
			printf("check : %d\n",check );
			ptrArry[inComplete][check]='\0';
		 	printf("%s - Incomplete\n",ptrArry[inComplete]);
}else
{
int check = strlen(ptrArry[inComplete])-2;
			printf("check : %d\n",check );
		 	ptrArry[inComplete][check]='\0';
		 	printf("%s\n",ptrArry[inComplete]);
}
}
/***********************************************************************************
*** putFileToServer function : puts the file to the server
***********************************************************************************/
int putFileToServer(char * filePathInput,char * serverFolderPath, int fileSize)
{
	if(fileSize<=0) 
	{
		printf("File Size is 0\n");return 0;
	}
     connectClient();
     int modVal = calculateMD5SumMod4(filePathInput);
     int fileChunkSize = _log2((unsigned int)(fileSize/NUMBER_OF_SERVERS));
     packetInfo * tcpPacketInfo = (packetInfo *) malloc(sizeof(packetInfo));
     char buf[MAXLIMIT];
    memset(buf, '\0', MAXLIMIT);
    int fileSizeCounter = 0, readBytes = 0, sendBytes = 0;
    FILE* fileOpen = NULL;
    fileOpen = fopen(filePathInput, "rb+"); //501error
    int totalCounter = 0;
    size_t length = sizeof(packetInfo);
    int serverVisit =0;
    //printf("Mod Value: %d\n",modVal);modVal=0;
	DEBUG(for(int i=0;i<4;i++) printf("sockID :%d status :%d\n",serverStatus->sockID[i],serverStatus->status[i]));
    for(int i=modVal; serverVisit<_global_Counter; i++)
    {  	
	if(i==_global_Counter)i=0; 
	
	int j =((i-1)<0?3:i-1);
        memset(tcpPacketInfo, '\0', length);
        sprintf(tcpPacketInfo->command,"%s",PUT_COMMAND); 
	sprintf(tcpPacketInfo->fileName,"%s.%d",filePathInput,i);
	if(serverStatus->status[i]||serverStatus->status[j])
	{	
                if(serverVisit==(_global_Counter-1)) fileChunkSize = fileSize-totalCounter;
                //printf("index : %d, filechunkSize : %d\n", i, fileChunkSize);
                tcpPacketInfo->fileSize = fileChunkSize;
		sprintf(tcpPacketInfo->folderName , "%s",serverFolderPath);
		sprintf(tcpPacketInfo->username , "%s",_global_UserName);
		sprintf(tcpPacketInfo->pasword , "%s",_global_Password);
		printf("The I value :%d ; The J value :%d sockID i:%d sockID j:%d\n",i,j, serverStatus->sockID[i],serverStatus->sockID[j]);
		if(serverStatus->status[i]) 
		{
			if(send(serverStatus->sockID[i], tcpPacketInfo, length, MSG_NOSIGNAL)<0)
			{ 
				serverStatus->status[i] = 0;
				connectClient();
			 }
		}
		int read_size =0, errorServer1=0, errorServer2=0;
         	
		if(serverStatus->status[j]) 
		{
			if(send(serverStatus->sockID[j], tcpPacketInfo, length, MSG_NOSIGNAL)<0)
				{ 
				serverStatus->status[i] = 0;
				connectClient();
			 }else{
			memset(tcpPacketInfo, '\0', length); 
		 	if ((read_size = recv(serverStatus->sockID[j], tcpPacketInfo, length, 0)) > 0) { 
				errorServer2 = tcpPacketInfo->errorFlag;
				if(errorServer2)printf("Error From Server %d : %s\n", j, tcpPacketInfo->errorMessage);
			}
			}
		}
		if(serverStatus->status[i]) 
		{
			memset(tcpPacketInfo, '\0', length); 
			if ((read_size = recv(serverStatus->sockID[i], tcpPacketInfo, length, 0)) > 0) { 
				errorServer1 = tcpPacketInfo->errorFlag;
				if(errorServer1)printf("Error From Server %d: %s\n",i, tcpPacketInfo->errorMessage);
			}
		}

		fileSizeCounter = 0, readBytes = 0, sendBytes = 0;
		while ((fileSizeCounter < fileChunkSize)&&(!(errorServer1||errorServer2))) {
			//printf("here2\n");
			memset(tcpPacketInfo, '\0', length);
		        sprintf(tcpPacketInfo->command,"%s",PUT_COMMAND); 
			sprintf(tcpPacketInfo->fileName,"%s.%d",filePathInput,serverVisit);
			sprintf(tcpPacketInfo->folderName , "%s",serverFolderPath);
			sprintf(tcpPacketInfo->username , "%s",_global_UserName);
			sprintf(tcpPacketInfo->pasword , "%s",_global_Password);
			printf("THe file name :%s\n",tcpPacketInfo->fileName);
			tcpPacketInfo->fileSize = fileChunkSize;
		        int readLimit;  if(fileChunkSize<MAXLIMIT)readLimit = fileChunkSize;else readLimit=MAXLIMIT;
			tcpPacketInfo->dataFlag = 1;
	 		readBytes = fread(tcpPacketInfo->data, 1, readLimit, fileOpen);//printf("here3\n"); 
			encryptDecryptData(tcpPacketInfo->data,readBytes, _global_Password, strlen(_global_Password));
		        tcpPacketInfo->readBytes= readBytes;
			//Send the message back to client
			if((!(errorServer1!=0))&&(serverStatus->status[i]))
			{
				sendBytes = send(serverStatus->sockID[i], tcpPacketInfo, length, MSG_NOSIGNAL);
				if(sendBytes<0){ 

				serverStatus->status[i] = 0;
				connectClient();
			 }
			}
			if((!(errorServer2!=0))&&(serverStatus->status[j]))
			{
				sendBytes = send(serverStatus->sockID[j], tcpPacketInfo, length, MSG_NOSIGNAL);
				if(sendBytes<0){ 
				serverStatus->status[i] = 0;
				connectClient();
			 }
			}
			fileSizeCounter += readBytes;printf("fileSizeCounter:%d fileChunkSize :%d\n", fileSizeCounter, fileChunkSize);
	        } 
		totalCounter+= fileSizeCounter;
	        printf("done with index : %d\n", i);
	} 
	serverVisit++;
    }
    fclose(fileOpen);
	free(tcpPacketInfo);
   
     return 0;
}

/***********************************************************************************
*** readWebConfig function : parses the ws.conf file and put the extracted values to global variables
*** the global variables are used to configure the webserver; thi fuction is called once after the main starts
***********************************************************************************/

int readWebConfig(char* confFileName)
{
    FILE* webConfig;
    char* tempBuf = (char*)malloc(MINLIMIT);
    memset(tempBuf, '\0', MINLIMIT);
    sprintf(tempBuf, "%s%s", WEBCONFIG_CLIENT, confFileName);
    webConfig = fopen(tempBuf, "rb");
    if (webConfig == NULL) {
        perror("WebConfig File error:");
        exit(-1);
    }
    free(tempBuf);
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char temp[MINLIMIT] = { 0 };
    int counter = 0;
    while ((read = getline(&line, &len, webConfig)) != -1) {
        DEBUG(printf("Retrieved line of length %zu :\n", read));
        DEBUG(printf("%s", line));
        if (strstr(line, USERNAME) != NULL) //Listen
        {
            sscanf(line, "%*s %s", _global_UserName);
            DEBUG(printf("_global_UserName:%s\n", _global_UserName));
            continue;
        }
        if (strstr(line, PASSWORD) != NULL) //Listen
        {
            sscanf(line, "%*s %s", _global_Password);
            DEBUG(printf("_global_Password:%s\n", _global_Password));
            continue;
        }
        if (strstr(line, SERVER) != NULL) {
            sscanf(line, "%*s %s %s %s", (_global_ServerIPAndPort[counter][0] = (char*)malloc(strlen(line))), (_global_ServerIPAndPort[counter][1] = (char*)malloc(strlen(line))), (_global_ServerIPAndPort[counter][2] = (char*)malloc(sizeof(int))));
            DEBUG(printf("_global_ServerIPAndPort:%s %s\n", _global_ServerIPAndPort[counter][0], _global_ServerIPAndPort[counter][1]));
            counter++;
        }
    }
    _global_Counter = counter;
}


void connectClient()
{
    for (int i = 0; i < _global_Counter; i++) {
       if(!serverStatus->status[i]){
        struct sockaddr_in servaddr;
        if ((serverStatus->sockID[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("The socket could not be created\n");
            exit(-1);
        }
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(strToInteger(_global_ServerIPAndPort[i][2]));
        if (inet_pton(AF_INET, _global_ServerIPAndPort[i][1], &servaddr.sin_addr) != 1) {
            printf("Invalid IP Address\n");
            exit(-1);
        }

        /* Establish a connection with the server */
        if (connect(serverStatus->sockID[i], (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
            printf("Could not connect to the Server IP:%s Port:%s\n", _global_ServerIPAndPort[i][1], _global_ServerIPAndPort[i][2]);
             	
	    time_t start, end;
	    double elapsed;  // seconds
	    start = time(NULL);
	    int terminate = 1;
	    while (terminate) {
	     end = time(NULL);
	     elapsed = difftime(end, start);
	     if (elapsed >= 1.0 /* seconds */)
	       terminate = 0;
	         if(connect(serverStatus->sockID[i], (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
		     serverStatus->status[i] = 0; 
		 }
		 else {
	 		serverStatus->status[i] = 1; 
    		 }
		}
        }else serverStatus->status[i] = 1;
    } 
   }
}

/**********************************************************************
*** The main program =: 
*** PLease give the IP address and the POrt number as argumenst to the program
*** example : ./client 127.0.0.1 50001
**********************************************************************/
int main(int argc, char* argv[])
{
    DEBUG(printf("hey\n"));
    int sockfd;
    int clientfd;
    char buf[MAXLIMIT];
    struct sockaddr_in servaddr;
    char commandInput[MAXLIMIT];
    char filePathInput[MAXLIMIT];
    char serverFolderPath[MAXLIMIT];
    
    if (argc != 2) {
        printf("Please specify the Port\nExample: ./client AdamDFC.conf\n");
        exit(0);
    }

    if (!readWebConfig(argv[1])) // using NOt since it is faster that comparator operation
    {
        printf("Error in Web Config, Please restart your webserver again\n");
        exit(0);
    }
	serverStatus = ( serverALive *) malloc(sizeof(serverALive));
    packetInfo * tcpPacketInfo = ( packetInfo *) malloc(sizeof(packetInfo));
    memset(serverStatus, '\0', sizeof(serverALive));
    memset(tcpPacketInfo, '\0', sizeof(packetInfo));
	serverStatus->sockID = (int *)malloc(_global_Counter * sizeof(int));
	serverStatus->status = (int *)malloc(_global_Counter * sizeof(int));
    connectClient();
    printf("Usage of Command:\n   GET [path/fileName]\n   PUT [path/fileName]\n   LIST\n%sExample: GET MyFileName.jpg\n", DELIMITER_1);
    
    while (1) {

        printf("%s@Waiting for User Input\n\n", DELIMITER_1);
        memset(buf, 0, MAXLIMIT);
        memset(commandInput, 0, MAXLIMIT);
        memset(filePathInput, 0, MAXLIMIT);
        memset(serverFolderPath, 0, MAXLIMIT);

        if (fgets(buf, MAXLIMIT - 1, stdin) == NULL) {
            printf("Invalid Entry\n");
            exit(-1);
        }
        if (sscanf(buf, "%s %s %s", commandInput, filePathInput, serverFolderPath) < 1) // this itself takes care of trailing and leading zeros
        {
            printf("Invalid Entry\n");
            continue;
        }
        else {
            DEBUG(printf("%s %s", commandInput, filePathInput));
	printf("%s %s %s", commandInput, filePathInput ,serverFolderPath);
            //since switch statement doesnt work, strcmp is better- but I can aslo use my own hash to
            //the inputs and #define them. Once that is done it is easy for int
            if (strcmp(commandInput, PUT_COMMAND) == 0){
                printf("Here21\n");
                if (fileExist(filePathInput)) {
		 FILE* fileOpen = NULL;
	         fileOpen = fopen(filePathInput, "r"); //501error
       	         if (fileOpen == NULL) {
	  	    printf("Error in FIle\n");
		 }
	         fseek(fileOpen, 0, SEEK_END);
	         int fileSize = ftell(fileOpen);
	         rewind(fileOpen);
	         fclose(fileOpen);

                    printf("Here22\n");
                    if(putFileToServer(filePathInput, serverFolderPath ,fileSize)==0) {
                            printf("The Md5sum of the file %s is:",filePathInput);
                            sprintf(buf,"md5sum %s",filePathInput);
                            system(buf);
                        }
                }
                printf("put command selected\n");
            }
            else if (strcmp(commandInput, GET_COMMAND) == 0) {
                printf("get command selected\n");
                if(getFileFromServer(filePathInput,serverFolderPath)==0)
                {
                }
            }
            else if (strcmp(commandInput, LIST_COMMAND) == 0) {
                printf("ls command selected\n");
                listFileAtServer(filePathInput);
            }
            else {
                printf("other command selected\n");
                //otherCommandToServer(buf, &servaddr);
            }
        }
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

