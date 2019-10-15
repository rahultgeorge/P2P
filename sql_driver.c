//SQLLite Driver

#include <stdio.h>
#include <stdlib.h> 
#include <sqlite3.h> 
#include <string.h>
#include "mysqldriver.h"


/*0 success, -1 failure*/
int insertChunk(int chunkID,char* fileName,char* chunkName,int size,char* ipAddress,int port)
{
	sqlite3 *db; 
	sqlite3_stmt *stmt;
	char* query;
	char *zErrMsg = 0;


	    int rc = sqlite3_open("P2P.db", &db);
	    asprintf(&query, "insert into CHUNKS (chunk_id, FILE_NAME,CHUNK_NAME,SIZE,IP_ADDRESS,PORT) values (%d, '%s','%s',%d,'%s',%d);", chunkID,fileName,chunkName,size,ipAddress,port);

	    sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);

	    rc = sqlite3_step(stmt);
	    if (rc != SQLITE_DONE) {
	        printf("ERROR inserting data: %s\n", sqlite3_errmsg(db));
		return -1;
	    }

	    sqlite3_finalize(stmt);
	    free(query);
	    return 0;
}


int insertIntoFileList(char* fileName,int size,char* ipAddress,int port)
{
	    sqlite3 *db; 
	    sqlite3_stmt *stmt;
	    char* query;
	    char *zErrMsg = 0;
		int numberOfChunks=0;
		int chunkID=-1;
	    int rc = sqlite3_open("P2P.db", &db);
		char chunkName[50];
		int chunkSize;
		char str[5];
		int i,bytesLeft=-1;
		
		
		if(size>1048576)
		{	numberOfChunks=3+(size%3!=0 ? 1 : 0);
		    chunkSize=size/3;
		}
		else
		{	numberOfChunks=2+(size%2);
			chunkSize=size/2;
			
		}
		printf("Updating the server file list\n");
		bytesLeft=size;
		/*Insert into the chunks table*/
		for(i=1;i<=numberOfChunks;i++)
		{	
		    chunkID=i;
			bzero(chunkName,50);
	        memcpy(chunkName,&("Chunk_"),6);
	        memcpy(chunkName+6,fileName,strlen(fileName));
	        sprintf(str, "_%d", chunkID);
   		    memcpy(chunkName+6+strlen(fileName),&str,sizeof(str));	
			if(bytesLeft>=chunkSize)
	         insertChunk(chunkID,fileName,chunkName,chunkSize,ipAddress,port);
			else
   	         insertChunk(chunkID,fileName,chunkName,bytesLeft,ipAddress,port);	
			bytesLeft-=chunkSize;
	    }
		
        asprintf(&query, "insert into FILES (FILE_NAME,SIZE,IP_ADDRESS,PORT) values ('%s',%d,'%s',%d);",fileName,size,ipAddress,port);

        sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
         printf("ERROR inserting data: %s\n", sqlite3_errmsg(db));
	     return -1;
         }

        sqlite3_finalize(stmt);
        free(query);
	    return 0;
}



int retrieveFileList(char** fileList)
{
    sqlite3 *db; 
    sqlite3_stmt *stmt;
    char *query;
    char *err_msg = 0;
    int rc = -1;
	int numberOfFiles=-1;
	int row=0;
	int offset=0;
	int fileNameLength=-1;
	struct FileList *myFileList;
	
		
	rc=sqlite3_open("P2P.db", &db);
	
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", 
        sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    
	
	printf("Retrieving file list\n");
	
	//Get count
    query="SELECT COUNT(*) FROM FILES;";
    //asprintf(&query, "SELECT FILE_NAME,SIZE FROM FILES WHERE IP_ADDRESS !='%s';",ipAddress);
	
    sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
    rc = sqlite3_step(stmt);
	
	if(rc==SQLITE_ROW)
	{
	  	numberOfFiles=sqlite3_column_int(stmt,0);
		/*Now allocate space for the file list data structure */
	    myFileList=malloc(sizeof(struct FileList));
	    myFileList->noFiles=numberOfFiles;
		myFileList->files=malloc(sizeof(char)*20*(myFileList->noFiles));
		myFileList->fileSizes=malloc(sizeof(int)*(myFileList->noFiles));		
	}

    else if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        
        return -1;
    } 
	
    sqlite3_finalize(stmt);
 
	
	/*Now get the list of files and their sizes*/
	/*TODO Update the query*/
    query="SELECT DISTINCT FILE_NAME,SIZE FROM FILES;";
    //asprintf(&query, "SELECT FILE_NAME,SIZE FROM FILES WHERE IP_ADDRESS !='%s';",ipAddress);
    sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	
	rc = sqlite3_step(stmt);
	row=0;
	offset=0;
	
	memcpy((*fileList)+offset,FILE_LIST_REPLY,MESSAGE_HEADER_LENGTH);
	offset+=MESSAGE_HEADER_LENGTH;
	
	memcpy((*fileList)+offset,&numberOfFiles,sizeof(int));
	offset+=sizeof(int);	
	while(rc==SQLITE_ROW)
	{
		myFileList->files[row]=sqlite3_column_text(stmt,0);
		myFileList->fileSizes[row]=sqlite3_column_int(stmt,1);
		// printf("File name %s and size %d \n",myFileList->files[row],myFileList->fileSizes[row]);
		
		fileNameLength=strlen(myFileList->files[row]);
		// printf("%d is the file name size\n",fileNameLength);
		
		memcpy((*fileList)+offset,&(fileNameLength),sizeof(int));
		offset+=sizeof(int);		
		// printf("File name %s and size %d \n",myFileList->files[row],myFileList->fileSizes[row]);
				
		memcpy((*fileList)+offset,myFileList->files[row],fileNameLength+1);
		offset+=fileNameLength+1;
		
		memcpy((*fileList)+offset,&myFileList->fileSizes[row],sizeof(int));
		offset+=sizeof(int);
		
		
		rc = sqlite3_step(stmt);
		++row;
	}
	
    return 0;
}


int retrieveFileChunks(char* fileName,char** chunkList)
{
    sqlite3 *db; 
	sqlite3_stmt *stmt;
    char *query;
    char *err_msg = 0;
	int row=0;
    int rc = sqlite3_open("P2P.db", &db);
	int numberOfChunks=-1,offset=0;
	
	int chunkID,size,port,fileNameLength,chunkNameLength,ipSize;
	char chunkName[50],ipAddress[50];
	
    if (rc != SQLITE_OK) 
	{
        fprintf(stderr, "Cannot open database: %s\n", 
                sqlite3_errmsg(db));
        sqlite3_close(db);    
        return -1;
    }
	
	
	
    asprintf(&query, "SELECT COUNT(*) FROM CHUNKS WHERE FILE_NAME='%s';",fileName);
    	
    sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
    rc = sqlite3_step(stmt);
	if(rc==SQLITE_ROW)
     numberOfChunks=sqlite3_column_int(stmt,0);
	
	
    sqlite3_finalize(stmt);
	
	
	offset=0;
	memcpy((*chunkList)+offset,FILE_LOCATION_REPLY,MESSAGE_HEADER_LENGTH);
	offset+=MESSAGE_HEADER_LENGTH;
	
	memcpy((*chunkList)+offset,&numberOfChunks,sizeof(int));
	offset+=sizeof(int);	
	
    asprintf(&query, "SELECT * FROM CHUNKS WHERE FILE_NAME='%s';",fileName);
    	
    sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
    rc = sqlite3_step(stmt);
	
	/*    "CHUNK_ID INT      NOT NULL," \
       "FILE_NAME CHAR(20)    NOT NULL," \
	   "CHUNK_NAME  CHAR(50)    NOT NULL," \
       "SIZE            INT     NOT NULL," \
       "IP_ADDRESS        CHAR(50)," \
       "PORT  */
	
	while(rc==SQLITE_ROW)
	{
        chunkID=sqlite3_column_int(stmt,0);
		memcpy((*chunkList)+offset,&chunkID,sizeof(int));
		offset+=sizeof(int);
		
		/*File name length and file name*/
		
		fileNameLength=strlen(sqlite3_column_text(stmt,1));
		memcpy((*chunkList)+offset,&fileNameLength,sizeof(int));
		offset+=sizeof(int);
		
		strncpy(fileName,sqlite3_column_text(stmt,1),fileNameLength+1);
		memcpy((*chunkList)+offset,fileName,fileNameLength+1);
		offset+=fileNameLength+1;
		
		/*Chunk name length and file name*/
		
		chunkNameLength=strlen(sqlite3_column_text(stmt,2));
		memcpy((*chunkList)+offset,&chunkNameLength,sizeof(int));
		offset+=sizeof(int);
		
		strncpy(chunkName,sqlite3_column_text(stmt,2),chunkNameLength+1);
		memcpy((*chunkList)+offset,chunkName,chunkNameLength+1);
		offset+=chunkNameLength+1;
		
		/* Size */
        size=sqlite3_column_int(stmt,3);
		memcpy((*chunkList)+offset,&size,sizeof(int));
		offset+=sizeof(int);
		
		/* IP Address */
		
		ipSize=strlen(sqlite3_column_text(stmt,4));
		memcpy((*chunkList)+offset,&ipSize,sizeof(int));
		offset+=sizeof(int);
		
		strncpy(ipAddress,sqlite3_column_text(stmt,4),ipSize);
		memcpy((*chunkList)+offset,ipAddress,ipSize+1);
		offset+=ipSize+1;
		
		/* Port */
		
        port=sqlite3_column_int(stmt,5);
		memcpy((*chunkList)+offset,&port,sizeof(int));
		offset+=sizeof(int);
		
		printf("Chunk id %d, file name %s chunk name %s \n", chunkID, fileName, chunkName);		
		rc = sqlite3_step(stmt);
	}
	
	sqlite3_finalize(stmt);
    return 0;
}





int initDB() 
{
   sqlite3 *db;
   sqlite3_stmt *stmt;
   char *zErrMsg = 0;
   int rc;
   char *query;
   
   printf("Initialzing database\n");
   rc = sqlite3_open("P2P.db", &db);

   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
   } 
   
   query = "CREATE TABLE IF NOT EXISTS FILES("  \
       "FILE_NAME           CHAR(20)    NOT NULL," \
       "SIZE            INT     NOT NULL," \
	       "IP_ADDRESS        CHAR(50) NOT NULL," \
	       "PORT         INT NOT NULL," \
	"PRIMARY KEY (FILE_NAME,IP_ADDRESS)	 );";
   
   
   sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
   rc = sqlite3_step(stmt);
   sqlite3_finalize(stmt);
   
   
   if( rc != SQLITE_DONE) {
      fprintf(stderr, "Can't create table database: %s\n", sqlite3_errmsg(db));
      return -1;
   } 
         
   		 
		 
   query = "CREATE TABLE IF NOT EXISTS CHUNKS("  \
       "CHUNK_ID INT      NOT NULL," \
       "FILE_NAME CHAR(20)    NOT NULL," \
	   "CHUNK_NAME  CHAR(50)    NOT NULL," \
       "SIZE            INT     NOT NULL," \
       "IP_ADDRESS        CHAR(50)," \
       "PORT         INT NOT NULL," \
	   "PRIMARY KEY (CHUNK_ID, FILE_NAME, IP_ADDRESS)	 );";
   
   sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
   rc = sqlite3_step(stmt); 
   sqlite3_finalize(stmt);
     
   
   if( rc != SQLITE_DONE) {
      fprintf(stderr, "Can't create table database: %s\n", sqlite3_errmsg(db));
      return -1;
   } 	                                                                      
   
   sqlite3_close(db);
   return 0;
}

// int main()
// {
// 	retrieveFileChunks("Makefile",NULL);
// 	return 1;
//
// }

