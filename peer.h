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

const char *FILE_LIST_REQUEST="FILE_LIST_REQUEST";
const char *FILE_LOCATION_REQUEST="FILE_LOCATION_REQUEST";


/* The functions accepts a list of files
 * and chunks the said files.
 */
bool chunk_files(char*);

unsigned char* fileChunkRequestHandler(char * fileName, uint32_t chunkID);

void fileChunkReply();