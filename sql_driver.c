//SQLLite Driver

#include <stdio.h>
#include <stdlib.h> 
#include <sqlite3.h> 
#include <string.h>
#include "mysqldriver.h"



int callback(void *NotUsed, int argc, char **argv, 
                    char **azColName) {
    
    NotUsed = 0;
	int queryType=-1;
	if(strncmp(azColName[1],"FILE_NAME",9)==0)
	{	
		queryType=0;
   		//      myFileList=malloc(sizeof(FileList));
		// myFileList->noFiles=argv[0];
		// myFileList->files=malloc(sizeof(char)*20*(myFileList->noFiles));
		// myFileList->fileSizes=malloc(sizeof(int)*(myFileList->noFiles));
	}
    for (int i = 0; i < argc; i++) {

        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		if(queryType==0)
		{
			//myFileList->files
		}
    }
    
    printf("\n");
    
    return 0;
}


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
		char str[5];
		
		if(size>1048576)
			numberOfChunks=3;
		else
			numberOfChunks=2;
		
		printf("Updating the server file list\n");
		/*Insert into the chunks table*/
		for(int i=1;i<=numberOfChunks;i++)
		{	
		    chunkID=i;
	        memcpy(chunkName,&("Chunk_"),6);
	        memcpy(chunkName+6,fileName,strlen(fileName));
	        sprintf(str, "_%d", chunkID);
   		    memcpy(chunkName+6+strlen(fileName),&str,sizeof(str));	
	        insertChunk(chunkID,fileName,chunkName,size,ipAddress,port);
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



int retrieveFileList()
{
    sqlite3 *db; 
    char *query;
    char *err_msg = 0;
    int rc = sqlite3_open("P2P.db", &db);
	
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", 
        sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    
	
	printf("Retrieving file list\n");
	/*TODO Update the query*/
    query="SELECT DISTINCT FILE_NAME,SIZE FROM FILES;";
    //asprintf(&query, "SELECT FILE_NAME,SIZE FROM FILES WHERE IP_ADDRESS !='%s';",ipAddress);
	

    rc = sqlite3_exec(db, query, callback, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        
        return -1;
    } 
	
    query="SELECT COUNT(*) FROM FILES;";
    //asprintf(&query, "SELECT FILE_NAME,SIZE FROM FILES WHERE IP_ADDRESS !='%s';",ipAddress);
	

    rc = sqlite3_exec(db, query, callback, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        
        return -1;
    } 
	
	
    return 0;
}


int retrieveFileChunks(char* fileName)
{
    sqlite3 *db; 
    char *query;
    char *err_msg = 0;
    int rc = sqlite3_open("P2P.db", &db);
	
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", 
                sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return -1;
    }
    
	
	printf("Executing SQL Select query\n");
    asprintf(&query, "SELECT * FROM CHUNKS WHERE FILE_NAME='%s';",fileName);
    	

    rc = sqlite3_exec(db, query, callback, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        
        return -1;
    } 
    return 0;
}





int initDB() 
{
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *query;
   

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
   
   rc = sqlite3_exec(db, query, NULL, 0, &zErrMsg);
   if( rc ) {
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
   
   rc = sqlite3_exec(db, query, NULL, 0, &zErrMsg);
   if( rc ) {
      fprintf(stderr, "Can't create table database: %s\n", sqlite3_errmsg(db));
      return -1;
   } 	                                                                      
   
   sqlite3_close(db);
   return 0;
}

// int main()
// {
// 	retrieveFileList();
// 	return 1;
//
// }

