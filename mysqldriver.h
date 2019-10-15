#ifndef _MY_SQL_D_
#define _MY_SQL_D_


#define MAX_MESSAGE_SIZE 2048
#define MAX_P2P_MESSAGE_SIZE 1024*1024

#define MESSAGE_HEADER_LENGTH 12
#define REGISTER_REQUEST "REG_REQUEST\0"
#define FILE_LIST_REQUEST "FLI_REQUEST\0"
#define FILE_LOCATION_REQUEST "FLO_REQUEST\0"
#define CHUNK_REGISTER_REQUEST "CHU_REQUEST\0"

#define REGISTER_REPLY       "REGIS_REPLY\0"
#define FILE_LIST_REPLY      "FI_LS_REPLY\0"
#define FILE_LOCATION_REPLY  "FI_LO_REPLY\0"
#define CHUNK_REGISTER_REPLY "CHUNK_REPLY\0"

struct FileList
{
  char** files;
  int noFiles;
  int *fileSizes;  	
};

struct myChunk
{
  char ipAddress[50];
  int port;
  int numberOfChunks;
  struct myChunksList* next;  	
};

struct myChunkList
{
	int numberOfEndpoints;
	struct myChunk Chunk;
}myChunks;	


int callback(void *NotUsed, int argc, char **argv, 
                    char **azColName);

/*0 success, -1 failure*/
int insertChunk(int chunkID,char* fileName,char* chunkName,int size,char* ipAddress,int port);


int insertIntoFileList(char* fileName,int size,char* ipAddress,int port);

int retrieveFileList(char **reply);

int retrieveFileChunks(char* fileName,char **reply);

int initDB();

#endif
