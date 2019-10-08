#include "peer.h"


/*Peer code*/

static int peerToServerFD;
static struct sockaddr_in clientAddress;
static char* serverFileList;
static char* myFiles;
static struct stat **myFilesStats;


bool chunkFiles(int noOfFiles,char **files)
{
	int inputFile,chunkFile,chunkID;
	int bytesRead=-1,bytesWritten=-1;
	size_t chunkSize;
	char *chunk;
	char *chunkName=(char *)malloc(sizeof(char)*50);
	myFilesStats=malloc(noOfFiles*sizeof(struct stat *));
	assert(myFilesStats!=NULL);
	char str[5];
	for(int i=1;i<noOfFiles;++i)
	{
		
		inputFile=open(files[i],O_RDONLY);
		assert(inputFile!=-1);
		assert(myFilesStats[i]!=NULL);
		myFilesStats[i]=malloc(sizeof(struct stat));
		assert(fstat(inputFile,myFilesStats[i])==0);
		chunkSize=myFilesStats[i]->st_size>(1048576)? (myFilesStats[i]->st_size)/3 : (myFilesStats[i]->st_size)/2;
		chunk=(char *)malloc(sizeof(char)*chunkSize);
		if(DEBUG)
		 printf("Dividing %s into chunks\n",files[i]);
		chunkID=1;
		bytesRead=-1;
		while(bytesRead!=0)
		{	
		 bzero((void *)chunk,chunkSize);
		 bzero((void *)chunkName,50);	
		 bzero((void *)str,5);	
		 bytesRead=read(inputFile,chunk,chunkSize);
		 if(bytesRead==0)
			 break;
		 assert(bytesRead>0);
		 if(bytesRead<chunkSize)
			 chunkSize=bytesRead;
		 assert(bytesRead==chunkSize);
		 memcpy(chunkName,&("Chunk_"),6);
		 memcpy(chunkName+6,files[i],strlen(files[i]));
		 sprintf(str, "_%d", chunkID);
		 memcpy(chunkName+6+sizeof(files[i]),&str,sizeof(str));
		 printf("%s\n",chunkName);
		 chunkFile=open(chunkName,O_CREAT| O_WRONLY,0644);
		 assert(chunkFile!=-1);
         bytesWritten=write(chunkFile,chunk,(size_t)chunkSize);
		 assert(bytesWritten==bytesRead);	
		 ++chunkID;
	    }
		free(chunk);
		close(inputFile);
		close(chunkFile);	
	}
   free(chunkName);	
   return true;	
}



unsigned char* fileChunkRequestHandler(char * fileName, uint32_t chunkName)
{
	unsigned char* chunk=NULL;
	return chunk;
}


bool requestForFileList()
{
	printf("Requesting for the file list\n");
	int bytes=write(peerToServerFD,FILE_LIST_REQUEST,MESSAGE_HEADER_LENGTH);
	printf("Bytes written %d\n",bytes);
}


void fileChunkReply()
{
	
}

void cleanUp()
{
	
}

inline void showOptions()
{
 printf("1. Show me the file List\n 2.Download a file 3. View download status 4.Misc\n");	
	
}

/* Number of files to register (uint16_t); and for every file, a file name (string) and its length (uint32_t) */
bool registerRequest(uint16_t noFiles,char **files)
{
	int offset=0,fileNameSize=0,totalMessageSize=0;
	uint32_t fileNamesSize=0,fileSize=0;
	
	for(int i=1;i<=noFiles;i++)
	  fileNamesSize+=(uint32_t)strlen(files[i]);
	printf("File Names Size %d\n",fileNamesSize);
	
	totalMessageSize=(sizeof(uint16_t)+sizeof(uint32_t))+(sizeof(char)*fileNamesSize)+((sizeof(uint32_t)+sizeof(int))*noFiles)+MESSAGE_HEADER_LENGTH;
	printf("Message size: %d\n",totalMessageSize);
	char *request=(char *)malloc(sizeof(char)*totalMessageSize);
	bzero(request,totalMessageSize);
	printf("Request size %s %d\n",request,strlen(request));
	
	
	memcpy(request,REGISTER_REQUEST,MESSAGE_HEADER_LENGTH);
	offset+=MESSAGE_HEADER_LENGTH;
	
	memcpy(request+offset,&noFiles,sizeof(uint16_t));
	offset+=sizeof(uint16_t);
	printf("No of Files %d\n",noFiles);
	
	memcpy(request+offset,&(fileNamesSize),sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	printf("File Names Size %d\n",fileNamesSize);
	
	
    for(int i=1;i<=noFiles;i++)
 	{

	  /*File name size */
    fileNameSize=strlen(files[i]);
    memcpy(request+offset,&fileNameSize,sizeof(int));
  	offset+=sizeof(int);
	printf("File name Size %d \n",fileNameSize);
	
	  /*File name*/
	memcpy(request+offset,files[i],strlen(files[i])+1);
	offset+=(strlen(files[i])+1);
  	printf("File name %s\n",files[i]);

    /*File size*/
	fileSize=(uint32_t)(myFilesStats[i]->st_size);
	memcpy(request+offset,&fileSize,sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	printf("File size %d\n",fileSize);

	}

	request[offset+1]='\0';
	printf("Request %s %d\n",request,strlen(request));
    printf("Sending register request\n");
	int bytesSent=write(peerToServerFD,request,totalMessageSize);
	assert(bytesSent>0);
	printf("Bytes sent: %d\n",bytesSent);
	return true;
}






void viewFileList()
{
	//send(peerToServerFD,FILE_LIST_REQUEST,sizeof(FILE_LIST_REQUEST),0);
	//read();
	
}

int main(int argc, char **argv)
{
	
	    int option=-1;
	    peerToServerFD=socket(DOMAIN,SOCKET_TYPE,PROTOCOL);
	    assert(peerToServerFD!=0);
	    memset(&clientAddress, '\0', sizeof(clientAddress));
	    clientAddress.sin_family=DOMAIN;
	    clientAddress.sin_port=htons(PORT);
	    //memcpy((char *) &clientAddress.sin_addr.s_addr,SERVER,SERVER_LEN);
	    inet_aton("127.0.0.1", &clientAddress.sin_addr.s_addr);
	    chunkFiles(argc,argv);
        
       
	    if(connect(peerToServerFD,(struct sockaddr *)&clientAddress,sizeof(clientAddress))==0)
 	    {
 		  if(DEBUG)
 		   printf("Client Connected\n");
 		  /*Options to view file list, download a file and view download status*/
   		  if(registerRequest(argc-1,argv)==true)
		  		  {
					  printf("Successfully registered\n");
					  requestForFileList();
					  	   while(1)
					  		    {
								//	showOptions();
									 			  //scanf("%d",&option);												   			  
												  switch(option)
												  {
  			    case 1:
  			          //viewFileList();
 			          break;
  			    case 2:

  	                  break;
 			    case 3:
 	                  break;
 			    case 4:
  	                  break;
  			    default:
  			          //cleanUp();
 					  break;
   		    }
 	      }
   	       /* Write a response to the server */
	  }

 	    }

        return 0; 
}
