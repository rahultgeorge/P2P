#ifndef PEER_H
#define PEER_H 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h> 
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h> 


#define DEBUG 0
#define DOMAIN AF_INET
#define SOCKET_TYPE SOCK_STREAM
#define PROTOCOL 0
#define PORT 60001
#define SERVER "127.0.0.1"
#define SERVER_LEN 9
#define CHUNK_SIZE 524288

#define P2P_PORT 60002 //Peer listens dynamically on a port
#define MAX_CLIENTS 100


#define MAX_MESSAGE_SIZE 2048
#define MAX_P2P_MESSAGE_SIZE 1024*1024

#define MESSAGE_HEADER_LENGTH 12
#define REGISTER_REQUEST "REG_REQUEST\0"
#define FILE_LIST_REQUEST "FLI_REQUEST\0"
#define FILE_LOCATION_REQUEST "FLO_REQUEST\0"
#define CHUNK_REGISTER_REQUEST "CHU_REQUEST\0"
#define FILE_CHUNK_REQUEST "FCH_REQUEST\0"


#define REGISTER_REPLY       "REGIS_REPLY\0"
#define FILE_LIST_REPLY      "FI_LS_REPLY\0"
#define FILE_LOCATION_REPLY  "FI_LO_REPLY\0"
#define CHUNK_REGISTER_REPLY "CHUNK_REPLY\0"


struct FileChunkRequest
{
	char* fileName;
	int chunkID;
	int p2pFD;
};


struct FileList
{
  char** files;
  int noFiles;
  int *fileSizes;  	
};

/* The functions accepts a list of files
 * and chunks the said files.
 */
bool chunk_files(char*);

void* fileChunkRequestHandler(void *arg);
	
	
void fileChunkReply();

void showDownloadStatus();

#endif