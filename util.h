/**********************************************************************
*** This is a util.h file used at both client and the server 
*** it has common data shared between client and server
*** Autor: Shekhar 
*** REferences: Websites like Stackoverflow etc..
**********************************************************************/
#ifndef UTIL_H
#define UTIL_H

#define MAXLIMIT  1024
#define PUT_COMMAND "PUT"
#define GET_COMMAND "GET"
#define LIST_COMMAND "LIST"
#define OTHER_COMMAND "other"
#define DELIMITER_3 "*********************************************************\n"
#define DELIMITER_1 "--------------------------------\n"
#define DELIMITER_2 "\n\t"
#define WEBCONFIG "/home/shekhar/Netsys/p3/DFS.conf"
#define WEBCONFIG_CLIENT "/home/shekhar/Netsys/p3/"
#define PASS_KEY 'A'
/*** #defines ***/
//#define DEBUG_ON 1 //if debug is required #define DEBUG_ON shoudl be defined
#ifdef DEBUG_ON
#define DEBUG(X) X; fflush(stdout)
#else
#define DEBUG(X)
#endif

#define NUMBER_OF_SERVERS 4
#define LISTEN "Listen"
#define DOC_ROOT "DocumentRoot"
#define DIR_INDEX "DirectoryIndex"
#define CONN_KEEP_LIVE "Keep-Alive"
#define CONN_TYPE "Content-Type"

#define MINLIMIT 200
#define DELIMITER "\r\n"
#define FILE_DELIMITER "<"

#define USERNAME "Username:"
#define PASSWORD "Password:"
#define SERVER   "Server"

//the declaration of the common fucntions in util.c
int strToInteger(char *str);
int fileExist(const char * filename);
char* listFileInCurrentDirectory();
void encryptDecryptData(char * data, unsigned int len, char * password, unsigned int passlen);
int calculateMD5SumMod4(char * fileName);
unsigned int _log2( unsigned int x );
char * fileNameFromDirectoryString(char * fullPath);
int getFileChunkName(char * fileName, int index);
void show_dir_content(char* path, char* fileName, int* fileExists, char * fileNameArray[]);

//the strct tcpPacketInfo holds the value of the Packet INformations
typedef struct tcpPacketInfo{
unsigned int packetInfoNotpacket;
char command[MAXLIMIT]; 
char fileName[MAXLIMIT];
unsigned int fileSize;
}tcpPacketInfo;

//the strct udpPacket  is the packet by itself, it contains the data and the frame numver
typedef struct tcpPacket{
unsigned int packetInfoNotpacket;
char bufferData[MAXLIMIT];
unsigned int frameNumber;
unsigned int readBytes; 
}udpPacket;

//the struct udpPacketAck hols thevalues of the ack; it has the previous and current ack information
typedef struct tcpPacketAck{
unsigned int packetInfoNotpacket;
unsigned int frameNumber;
unsigned int previousFrameNumber;
}tcpPacketAck;


typedef struct serverALive{
    int * sockID;
    int * status;
}serverALive;

typedef struct packetInfo{
   int dataFlag;
   int fileSize;
   int readBytes; 
   int errorFlag;  
   char username[MINLIMIT];
   char pasword[MINLIMIT];
   char fileName[MINLIMIT];
   char folderName[MINLIMIT];
   char command[MINLIMIT];
   char data[MAXLIMIT];
   char errorMessage[MAXLIMIT];
}packetInfo;


#endif
