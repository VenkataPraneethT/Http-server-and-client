/* This application is a HTTP server application which supports HTTP GET, PUT and POST requests. 
The code to daemonise the server is borrowed from the lecture. 
The code for DNS parsing is borrowed from the "http://www.binarytides.com/dns-query-code-in-c-with-winsock/" and is modified as per the needs.
*/


//Header Files
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<dirent.h>
#include<netinet/in.h>
#include<errno.h>
#include<fcntl.h>
#include<syslog.h>


 
#define T_A 1 //Ipv4 address
#define T_NS 2 //Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6 /* start of authority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 //Mail server
#define	MAXFD	64
#define CONNMAX 1000
#define MAXLINE 500000


 
//List of DNS Servers registered on the system
char dns_servers[10][100];
int dns_server_count = 0;
char *ROOT;
int listenfd, clients[CONNMAX];
//void error(char *);
//Types of DNS resource records

//Function Prototypes
void hostbyname (unsigned char* , int, int n);
void dnsnameconversion (unsigned char*,unsigned char*);
unsigned char* dnspar (unsigned char*,unsigned char*,int*);
void dnsserverquery();
void sendresponse(char *reqline, int n);
void getputresponse(int n,char *hostname,char* PORT);

//DNS header structure
struct DNS_HEADER
{
    unsigned short id; // identification number
 
    unsigned char rd :1; // recursion desired
    unsigned char tc :1; // truncated message
    unsigned char aa :1; // authoritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag
 
    unsigned char rcode :4; // response code
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authenticated data
    unsigned char z :1; 
    unsigned char ra :1; // recursion available
 
    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};
 
//Constant sized fields of query structure
struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
};
 
//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)
 
//Pointers to resource record contents
struct RES_RECORD
{
    unsigned char *name;
    struct R_DATA *resource;
    unsigned char *rdata;
};
 
//Structure of a Query
typedef struct
{
    unsigned char *name;
    struct QUESTION *ques;
} QUERY;


//daemon function
int daemon_init(const char *pname, int facility) // Borrowed code from the lecture (the entire daemon function)
{
	int	i;
	pid_t	pid;

	
	if ( (pid = fork()) < 0)
		return -1;  
	else if (pid)
		exit(0);			
	
	if (setsid() < 0)			
		return -1;

	signal(SIGHUP, SIG_IGN);

	if ( (pid = fork()) < 0)
		return -1;
	else if (pid)
		exit(0);			

	chdir("/");				

	for (i = 0; i < MAXFD; i++)
		close(i);

	open("/dev/null", O_RDONLY); // fd 0 == stdin
	open("/dev/null", O_RDWR); // fd 1 == stdout
	open("/dev/null", O_RDWR); // fd 2 == stderr

	// open syslog
	openlog(pname, LOG_PID, facility);

	return 0;				
}


//start server function
void beginserver(char *port)
{
    struct addrinfo hints, *serverinf, *p;
    int s;
    // getaddrinfo for host
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
   
if ((s = getaddrinfo(NULL, port, &hints, &serverinf)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(1);
}

// loop through all the results and bind to the first we can
for(p = serverinf; p != NULL; p = p->ai_next) {
    if ((listenfd = socket(p->ai_family, p->ai_socktype,0)) == -1) {
        perror("socket");
        continue;
    }

    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(listenfd);
        perror("bind");
        continue;
    }

    break;// connected successfully
}

if (p == NULL) {
    // looped off the end of the list with no successful bind
    fprintf(stderr, "failed to bind socket\n");
    exit(2);
}

    freeaddrinfo(serverinf);

    // listen for incoming connections
    if ( listen (listenfd, 1000000) != 0 )
    {
        perror("listen() error");
        exit(1);
    }
}

//POST response

//client connection
void sendresponse(char *reqline, int n)
{
   
    char dnsname[100];
    unsigned char domainname[100];
    char querytype[100];
    int lengths=0;
          printf("reqline is %s\n",reqline);              
     if ( strncmp( reqline, "Name=", 5)==0)
         {
            strcpy(querytype, reqline);
            strcpy(dnsname, &querytype[strlen("Name=")]);
            lengths = strcspn (dnsname,"&");
            printf("lengths is %d\n",lengths);
            strncpy((char *)domainname, dnsname, lengths );
            if (lengths > 0)
            domainname[lengths]= '\0';
            printf("dnsname is %s\n", domainname); 
            hostbyname(domainname , T_A, n); 
            
                                                        
         }       

}

//getputfunction for response

void getputresponse(int n,char *hostname,char* PORT)
{
    char mesg[MAXLINE], *reqline[13],  data_to_send[MAXLINE], path[MAXLINE],content[MAXLINE];
    int received, fd, bytes_read;
    char *header;
    const char *c=" Praneeth";
    
    header = (char*)malloc(1024*sizeof(char));

    memset( (void*)mesg, (int)'\0', MAXLINE );

      
    received=recv(clients[n], mesg, MAXLINE, 0); 
           
          
         if (received<0)    // receive error
               {
               fprintf(stderr,("received() error\n"));
               } 
           else if (received==0)    // receive socket closed
               { 
                fprintf(stderr,"Client disconnected upexpectedly.\n");
               }      
           else
               {          
        
                reqline[0] = strtok (mesg, " \r\n");
                         
     if ( (strncmp(reqline[0], "POST\0", 5)==0 ) || (strncmp(reqline[0], "GET\0", 4)==0 ) || (strncmp(reqline[0], "PUT\0", 4)==0 ) )  
               {      
                       
                   reqline[1] = strtok (NULL, " \r\n");
                   reqline[2] = strtok (NULL, "\r\n\r\n");
                   reqline[3] = strtok (NULL, "\r\n\r\n");
                   reqline[4] = strtok (NULL, "\r\n\r\n");
                   reqline[5] = strtok (NULL, "\r\n\r\n");
                   reqline[6] = strtok (NULL, "\r\n\r\n"); 
                   reqline[7] = strtok (NULL, "\r\n\r\n");
                   reqline[8] = strtok (NULL, "\0");
                   
                  
                 if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
                      {
                          write(clients[n], "HTTP/1.1 400 Bad Request\n", 25);
                      }
                  
                   if ( strncmp(reqline[0], "POST\0", 5)==0 )
                                 {
                              
                                 if ( strncmp(reqline[1], "/dns-query", 10)==0 )
                                     {
                                       dnsserverquery();
                                       sendresponse(reqline[7],n);
                                       
                                      } 
                                 }  


                        if ( strncmp(reqline[0], "GET\0", 4)==0 ) 
                                    {
                                 if ( strncmp(reqline[1], "/\0", 2)==0 )
                                       {
                                      reqline[1] = (char *)"/index.html";//if no file is specified, index.html will be opened by default
                                       }
                                      else {
                                       strcpy(path, ROOT);
                                       strcpy(&path[strlen(ROOT)], reqline[1]);
                                       printf("file: %s\n", path);
                                          if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
                                             {
                                               send(clients[n], "HTTP/1.1 200 OK\r\n", 17, 0);
                                               while ( (bytes_read=read(fd, data_to_send, MAXLINE))>0 )
                                              {  
sprintf(header,"Hostname: %s:%s\r\nIam:%s\r\nLocation: %s\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", hostname, PORT,c, ROOT,bytes_read);
                                              send(clients[n], header, strlen(header), 0);
                                              write (clients[n], data_to_send, bytes_read);
                                              }
                                            }
                                         else   
                                             { 
                                             write(clients[n], "HTTP/1.1 404 Not Found\n", 23); //FILE NOT FOUND
                                             }
                                          }
                                   }

                        if ( strncmp(reqline[0], "PUT\0", 4)==0 )

                             {
                               strcpy(path, ROOT);
                               strcat(&path[strlen(ROOT)],"/\0");
                               strcpy(&path[strlen(ROOT)+1], reqline[1]);
                               printf("file: %s\n", path);
                               fd=open(path, O_RDONLY);                                
                               FILE *fp = fopen(path, "w+");
                                strcat(content,reqline[8]); 
                                 while (received >0 )
                                    {
                                    memset( (void*)mesg, (int)'\0', MAXLINE );
                                    received=recv(clients[n], mesg, MAXLINE, MSG_DONTWAIT);
                                    strcat(content,mesg); 
                                    }
                                  fwrite(content, strlen(content) + 1, 1, fp);
                                                           
                               fclose(fp);
                              if (fd!=-1 ) 
                                 {
                                send(clients[n], "HTTP/1.1 200 OK\r\n", 17, 0);
                                  }
                              else 
                                 {
                                 send(clients[n], "HTTP/1.1 201 Created\r\n", 17, 0);
                                 }

                              }
                       }
                             
               }
    shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
    close(clients[n]);
    clients[n]=-1;
}



//main function
int main( int argc , char *argv[])
{
   
  struct sockaddr_in clientaddr;
   socklen_t addrlen;
   char *hostname;
   char PORT[6];
   ROOT = getenv("PWD");
   int index=0;

if (argc != 3) {
    fprintf(stderr, "usage:serv [hostname] [port]\n");
    exit(1);
  }
  
  hostname = argv[1];
  strcpy(PORT,argv[2]);    

 daemon_init(argv[0], LOG_WARNING);
     
  int i;
    for (i=0; i<CONNMAX; i++)
        {
     clients[i]=-1;
      }
    beginserver(PORT);

while (1)   // ACCEPT connections
    {
        addrlen = sizeof(clientaddr);
        clients[index] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[index]<0)
         {  
         printf( "accept() error: %s\n", strerror( errno ) );
         }
         else
        {
            if ( fork()==0 )
            {
                
                getputresponse(index,hostname,PORT);
                exit(0);
            }
        }
            
        while (clients[index]!=-1) index = (index+1)%CONNMAX;
    }
     
    
  return 0;
}
 
//DNS query by sending a packet

void hostbyname(unsigned char *host , int query_type, int n)
{
    unsigned char buf[65536],*qname,*reader;
    int i , j , stop , s;
    const char *header1 = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\nContent-length: 500\r\n\r\n";
    struct sockaddr_in a;
 
    struct RES_RECORD reply[20];  //the replies from the DNS server
    struct sockaddr_in dest;
 
    struct DNS_HEADER *dns = NULL;
    struct QUESTION *qinfo = NULL;
    //char *header = NULL;
    
 
    s = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP); //UDP packet for DNS queries
 
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = inet_addr(dns_servers[0]); //dns servers
    
    //Set the DNS structure to standard queries
    dns = (struct DNS_HEADER *)&buf;
 
    dns->id = (unsigned short) htons(getpid());
    dns->qr = 0; //This is a query
    dns->opcode = 0; //This is a standard query
    dns->aa = 0; //Not Authoritative
    dns->tc = 0; //This message is not truncated
    dns->rd = 1; //Recursion Desired
    dns->ra = 0; //Recursion not available
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->q_count = htons(1); //we have only 1 question
    dns->ans_count = 0;
    dns->auth_count = 0;
    dns->add_count = 0;
 
    //point to the query portion
    qname =(unsigned char*)&buf[sizeof(struct DNS_HEADER)];
    
    dnsnameconversion(qname , host);
    qinfo =(struct QUESTION*)&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)]; //fill it
 
    qinfo->qtype = htons( query_type ); //type of the query
    qinfo->qclass = htons(1); 
 
   
    if( sendto(s,(char*)buf,sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION),0,(struct sockaddr*)&dest,sizeof(dest)) < 0)
    {
        perror("sendto failed");
    }
    
    //Receive the answer
    i = sizeof dest;
  
    if(recvfrom (s,(char*)buf , 65536 , 0 , (struct sockaddr*)&dest , (socklen_t*)&i ) < 0)
    {
        perror("recvfrom failed");
    }
    
 
    dns = (struct DNS_HEADER*) buf;
 
    //move ahead of the dns header and the query field
    reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION)];
 

    //Start reading answers
    stop=0;
 
    for(i=0;i<ntohs(dns->ans_count);i++)
    {
        reply[i].name=dnspar(reader,buf,&stop);
        reader = reader + stop;
 
        reply[i].resource = (struct R_DATA*)(reader);
        reader = reader + sizeof(struct R_DATA);
 
        if(ntohs(reply[i].resource->type) == 1) //if its an ipv4 address
        {
            reply[i].rdata = (unsigned char*)malloc(ntohs(reply[i].resource->data_len));
 
            for(j=0 ; j<ntohs(reply[i].resource->data_len) ; j++)
            {
                reply[i].rdata[j]=reader[j];
            }
 
            reply[i].rdata[ntohs(reply[i].resource->data_len)] = '\0';
 
            reader = reader + ntohs(reply[i].resource->data_len);
        }
        else
        {
            reply[i].rdata = dnspar(reader,buf,&stop);
            reader = reader + stop;
        }
    }
  
 send(clients[n], header1, strlen(header1), 0);  
  //get answers
   for(i=0 ; i < ntohs(dns->ans_count) ; i++)
    {
      
        send(clients[n], reply[i].name, strlen((char* )reply[i].name), 0);
        if( ntohs(reply[i].resource->type) == T_A) //IPv4 address
        {
            long *p;
            p=(long*)reply[i].rdata;
            a.sin_addr.s_addr=(*p); //working without ntohl
            send(clients[n], " has  IPV4 address: ", strlen("has  IPV4 address: "), 0);
            send(clients[n], inet_ntoa(a.sin_addr), strlen((char*)inet_ntoa(a.sin_addr)), 0);
        }
         
        if(ntohs(reply[i].resource->type)==5) 
        {
            //Canonical name for an alias
            send(clients[n], " has alias name : ", strlen("has alias name : "), 0);
            send(clients[n], reply[i].rdata, strlen((char *)reply[i].rdata), 0);
        }
 
        send(clients[n], "\r\n", strlen("\r\n"), 0);
   
 }
    
    return;
}


//DNS parsing function
u_char* dnspar(unsigned char* reader,unsigned char* buffer,int* count)
{
    unsigned char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;
 
    *count = 1;
    name = (unsigned char*)malloc(256);
 
   name[0]='\0';
 
    //read the names in 3www6google3com format
    while(*reader!=0)
    {
        if(*reader>=192)
        {
            offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000
            reader = buffer + offset - 1;
            jumped = 1; 
        }
        else
        {
            name[p++]=*reader;
        }
 
        reader = reader+1;
 
        if(jumped==0)
        {
            *count = *count + 1; 
        }
    }
 
    name[p]='\0'; //string complete

    if(jumped==1)
    {
        *count = *count + 1; //number of steps we actually moved forward in the packet
    }
 
    //now convert 3www6google3com0 to www.google.com
    for(i=0;i<(int)strlen((const char*)name);i++) 
    {
        p=name[i];
        for(j=0;j<(int)p;j++) 
        {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    name[i-1]='\0'; //To remove the last dot
    return name;
}
 

 //Fetch DNS servers from /etc/resolv.conf file on Linux

void dnsserverquery()
{
    FILE *fp;
    char line[200] , *p;
    if((fp = fopen("/etc/resolv.conf" , "r")) == NULL)
    {
        printf("Failed opening /etc/resolv.conf file \n");
    }
     
    while(fgets(line , 200 , fp))
    {
        if(line[0] == '#')
        {
            continue;
        }
        if(strncmp(line , "nameserver" , 10) == 0)
        {
            p = strtok(line , " ");
            p = strtok(NULL , " ");
            strcpy(dns_servers[0] , p);
                       
        }
    }
     
    strcpy(dns_servers[0] , "208.67.222.222");
    strcpy(dns_servers[1] , "208.67.220.220");
}
 
//Function to convert www.google.com to 3www6google3com 

void dnsnameconversion(unsigned char* dns,unsigned char* host) 
{
    int ks = 0 , i;
    strcat((char *)host,".");
    int hl=strlen((char *)host); 
    for(i = 0 ; i < hl ; i++) 
    {
        if(host[i]=='.') 
        {
            *dns++ = i-ks;
            for(;ks<i;ks++) 
            {
                *dns++=host[ks];
            }
            ks++; 
        }
    }
    *dns++='\0';
}
