#ifndef _MY_SQL_D_
#define _MY_SQL_D_

struct FileList
{
  char** files;
  int noFiles;
  int *fileSizes;  	
}*myFileList;

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

int retrieveFileList();

int retrieveFileChunks(char* fileName);

int initDB();

#endif
