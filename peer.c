#include "peer.h"


/*Peer code*/

static int peerToServerFD;
static int peerListenerSocket;

static struct sockaddr_in peerListeningAddress,peerDownloadAddress;
static struct sockaddr_in serverAddress;

int peerListeningAddressLength;	

static struct stat **myFilesStats;

static pthread_t *peerThread; //Peer listening thread
static pthread_t *p2pThreads;
pthread_mutex_t printfLock;

void display(char* message)
{
	pthread_mutex_lock(&printfLock);
	printf("%s\n",message);
	pthread_mutex_unlock(&printfLock);	
}

void initialize()
{
	int flag,on=1; 
    peerListenerSocket=socket(DOMAIN,SOCKET_TYPE,PROTOCOL);
    assert(peerListenerSocket!=0);
	if(DEBUG)
     display("Peer listener socket Created\n"); 
		
    memset(&peerListeningAddress,'\0',sizeof(peerListeningAddress));
    peerListeningAddress.sin_family=DOMAIN;
	inet_aton("127.0.0.1", &peerListeningAddress.sin_addr.s_addr);
	/*TODO Change port number on which it listens */
    peerListeningAddress.sin_port=htons(P2P_PORT);
	peerListeningAddressLength=sizeof(peerListeningAddress);
    flag=setsockopt(peerListenerSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));	
	
	flag=bind(peerListenerSocket,(const struct sockaddr *)&peerListeningAddress,sizeof(peerListeningAddress));
	assert(flag==0);
	if(DEBUG)
     display("Peer listening Socket Bound\n");  
    flag=listen(peerListenerSocket,5);
	assert(flag==0);
	if(DEBUG)
     display("Peer Socket Listening\n"); 
	display("INSIDE"); 
}


void showDownloadStatus(){
	
	
}


void combineChunks(char* fileName, int numberOfChunks)
{
	int chunkFD=-1,file=-1;
	int bytesRead=1,bytesWritten=1;
	char chunkName[50],str[5];
	char buffer[BUFFER_SIZE];
	
    file=open(fileName,O_CREAT|O_APPEND|O_WRONLY,0744);
	for(int i=0;i<numberOfChunks;i++)
	{
	  memcpy(chunkName,&("Chunk_"),6);
	  memcpy(chunkName+6,fileName,strlen(fileName));
	  sprintf(str, "_%d", (i+1));
	  memcpy(chunkName+6+strlen(fileName),&str,sizeof(str));
      chunkFD=open(chunkName,O_RDONLY,0644);
	  printf("Trying to combine\n");
	  bytesRead=bytesWritten=1;
	  while(bytesRead>0 && bytesWritten>0)
	  { 
		  bytesRead=read(chunkFD,buffer,BUFFER_SIZE); 
		  bytesWritten=write(file,buffer,bytesRead);
		  //assert(bytesRead==bytesWritten); 
	  }
	  close(chunkFD);
	}
    close(file);	  	
    	
}


void* fileChunkRequestHandler(void *arg)
{
	/*Handles each peer to peer connection for peers requesting a chunk */
	int newSocket=*((int *)arg);
	char message[MAX_MESSAGE_SIZE];
	int offset=0;
	char header[MESSAGE_HEADER_LENGTH];
	char str[5];
	char fileName[50],chunkName[30],*buffer;
	int chunkFD=-1,chunkID=-1,fileNameSize=-1,chunkSize=-1;
	static struct stat *myFilesStats;	
	int bytesRead=-1,bytesWritten=-1,totalBytesRead=0;
	buffer=malloc(sizeof(char)*BUFFER_SIZE);
	

	 memset(message, '\0',MAX_MESSAGE_SIZE);
	 //Blocking call
	 recv(newSocket , message , MAX_MESSAGE_SIZE , 0);
	 offset=0;
	 if(strlen(message)!=0)
	 {

	 myFilesStats=malloc(sizeof(struct stat ));	 
	 memcpy(header,message+offset,MESSAGE_HEADER_LENGTH);
	 offset+=MESSAGE_HEADER_LENGTH;
	 
	 if(strncmp(header,FILE_CHUNK_REQUEST,MESSAGE_HEADER_LENGTH)==0)
	 {
		 //Read the file name size
		 
		 memcpy(&fileNameSize,message+offset,sizeof(int));
		 offset+=sizeof(int);
		 
		 //printf("File name size: %d\n",fileNameSize);
         		 
		 //Read the file name
		 memcpy(fileName,message+offset,fileNameSize+1);
		 offset+=fileNameSize+1;
		 
		 //Read the chunk ID
		 memcpy(&chunkID,message+offset,sizeof(int));
		 offset+=sizeof(int);
		 
		 memcpy(chunkName,&("Chunk_"),6);
		 memcpy(chunkName+6,fileName,strlen(fileName));
		 sprintf(str, "_%d", chunkID);
		 memcpy(chunkName+6+strlen(fileName),&str,sizeof(str));
		 
		 printf("Received a file chunk request for %s \n",chunkName);
		 
		 
		 //Upload
		 chunkFD=open(chunkName,O_RDONLY,0644);
		 assert(chunkFD!=-1);
		 
 		 assert(fstat(chunkFD,myFilesStats)==0);
 		 chunkSize=myFilesStats->st_size;
		 assert(chunkSize!=0);
		 bytesRead=-1;
		 totalBytesRead=0;
		 while(totalBytesRead!=chunkSize)
		 {
		 			  bytesRead=read(chunkFD,buffer,BUFFER_SIZE);
		 			  bytesWritten=send(newSocket,buffer,bytesRead,0);
					  //printf("Bytes sent %d - %s\n",bytesRead,chunkName);
		 			  assert(bytesRead==bytesWritten);
		 			  totalBytesRead+=bytesRead;
		 }	
		 printf("Done uploading file\n");  	
         close(chunkFD);
		 close(newSocket);	   
	 }

    }
		
}



void* peerListenerThreadHandler(void* arg)
{
	/*Listening for download requests*/
	int flag,newSocket=-1,i=0;
   
	p2pThreads=malloc(sizeof(pthread_t));
	initialize();
	
	while(1)
	{	
	 newSocket=accept(peerListenerSocket,( struct sockaddr *) & peerListeningAddress, &peerListeningAddressLength ); 
	 assert(newSocket!=-1);
	 //display("New peer requesting for file chunk\n");
	 flag=pthread_create(p2pThreads,NULL,&fileChunkRequestHandler,&newSocket);
	 assert(flag==0);
    }	

	return NULL;
}


bool chunkFiles(int noOfFiles,char **files)
{
	int inputFile,chunkFile,chunkID;
	int bytesRead=-1,bytesWritten=-1;
	size_t chunkSize;
	char *chunk;
	char *chunkName=(char *)malloc(sizeof(char)*50);
	int i=0;
	myFilesStats=malloc(noOfFiles*sizeof(struct stat *));
	assert(myFilesStats!=NULL);
	char str[5];
	for(i=1;i<noOfFiles;++i)
	{
		inputFile=open(files[i],O_RDONLY,0644);
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
		 memcpy(chunkName+6+strlen(files[i]),&str,sizeof(str));
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


void* peerDownloadThreadHandler(void* arg)
{
	/*Download file thread specific*/
	char chunkName[50];
	char request[MAX_MESSAGE_SIZE];
	int p2pFD=0;
	/*We can actually read the chunk id and file name from arg*/
	int chunkID,fileNameSize;
	int downloadedChunk;
	int num=-1,offset=0;
	int size=(1024);
	char *buffer=malloc(sizeof(char)*BUFFER_SIZE);
	char str[5];
	char* fileName;
	struct FileChunkRequest* fileChunkRequest;
	int requestSize=0,done;
	
	fileChunkRequest=(struct FileChunkRequest *)arg;
	
	fileName=fileChunkRequest->fileName;
	chunkID=fileChunkRequest->chunkID;
	p2pFD=fileChunkRequest->p2pFD;
	fileNameSize=strlen(fileName);
	
	offset=0;
    memcpy(request+offset,FILE_CHUNK_REQUEST,MESSAGE_HEADER_LENGTH);
    offset+=MESSAGE_HEADER_LENGTH;
	
	//Write the file name size
    memcpy(request+offset,&fileNameSize,sizeof(int));
    offset+=sizeof(int);
	
    //Write the file name
    memcpy(request+offset,fileName,fileNameSize+1);
    offset+=fileNameSize+1;
	
    //Write the chunk ID
    memcpy(request+offset,&chunkID,sizeof(int));
    offset+=sizeof(int);

	
	//printf("Sending %s for %d \n",request,chunkID);
	requestSize=8+MESSAGE_HEADER_LENGTH+fileNameSize;
	send(p2pFD,request,requestSize,0);	
	
	
	bzero(chunkName,50);
    memcpy(chunkName,&("Chunk_"),6);
    memcpy(chunkName+6,fileName,strlen(fileName));
    sprintf(str, "_%d", chunkID);
    memcpy(chunkName+6+strlen(fileName),&str,sizeof(str));	
	
  
	    printf("Downloading the chunk %s \n",chunkName);	
		downloadedChunk=open(chunkName,O_CREAT | O_APPEND| O_WRONLY, 0644);
		assert(downloadedChunk!=-1);
		done=0;
		while(done!=1)
		{	
		while((num=read(p2pFD,buffer,BUFFER_SIZE))!=0)
		{
			//printf("Looping for chunk %s\n",chunkName);
			//printf("Read %d bytes \n",num);
			num=write(downloadedChunk,buffer,num);
			//printf("Wrote %d bytes into %s\n",num,chunkName);
		}
		done=1;
	   }
		close(downloadedChunk);
	    printf("Done downloading chunk %s \n",chunkName);		
	
		arg=(struct FileChunkRequest*)arg;
	free(arg);
	return NULL;
}
 

void  requestForFileChunk(char * fileName, uint32_t chunkID,char* ipAddress, int port)
{
	
	/*Downloading chunk from other peers using the data received as part of the file location reply*/
	int flag=-1;
	int p2pFD=0;
    void* arg=NULL;
	pthread_t *p2pThread; //P2P threads
	p2pThread=malloc(sizeof(struct pthread_t*));
	struct FileChunkRequest *fileChunkRequest;
	fileChunkRequest=malloc(sizeof(struct FileChunkRequest));
	printf("Requested for a file chunk\n");
	bzero(fileChunkRequest,sizeof(fileChunkRequest));
		
    p2pFD=socket(DOMAIN,SOCKET_TYPE,PROTOCOL);
    assert(p2pFD!=0);
	
    memset(&peerDownloadAddress, '\0', sizeof(peerDownloadAddress));
    peerDownloadAddress.sin_family=DOMAIN;
	
	//Connect to the port the peer is listening to
    peerDownloadAddress.sin_port=htons(60002); 
    inet_aton(ipAddress, &peerDownloadAddress.sin_addr.s_addr);
	
	if(connect(p2pFD,(struct sockaddr *)&peerDownloadAddress,sizeof(peerDownloadAddress))==0)
	{
		printf("Connected to peer\n");
		fileChunkRequest->fileName=fileName;
		fileChunkRequest->chunkID=chunkID;
		fileChunkRequest->p2pFD=p2pFD;
		arg=(void *)fileChunkRequest;
		
	    flag=pthread_create(p2pThread,NULL,&peerDownloadThreadHandler,arg);  

    }

}





static inline void  showOptions()
{
 printf("Peer options\n 1. Show me the file List\n 2.Download a file\n 3. View download status\n 4.Misc\n");	
	
}

/* Number of files to register (uint16_t); and for every file, a file name (string) and its length (uint32_t) */
bool registerRequest(uint16_t noFiles,char **files)
{
	int offset=0,fileNameSize=0,totalMessageSize=0;
	int expectedBitMapReply=0,bitMapReply=-1;
	uint32_t fileNamesSize=0,fileSize=0;
	char reply[MAX_MESSAGE_SIZE];
	int i=0;
	for(i=1;i<=noFiles;i++)
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
	
	
    for(i=1;i<=noFiles;i++)
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
    expectedBitMapReply |= 1UL << (i-1);
	
	}

	request[offset+1]='\0';
	printf("Request %s %d\n",request,strlen(request));
    printf("Sending register request\n");
	int bytesSent=write(peerToServerFD,request,totalMessageSize);
	assert(bytesSent>0);
	printf("Bytes sent: %d\n",bytesSent);
	
	bzero(reply,MAX_MESSAGE_SIZE);
	offset=0;
	/*Wait for reply*/
    recv(peerToServerFD,reply,MAX_MESSAGE_SIZE,0);
	printf("Registered successfully %d\n",expectedBitMapReply);
    offset+=MESSAGE_HEADER_LENGTH;
	//     memcpy(&bitMapReply,reply+offset,sizeof(int));
	// if(bitMapReply!=expectedBitMapReply)
	//  printf("Registered Successfully %d\n",bitMapReply);
	
	return true;
}

void fileListRequest()
{
	char reply[MAX_MESSAGE_SIZE];
	int numberOfFiles=-1,offset=0;
	int fileSize=0,fileNameSize=0;
	char fileName[30];
	
	printf("Sending %s\n",FILE_LIST_REQUEST);
	send(peerToServerFD,FILE_LIST_REQUEST,sizeof(FILE_LIST_REQUEST),0);	
	
    recv(peerToServerFD,reply,MAX_MESSAGE_SIZE,0);
	
	printf("File list received successfully %s\n",reply);
	
	offset=0;
	offset+=MESSAGE_HEADER_LENGTH;
	memcpy(&numberOfFiles,reply+offset,sizeof(int));
	offset+=sizeof(int);	
	printf("Number of files %d\n",numberOfFiles);
	for(int i=0;i<numberOfFiles;i++)
	{		
		memcpy(&fileNameSize,reply+offset,sizeof(int));
		offset+=sizeof(int);
		
		// printf("%d is the file name size\n",fileNameSize);
		
		memcpy(fileName,reply+offset,fileNameSize+1);
		offset+=strlen(fileName)+1;
		
		memcpy(&fileSize,reply+offset,sizeof(int));
		offset+=sizeof(int);
		
		printf("File %d's name:%s and size:%d \n",i+1,fileName,fileSize);
	}
	
	
}

void fileLocationRequest(char* fileName)
{
	int totalMessageSize=0,offset=0,chunkID;
	int fileNameSize=strlen(fileName),numberOfChunks,size,ipSize,port,chunkNameLength,fileNameLength;
	char reply[MAX_MESSAGE_SIZE];
	char chunkName[50],ipAddress[50];
	

	totalMessageSize+=(strlen(fileName)+MESSAGE_HEADER_LENGTH+sizeof(int));
	
	char *request=(char *)malloc(sizeof(char)*totalMessageSize);
	bzero(request,totalMessageSize);
	
	memcpy(request+offset,FILE_LOCATION_REQUEST,MESSAGE_HEADER_LENGTH);
	offset+=MESSAGE_HEADER_LENGTH;
	
	memcpy(request+offset,&fileNameSize,sizeof(int));
	offset+=sizeof(int);
	
	memcpy(request+offset,fileName,strlen(fileName)+1);
	offset+=strlen(fileName)+1;
	
	printf("Sending %s\n",FILE_LOCATION_REQUEST);
	send(peerToServerFD,request,totalMessageSize+1,0);	
	
	//Parse reply and use reply to request for chunks 
	offset=0;
    recv(peerToServerFD,reply,MAX_MESSAGE_SIZE,0);
    offset+=MESSAGE_HEADER_LENGTH;
	printf("File(Chunk) locations received\n ");
	/*Need to unmarshall the reply*/
	
	memcpy(&numberOfChunks,reply+offset,sizeof(int));
	offset+=sizeof(int);	
	
	for(int i=0;i<numberOfChunks;i++)
	{
		memcpy(&chunkID,reply+offset,sizeof(int));
		offset+=sizeof(int);
		
		/*File name length and file name*/
		memcpy(&fileNameLength,reply+offset,sizeof(int));
		offset+=sizeof(int);
		
		memcpy(fileName,reply+offset,fileNameLength+1);
		offset+=fileNameLength+1;
		
		/*Chunk name length and file name*/
		
		memcpy(&chunkNameLength,reply+offset,sizeof(int));
		offset+=sizeof(int);
		
		memcpy(chunkName,reply+offset,chunkNameLength+1);
		offset+=chunkNameLength+1;
		
		/* Size */
		memcpy(&size,reply+offset,sizeof(int));
		offset+=sizeof(int);
		
		/* IP Address */
		
		memcpy(&ipSize,reply+offset,sizeof(int));
		offset+=sizeof(int);
		
		memcpy(ipAddress,reply+offset,ipSize+1);
		offset+=ipSize+1;
		
		/* Port */

		memcpy(&port,reply+offset,sizeof(int));
		offset+=sizeof(int);
		
		printf("Chunk id %d, file name %s chunk name %s \n", chunkID, fileName, chunkName);	
		requestForFileChunk(fileName,chunkID,ipAddress,P2P_PORT);
	}
	//TODO wait for all downloads or check /spin on some variable then combine
	
	combineChunks(fileName,numberOfChunks);

}

int main(int argc, char **argv)
{
	
	    int option=-1,flag=-1;
		char fileName[50];
		
		pthread_mutex_init(&printfLock,NULL);

	    peerToServerFD=socket(DOMAIN,SOCKET_TYPE,PROTOCOL);
	    assert(peerToServerFD!=0);
	    memset(&serverAddress, '\0', sizeof(serverAddress));
	    serverAddress.sin_family=DOMAIN;
	    serverAddress.sin_port=htons(PORT);
	    //memcpy((char *) &clientAddress.sin_addr.s_addr,SERVER,SERVER_LEN);
	    inet_aton("127.0.0.1", &serverAddress.sin_addr.s_addr);
	    chunkFiles(argc,argv);
	    peerThread=malloc(sizeof(pthread_t));
		flag=pthread_create(peerThread,NULL,&peerListenerThreadHandler,NULL);
		assert(flag==0);


 		if(connect(peerToServerFD,(struct sockaddr *)&serverAddress,sizeof(serverAddress))==0)
 	    {

 		  if(DEBUG)
 		   printf("Peer Connected to Server\n");
 		  /*Options to view file list, download a file and view download status*/
 	   		  if(registerRequest(argc-1,argv)==true)
 			  		  {
 						while(1)
 					    {
 						showOptions();
 						scanf("%d",&option);
 						switch(option)
 					    {
 	  			          case 1:
 						  //Requesting for the file list
 	  			          fileListRequest();
 			               break;

 	  			          case 2:
 						  //Download a file
 						  printf("Specify file name\n");
 						  scanf("%s",fileName);
 						  fileLocationRequest(fileName);
 	  	                  break;

 			              case 3:
 						  showDownloadStatus();
 	                      break;

 						  case 4:
 	  	                  break;

 						  default:
 	  			          //cleanUp();
 					  break;
 	   		            }
 	            }
 		  }

 	}

        return 0; 
}
