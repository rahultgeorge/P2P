/* Central server code */

#include "core.h"

int serverSocketFD;
unsigned int clientLength;
struct sockaddr_in serverAddress,clientAddress;
static pthread_t *peerThreads;
//const char* registerReply;

/* Creates a server socket, binds and starts listening on that port*/
void initalize()
{
    int flag;
    serverSocketFD=socket(DOMAIN,SOCKET_TYPE,PROTOCOL);
    assert(serverSocketFD!=0);
	if(DEBUG)
     printf("Server Socket Created\n"); 
		
    memset(&serverAddress,'\0',sizeof(serverAddress));
    serverAddress.sin_family=DOMAIN;
	inet_aton("127.0.0.1", &serverAddress.sin_addr.s_addr);
    serverAddress.sin_port=htons(PORT);
	
    clientLength=sizeof(clientAddress); 
	
	flag=bind(serverSocketFD,(const struct sockaddr *)&serverAddress,sizeof(serverAddress));
	assert(flag==0);
	if(DEBUG)
     printf("Server Socket Bound\n");  
    flag=listen(serverSocketFD,5);
	assert(flag==0);
	if(DEBUG)
     printf("Server Socket Listening\n"); 
}

void registerRequestHandler(char* request,char* reply)
{
	int offset=MESSAGE_HEADER_LENGTH,fileNameSize=0;
	char** fileNames;
	uint16_t noOfFiles;
	uint32_t fileNamesSize,*fileSizes;
	
	memcpy(&noOfFiles,request+offset,sizeof(uint16_t));
	offset+=sizeof(uint16_t);
	printf("No of files: %d \n",noOfFiles);
	assert(noOfFiles!=0);
	
	fileSizes=(uint32_t*)malloc(sizeof(uint32_t)*noOfFiles);
	
	memcpy(&fileNamesSize,request+offset,sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	printf("File names size %d \n",fileNamesSize);
	
	fileNames=malloc(sizeof(char *) * noOfFiles);
	for(int i=1;i<=noOfFiles;i++)
	{
		
      memcpy(&fileNameSize,request+offset,sizeof(int));
  	  offset+=sizeof(int);
	  printf("File name size %d \n",fileNameSize);
	  
	  fileNames[i-1]=malloc((sizeof(char)*fileNameSize)+1);
  	  memcpy(fileNames[i-1],request+offset,fileNameSize+1);
  	  offset+=(fileNameSize+1);
	  printf("File name: %s \n",fileNames[i-1]);
	  	  
	  memcpy(&fileSizes[i],request+offset,sizeof(uint32_t));
	  offset+=sizeof(uint32_t);	  
	  printf("File size: %d \n",fileSizes[i]);
	  
	}
	
	//Reply here?
	/*Insert into DB*/
	
	
	/*reply*/
	 offset=0;
	 memcpy(reply+offset,REGISTER_REPLY,MESSAGE_HEADER_LENGTH);
	 offset+=MESSAGE_HEADER_LENGTH;
	 sprintf(reply+offset, "_%d", noOfFiles);
	
}


void fileListRequestHandler(char* reply)
{
	
}

void fileLocationRequestHandler(char* request,char* reply)
{
	printf("Received a file location request \n");
	int totalMessageSize=0,offset=0;
	int fileNameSize=0;
    
	
	offset+=MESSAGE_HEADER_LENGTH;
	
	memcpy(&fileNameSize,request+offset,sizeof(int));
	offset+=sizeof(int);
	char fileName[fileNameSize];
	
	memcpy(fileName,request+offset,fileNameSize);
	offset+=strlen(fileName);
	
	printf("Received a file location request for file %s \n",fileName);
		
}

void chunkRegisterHandler(char* message, char* reply)
{
	
}

void messageHandler(char* message,char* reply)
{
	char* messageType=malloc(sizeof(char)*MESSAGE_HEADER_LENGTH);
	memcpy(messageType,message,MESSAGE_HEADER_LENGTH);
	printf("Request received %s \n",messageType);	
	
	if(strcmp(messageType,REGISTER_REQUEST)==0)
		registerRequestHandler(message,reply);
	else if(strcmp(messageType,FILE_LIST_REQUEST)==0)
		fileListRequestHandler(reply);	
	else if(strcmp(messageType,FILE_LOCATION_REQUEST)==0)
		fileLocationRequestHandler(message,reply);
	else if(strcmp(messageType,CHUNK_REGISTER_REQUEST)==0)
		chunkRegisterHandler(message,reply);
	else
		printf("Invalid message\n");
}


void* peerConnectionHandler(void* arg)
{
	int newSocket=*((int *)arg);
	char message[MAX_MESSAGE_SIZE];
	char reply[MAX_MESSAGE_SIZE];
		
	printf("New peer connected: %d\n",newSocket);
	while(1)
	{	
	 memset(message, '\0',MAX_MESSAGE_SIZE);
	 //Blocking call
	 recv(newSocket , message , MAX_MESSAGE_SIZE , 0);
	 messageHandler(message,reply);
	 printf("Reply %s\n",reply);
	 send(newSocket,reply,MAX_MESSAGE_SIZE,0);
    }
	return NULL;
}



int main(int argc, char **argv)
{
	char* buf=malloc(sizeof(char)*MAX_MESSAGE_SIZE);
    int len = 0,newSocket=0,flag=0,i=0;
	peerThreads=malloc(sizeof(pthread_t)*MAX_CLIENTS);
	initalize();
	while(1)
	{
	 newSocket=accept(serverSocketFD,( struct sockaddr *) & clientAddress, &clientLength); 
	 assert(newSocket!=-1);
	 flag=pthread_create(&peerThreads[i],NULL,&peerConnectionHandler,&newSocket);
	 assert(flag==0);
	 ++i;
    }
    return 0; 






}
