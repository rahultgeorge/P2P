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



int main(int argc, char **argv)
{
	initalize();
	char* buf=malloc(sizeof(char)*2048);
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
		    int len = 0;
			ioctl(peerFD, FIONREAD, &len);
			if (len > 0) 
			 { len = read(peerFD, buf, len);
				 buf[len]='\0';
		  printf("Received something %s\n",buf);	
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
