#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h> 
#include <sys/ioctl.h>


#define PORT 60001
#define DEBUG 1
#define DOMAIN AF_INET
#define SOCKET_TYPE SOCK_STREAM
#define PROTOCOL 0
#define MAX_CLIENTS 1000 


