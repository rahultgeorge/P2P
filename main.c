/* Central server code */

#include "core.h"

int serverSocketFD,active,sd,max_sd;
int peerFD,client_socket[MAX_CLIENTS];
unsigned int clientLength;
struct sockaddr_in serverAddress,clientAddress;
char buffer[14];   
fd_set readfds, writefds;


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




void registerRequestHandler(char* request)
{
	int offset=MESSAGE_HEADER_LENGTH,fileNameSize=0;
	char** fileNames;
	uint16_t noOfFiles;
	uint32_t fileNamesSize,*fileSizes;
	
	memcpy(&noOfFiles,request+offset,sizeof(uint16_t));
	offset+=sizeof(uint16_t);
	printf("No of files: %d \n",noOfFiles);
	assert(noOfFiles!=0);
	
	fileSizes=malloc(sizeof(uint32_t)*noOfFiles);
	
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
	  	  
		uint32_t temp;  
	  memcpy(&temp,request+offset,sizeof(uint32_t));
	  offset+=sizeof(uint32_t);	  
	  printf("File size: %d \n",temp);
	  
	}
}


void messageHandler(char* message)
{
	char* messageType=malloc(sizeof(char)*MESSAGE_HEADER_LENGTH);
	memcpy(messageType,message,MESSAGE_HEADER_LENGTH);
	if(strcmp(messageType,REGISTER_REQUEST)==0)
	{
		printf("%s \n",messageType);	
		registerRequestHandler(message);
	}
	else if(strcmp(messageType,FILE_LIST_REQUEST)==0)
		return;
	
	
	
}

int main(int argc, char **argv)
{
	
	char* buf=malloc(sizeof(char)*MAX_MESSAGE_SIZE);
    int len = 0;
	initalize();
	while(1)
	{
	  FD_ZERO(&readfds);  
	  FD_ZERO(&writefds);  
	  FD_SET(serverSocketFD, &readfds); 
	  FD_SET(serverSocketFD, &writefds); 
	    
	  max_sd = serverSocketFD;   
      //add child sockets to set  
	  for (int i = 0 ; i < MAX_CLIENTS ; i++)   
	  {   
	   //socket descriptor  
	   sd = client_socket[i];     
	   //if valid socket descriptor then add to read list  
	   if(sd > 0)   
	    { 
			FD_SET( sd , &readfds);  
			
		}	 
       //highest file descriptor number, need it for the select function  
	   if(sd > max_sd)   
	     max_sd = sd;   
	  }   
	   
	  active=select(max_sd+1,&readfds,&writefds,NULL,NULL);   
	  if(FD_ISSET(serverSocketFD,&readfds))
	  {
		peerFD=accept(serverSocketFD,( struct sockaddr *) & clientAddress, &clientLength);
		if(peerFD>=0)
		{
			while(len==0)
				ioctl(peerFD, FIONREAD, &len);
			
		    len = read(peerFD, buf, len);
			if(len>0)
		   { printf("Received something %s\n",buf);	
		    messageHandler(buf);
		   }
		  
	     
          for (int i = 0; i < MAX_CLIENTS; i++)   
		   {   
			//if position is empty  
			if( client_socket[i] == 0 )   
			 {   
			   client_socket[i] = peerFD;   
			   printf("Adding to list of sockets as %d\n" , i);   
			   break;   
			 }   
		   } 
		 }
	  }	
    }
    return 0; 






}
