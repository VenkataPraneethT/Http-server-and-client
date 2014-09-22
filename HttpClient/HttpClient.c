/*
This C code is a HTTP Client which is created to download a file from a server, saves the file to the local machine and then uploads it to the http address specified by the user.

Http Methods used are: GET and PUT.

*/
#include <stdio.h>
#include <ctype.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#define MAXBUF  500000

//POST request function

int get_file(char *ipaddr, char *port){
	int sock;
       	char httpheader[1024];
        const char *page = "/dns-query";
        const char *b=" Praneeth";
	    char poststr[256]; 
        char dnsname[100];
        char type[50];
        char content[MAXBUF];
        char s[MAXBUF];
        int bytes_read;
        int sockbufsize;
        unsigned int intlen = sizeof(int);
        
        struct addrinfo hints, *res,*rp;
	    memset(&hints, 0, sizeof hints);
	    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
	    hints.ai_socktype = SOCK_STREAM; //TCP socket
        hints.ai_protocol = 0;


     printf("Please enter the DNS name to be queried \n");
     scanf("%s", dnsname);

     printf("Please enter the type of query (ex: A, AAAA, MX etc)\n");
     scanf("%s", type);

     strcat(poststr,"Name=");
     strcat(poststr,dnsname);
     strcat(poststr,"&Type=");
     strcat(poststr,type);

     int lenpostr=strlen(poststr);

snprintf(httpheader, MAXBUF, "POST %s HTTP/1.1\r\nHost: %s\r\nIam:%s\r\nContent-type: application/x-www-form-urlencoded\r\nContent-length: %d\r\n\r\n%s", page, ipaddr,b, lenpostr, poststr);

     //printf("%s\n",ipaddr);

	if ( getaddrinfo(ipaddr, port, &hints, &res) != 0 ){
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit(0);
		}

	// Establish a socket connection
           
      for (rp = res; rp != NULL; rp = rp->ai_next) {
              sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
               if (sock == -1)
                   continue;

                if ( connect(sock, res->ai_addr, res->ai_addrlen) != 0 ){
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit(0);
		} else {break;}                  /* Success */

               close(sock);
           }

           if (rp == NULL) {               /* No address succeeded */
               fprintf(stderr, "unable to connect\n");
               exit(EXIT_FAILURE);
           }        

if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF,
		       &sockbufsize, &intlen) < 0) {
		perror("getsockopt");
		return -1;
	}
  
        int optval = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

	write(sock, httpheader, strlen(httpheader));

       bzero(s, sizeof(s));
     do
     { 
      bytes_read = recv(sock, s, sizeof(s), 0);
      strcat(content,s);
      bzero(s, sizeof(s));
     }
     while(bytes_read>0);
      
    printf("Your POST request has been completed with the following response \n %s \n", content);
  
return sock;
}

//getrequest function

int Request(char *ipaddr, char *port){      //This function is created for HTTP GET Request.

	int sockfd;              //In this function socket creation, connection and HTTP GET header formatting takes place.
        char  path[100];   
	    char getrequest[1024];
        const char *b=" Praneeth";
        int sockbufsize;
        char s[MAXBUF];
        unsigned int intlen = sizeof(int);
        char filename[100];
        int bytes_read;
        char *content[3];

        struct addrinfo hints, *res, *rp;           //using addressinfo struct
	    memset(&hints, 0, sizeof hints);
	    hints.ai_family = AF_UNSPEC;           //Can handle IP address family of either IPV4 or IPV6
	    hints.ai_socktype = SOCK_STREAM;

        if ( getaddrinfo(ipaddr, port, &hints, &res) != 0 ){
	      fprintf(stderr, "Either of hostname or IP address is not valid\n");
	      exit(0);
	          }

	     for (rp = res; rp != NULL; rp = rp->ai_next) {
              sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
               if (sockfd == -1)
                   continue;

               if ( connect(sockfd, res->ai_addr, res->ai_addrlen) != 0 ){
		        fprintf(stderr, "ERROR: %s\n", strerror(errno));
		         exit(0);
		         } else {break;}                  /* Succeded */
               
                 close(sockfd);
                 }

           if (rp == NULL) {               /* No address succeeded */
               fprintf(stderr, "unable to connect\n");
               exit(EXIT_FAILURE);
           }        

        if (getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF,
		       &sockbufsize, &intlen) < 0) {
		perror("getsockopt");
		return -1;
	}

	    int optval = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
        
        printf("Please enter the filename to be downloaded preceeding with slash '/' ex: /test.txt\n");
        scanf("%s",path);

        sprintf(getrequest, "GET %s HTTP/1.1\r\nHOST: %s\r\nIam:%s\r\n\r\n", path, ipaddr,b);

	    write(sockfd, getrequest, strlen(getrequest));

        printf("Please enter the path and the filename to save it in that location\n");
        scanf("%s",filename);

        FILE *fp = fopen(filename, "w+");	//Opening a file.
       
        bzero(s, sizeof(s));
        bytes_read = recv(sockfd, s, sizeof(s), 0);

        if (bytes_read<0)    // receive error
           fprintf(stderr,("received() error\n"));
        else if (bytes_read==0)    // receive socket closed
           fprintf(stderr,"Server disconnected upexpectedly.\n");
         else    // message received
          {
           content[0]=strtok(s,"\r\n\r\n");
           content[1]=strtok(NULL, "\r\n\r\n");
           content[2]=strtok(NULL, "\r\n\r\n");
           content[3]=strtok(NULL, "\r\n\r\n");
           content[4]=strtok(NULL, "\r\n\r\n");
           content[5]=strtok(NULL, "\r\n\r\n");
           content[6]=strtok(NULL,"\0");
           fwrite(content[6], strlen(content[6]) + 1, 1, fp);
          }

      fclose(fp);

printf("Your GET request has been completed with the following response %s \n", content[0]);

return sockfd;
}

//put request function
int putrequest(char *ipaddr, char *port){    //This function is created for HTTP PUT Request. HTTP PUT request formatting takes place.

       int sockfd; 
       const char *ct = "text/plain";
       char putsrequest[MAXBUF];
       char filename[100];
       char filepath[100];
       char entirefilepath[256];
       struct addrinfo hints, *res, *rp ;
       memset(&hints, 0, sizeof hints);
       hints.ai_family = AF_UNSPEC;
       hints.ai_socktype = SOCK_STREAM;
       int i=0,f=0,n, totbytes=0;
       const char *c=" Praneeth";
       char a[MAXBUF];
       char *content[2];
       char s[MAXBUF];
       int bytes_read;

  printf("Please enter the path where the file is located\n");
  scanf("%s",filepath);
 
  strcat(entirefilepath,filepath);
  strcat(entirefilepath,"/");

  printf("Please enter the filename\n");
  scanf("%s",filename);

  strcat(entirefilepath,filename);
  printf("%s\n",entirefilepath);


    FILE *fp = fopen(entirefilepath, "r");   //opening the file from the local desktop to read.

    while((a[i++] = fgetc(fp))!= EOF)
           ;                                                //Fetching the contents of the file.

     a[i] ='\0';
     f=strlen(a);

snprintf(putsrequest,MAXBUF,"PUT %s HTTP/1.1\r\nHost: %s\r\nIam:%s\r\nContent-Type:%s\r\nContent-Length: %d\r\n\r\n %s\r\n",filename,ipaddr,c,ct,f,a);

	if ( getaddrinfo(ipaddr, port, &hints, &res) != 0 ){
		fprintf(stderr, "Either of hostname or IP address is not valid\n");
		exit(0);
		}


	 for (rp = res; rp != NULL; rp = rp->ai_next) {
              sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
               if (sockfd == -1)
                   continue;

               if ( connect(sockfd, res->ai_addr, res->ai_addrlen) != 0 ){
		         fprintf(stderr, "ERROR: %s\n", strerror(errno));
		         exit(0);
		         } else {break;}                  /* Success */

               close(sockfd);
                 }

           if (rp == NULL) {               /* No address succeeded */
               fprintf(stderr, "Could not connect\n");
               exit(EXIT_FAILURE);
           }        


     	int optval = 1;

        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
 
if ( (n = write(sockfd, putsrequest, strlen(putsrequest))) > 0) {
		totbytes += n;
		printf("wrote %d bytes, total %d bytes\n", n, totbytes);
		
	}
else {
		perror("write error");
		return 1;
	}

       pclose(fp);
       bzero(s, sizeof(s));
       bytes_read = recv(sockfd, s, sizeof(s), 0);
   
    if (bytes_read<0)    // receive error
        fprintf(stderr,("received() error\n"));
    else if (bytes_read==0)    // receive socket closed
        fprintf(stderr,"Server disconnected upexpectedly.\n");
    else    // message received
    {
       content[0]=strtok(s,"\r\n\r\n");
       printf("Your PUT request has been completed with the following response %s \n", content[0]);
    }
return sockfd;
}

//main function
int main(int argc, char **argv){   //Main function.

	int port, sockfd;
	char *host, *ptr;
        char choice[10];
      
        if ( argc != 3) {                  //validating the input arguments.
                fprintf(stderr, "Usage: hostname port \n");
                exit(0);
        }

        port = atoi(argv[2]);
	    host = argv[1];
    
	if ( (ptr = strstr(host, "http://")) != NULL || (ptr = strstr(host, "https://")) != NULL )

         {                                               //validating the hostnames that user enters
		   host = host + 7;
         }


        if ( port > 65536 || port < 0 ){  //validating the ports
                fprintf(stderr, "Invalid port number\n");
                exit(0);
        }

	printf("Please enter your choice \nGET\nPUT\nPOST\n");
        scanf("%[^\n]",choice);	

if (strcmp(choice,"get\0")==0 || strcmp(choice,"GET\0")==0)
       {  
     
      sockfd = Request(host, argv[2]);
      }

else  if ((strcmp(choice, "put")==0)|| (strcmp(choice, "PUT")==0))
   {
   sockfd = putrequest(host, argv[2]);
   } 

else if ((strcmp(choice, "POST")==0)|| (strcmp(choice, "post")==0))
{
  sockfd = get_file(host, argv[2]);
}

else{
printf("This application doesn't support such requests\n");
}

close(sockfd);

return 0;

}
