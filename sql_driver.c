//SQLLite Driver

#include <stdio.h>
#include <sqlite3.h> 
#include <mysqldriver.h>

/*0 success, -1 failure*/
int insertChunk(int chunkID,char* fileName,int size,char* ipAddress,int port)
{
	sqlite3 *db; 
	sqlite3_stmt *stmt;

	char* query;
	    char *zErrMsg = 0;


	    int rc = sqlite3_open("P2P.db", &db);
	    asprintf(&query, "insert into CHUNKS (chunk_id, FILE_NAME,SIZE,IP_ADDRESS,PORT) values (%d, '%s',%d,'%s',%d);", chunkID,fileName,size,ipAddress,port);

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


int insertIntoFileList(int chunkID,char* fileName,int size,char* ipAddress,int port)
{
	sqlite3 *db; 
	sqlite3_stmt *stmt;

	char* query;
	    char *zErrMsg = 0;


	    int rc = sqlite3_open("P2P.db", &db);
	    asprintf(&query, "insert into CHUNKS (chunk_id, FILE_NAME,SIZE,IP_ADDRESS,PORT) values (%d, '%s',%d,'%s',%d);", chunkID,fileName,size,ipAddress,port);

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





int init() 
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
       "CHUNK_ID INT      NOT NULL," \
       "FILE_NAME           CHAR(20)    NOT NULL," \
       "SIZE            INT     NOT NULL," \
       "IP_ADDRESS        CHAR(50)," \
       "PORT         INT NOT NULL," \
	"PRIMARY KEY (CHUNK_ID, IP_ADDRESS)	 );";
   
   rc = sqlite3_exec(db, query, NULL, 0, &zErrMsg);
   if( rc ) {
      fprintf(stderr, "Can't create table database: %s\n", sqlite3_errmsg(db));
      return -1;
   } 
                                                                              
   
   sqlite3_close(db);
   return 0;
}