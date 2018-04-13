#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/inotify.h>
#include "file_queue.h"
#include <signal.h>
#include "event_queue.h"
#include "inotify_utils.h"

//
#include<netinet/in.h> // sockaddr_in 
#include<sys/types.h>  // socket 
#include<sys/socket.h> // socket 
#include<stdio.h>    // printf 
#include<stdlib.h>   // exit 
#include<string.h>   // bzero 
#include <pthread.h>
#include <signal.h>
#include<dirent.h>
#define SERVER_PORT 8001 
#define LENGTH_OF_LISTEN_QUEUE 20 
#define BUFFER_SIZE 1024 
#define FILE_NAME_MAX_SIZE 512 
void *connection_handler(void *new_server_socket_fd);
void *process();
void *getuser();
void listfiles(char *dir,struct Queue* res);
void signal_handler (int signum);
int keep_running;
struct Queue *fn;//
struct QNode *cfn;
char serverip[20];
char* fail="/fail";
char* success="/success";
char* delete="/delete";
int id=0;
  //client socket address
struct sockaddr_in client_addr; 
socklen_t client_addr_length = sizeof(client_addr); 
pthread_t pro,acc;
int new_server_socket_fd,*new_sock,server_socket_fd ;
volatile int transmitting=0;
volatile int ex=0;
volatile int total=0;
volatile int newuser=0;//process will wait till newuser becomes 0 to continue all broadcast
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
volatile int pass=0;
char path[512];
char temp[512];
int flag=-1;
/* This program will take as arguments one or more directory 
   or file names, and monitor them, printing event notifications 
   to the console. It will automatically terminate if all watched
   items are deleted or unmounted. Use ctrl-C or kill to 
   terminate otherwise.
*/

////////////////////////////////////////////////



int main ()
{
	bzero(temp,512);
	bzero(path,512);
	printf("please input the directory to watch\nto watch current directory where this program is, type ./\n");
	scanf("%512[^\n]",temp);
	while(flag<0){
		if (0 != access(temp, F_OK)) {
			perror("errors occurs when finding the directory\n");
			while(getchar()!='\n');
			printf("please input the directory to watch\nto watch current directory where this program is, type ./\n");
			scanf("%512[^\n]",temp);
		}
		else{
			while(getchar()!='\n');
			printf("directory:\n%s\nexists\n",temp);
			realpath(temp, path);
			path[strlen(path)]='/';
			bzero(temp,BUFFER_SIZE);
			printf("files from server will be saved to\n%s\n",path);
			flag=1;
		}		
	}
  /* This is the file descriptor for the inotify watch */
	int inotify_fd;
	fn=createQueue();//
  // declare and initinalize a server socket 
  	printf("please input the server ip for binding, like:192.168.56.101\n");
	scanf("%s",serverip);
	//strcpy(serverip,"192.168.56.101");
	while(getchar()!='\n');
	
	struct sockaddr_in server_addr; 
	bzero(&server_addr, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = inet_addr(serverip);
	server_addr.sin_port = htons(SERVER_PORT); 
   // create the socket 
	server_socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
	if(server_socket_fd < 0) { 
		perror("Create Socket Failed:"); 
		exit(1); 
	} 
	int opt = 1; 
	setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 
  
  // bind the server socket 
	if(-1 == (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)))) { 
		perror("Server Bind Failed:"); 
		exit(1); 
	} 
    
  // socket listen
	if(-1 == (listen(server_socket_fd, LENGTH_OF_LISTEN_QUEUE))) { 
		perror("Server Listen Failed:"); 
		exit(1); 
	} 
	if(pthread_create(&pro, NULL,process , NULL)) {
		fprintf(stderr, "Error creating thread\n");
		return 1;
	}
	if(pthread_create(&acc, NULL,getuser , NULL)) {
	fprintf(stderr, "Error creating thread\n");
	return 1;
	}
	keep_running = 1;

  /* Set a ctrl-c signal handler */
	if (signal (SIGINT, signal_handler) == SIG_IGN){
      /* Reset to SIG_IGN (ignore) if that was the prior state */
      signal (SIGINT, SIG_IGN);
    }

  /* First we open the inotify dev entry */
	inotify_fd = open_inotify_fd ();
	if (inotify_fd > 0){

      /* We will need a place to enqueue inotify events,
         this is needed because if you do not read events
         fast enough, you will miss them. This queue is 
         probably too small if you are monitoring something
         like a directory with a lot of files and the directory 
         is deleted.
       */
    queue_t q;
    q = queue_create (128);

      /* This is the watch descriptor returned for each item we are 
         watching. A real application might keep these for some use 
         in the application. This sample only makes sure that none of
         the watch descriptors is less than 0.
       */
    int wd;


      /* Watch all events (IN_ALL_EVENTS) for the directories and 
         files passed in as arguments.
         Read the article for why you might want to alter this for 
         more efficient inotify use in your app.      
       */
    wd = 0;
    printf("\n");
	 // wd = watch_dir (inotify_fd, path, IN_ALL_EVENTS);IN_ACCESS
//wd = watch_dir (inotify_fd, path, IN_ALL_EVENTS & ~IN_ACCESS);
		wd = watch_dir (inotify_fd, path, IN_ALL_EVENTS);
	  /*wd = watch_dir (inotify_fd, path, IN_ALL_EVENTS & ~(IN_CLOSE | IN_OPEN) ); */
    if (wd > 0) {
	  /* Wait for events and process them until a 
             termination condition is detected
           */
	  process_inotify_events (q, inotify_fd);
	}
    printf ("\nServer is terminating.\nIf you didn't close the server, it might because the directory monitored by the server is not there\n");
	printf("There are %d user connected to the server\n",newuser+total);
	printf("There are %d event need to be transfered to users\n",fn->capacity);
	printf("Thanks for using this\n");
      /* Finish up by closing the fd, destroying the queue,
         and returning a proper code
       */
    close_inotify_fd (inotify_fd);
    queue_destroy (q);
    }
	return 0;
}
void *connection_handler(void *new_server_socket_fd)
{
    //Get the socket descriptor
	char filename[512];
	char buffer[BUFFER_SIZE]; 
	struct Queue *nu;//for new user
	struct QNode *cu;
	nu=createQueue();
	listfiles(path,nu);
	
    int sock = *(int*)new_server_socket_fd;
    if(sock < 0) { 
		perror("Server Accept Failed:"); 
   //   return 0; 
    } 
	signal(SIGPIPE, SIG_IGN);
	
	while(nu->capacity!=0){
		cu=deQueue(nu);
		printf("fn:%s,pu:%d\n",cu->fn,cu->purpose);
		
			bzero(buffer, BUFFER_SIZE);  
			bzero(filename,512);
			strcat(filename,path);
			strcat(filename,cu->fn);
			printf("fn:%s,pu:%d\n",cu->fn,cu->purpose);

			FILE *fp = fopen(filename, "r"); 
			if(NULL == fp) { 
				printf("File:%s Not Found\n",filename); 
				if(send(sock, fail, (strlen(fail)+1)*sizeof(char), 0)<0){
					perror("Connection Failed./n"); 
					newuser--;
					printf("disconnect with this client\nonly %d clients & %d new users now\n",total,newuser);
					fclose(fp);
					pthread_exit(&sock);
				}
			} 
			else { 
				if(send(sock, cu->fn, (strlen( cu->fn) + 1 ) * sizeof(char), 0)<0){
					perror("Connection Failed./n"); 
					newuser--;
					printf("disconnect with this client\nonly %d clients & %d new users now\n",total,newuser);
					fclose(fp);
					pthread_exit(&sock);
				}
				bzero(buffer, BUFFER_SIZE); 
				int length = 0; 
				int ret=-1;
      //send  to the client every line the server red
				while((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) { 
					while(ret!=length){
						if((ret=send(sock, buffer, length, 0)) < 0){
							perror("Connection Failed when sending a file./n"); 
							newuser--;
							printf("disconnect with this client\nonly %d clients & %d new users now\n",total,newuser);
							fclose(fp);
							pthread_exit(&sock);
						}
				//		printf("%s+%d\n",buffer,length);

						bzero(buffer, BUFFER_SIZE); 
						printf("%d characters received %d\n",ret,length);
						sleep(2);
					}
 //recv(sock, &ret, sizeof(char), 0);
					sleep(2);
					ret=-1;
					length=0;
   
				} 
				sleep(2);
				if(send(sock, ":EOF", 4, 0)<0){
					perror("Connection Failed before finishing sending a file./n"); 
					newuser--;
					printf("disconnect with this client\nonly %d clients & %d new users now\n",total,newuser);
					fclose(fp);
					pthread_exit(&sock);
				}
				fclose(fp); 
				printf("File:%s Transfer Successful!\n", filename); 
				sleep(2);
			}
	}
	total++;
	newuser--;
//prevent from a client closed and a kill signal will be sent to the server
	while(1){
		sleep(1);
		if(ex==1)break;
		else if (transmitting==1){
			bzero(buffer, BUFFER_SIZE);  
			bzero(filename,512);
			strcat(filename,path);
			strcat(filename,cfn->fn);
			printf("fn:%s,pu:%d\n",cfn->fn,cfn->purpose);
			if(cfn->purpose==0){

				FILE *fp = fopen(filename, "r"); 
				if(NULL == fp) { 
					printf("File:%s Not Found\n",filename); 
					if(send(sock, fail, (strlen(fail)+1)*sizeof(char), 0)<0){
						perror("Connection Failed./n"); 
						total--;
						printf("disconnect with this client\nonly %d clients now\n",total);
						fclose(fp);
						pthread_exit(&sock);
					}
				} 
				else { 
					if(send(sock, cfn->fn, (strlen( cfn->fn) + 1 ) * sizeof(char), 0)<0){
						perror("Connection Failed./n"); 
						total--;
						printf("disconnect with this client\nonly %d clients now\n",total);
						fclose(fp);
						pthread_exit(&sock);
					}
					bzero(buffer, BUFFER_SIZE); 
					int length = 0; 
					int ret=-1;
      // send each line read by the server 
					while((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) { 
						while(ret!=length){
							if((ret=send(sock, buffer, length, 0)) < 0){
								perror("Connection Failed when sending a file./n"); 
								total--;
								printf("disconnect with this client\nonly %d clients now\n",total);
								fclose(fp);
								pthread_exit(&sock);
							}
						//	printf("%s+%d\n",buffer,length);

							bzero(buffer, BUFFER_SIZE); 
							printf("%d characters received %d\n",ret,length);
							sleep(2);
						}
 //recv(sock, &ret, sizeof(char), 0);
						sleep(2);
						ret=-1;
						length=0;
					} 
					sleep(2);
					if(send(sock, ":EOF", 4, 0)<0){
						perror("Connection Failed before finishing sending a file./n"); 
						total--;
						printf("disconnect with this client\nonly %d clients now\n",total);
						fclose(fp);
						pthread_exit(&sock);
					}
					fclose(fp); 
					printf("File:%s Transfer Successful!\n", filename); 
					sleep(2);
				}
			}
			else if(cfn->purpose==1){
				printf("Delete file:%s\n",filename); 
				if(send(sock, delete, (strlen(delete)+1)*sizeof(char), 0)<0){
					perror("Connection Failed./n"); 
					total--;
					printf("disconnect with this client\nonly %d clients now\n",total);
					pthread_exit(&sock);
				}	
				sleep(2);
				if(send(sock, cfn->fn, (strlen(cfn->fn)+1)*sizeof(char), 0)<0){
					perror("Connection Failed./n"); 
					total--;
					printf("disconnect with this client\nonly %d clients now\n",total);
					pthread_exit(&sock);
				}
				sleep(2);
			}
			else{
				printf("undefined: \nfilename:%s\npurpose:%d\n",cfn->fn,cfn->purpose);
			}
			pass=pass+1;
	        pthread_mutex_lock(&mut);
			while (pass !=0) {
				pthread_cond_wait(&cond, &mut);
			}
          /* operate on x and y */
			pthread_mutex_unlock(&mut);
		}
	}
	printf("%d user left",--total);
  
    close(sock); 
    //Free the socket pointer
   // free(sock); 
    return 0;
}
void *process(){
	while ( fn->capacity==0||newuser!=0)sleep(5);
	cfn=deQueue(fn);
    transmitting=1;
	while(1){
		sleep(2);
		//while(total!=pass){sleep(6);}
		pthread_mutex_lock(&mut);
          /* modify x and y */
        if (pass==total) {
			while ( fn->capacity==0||newuser!=0)sleep(3);
			cfn=deQueue(fn);
			transmitting=1;
			pass=0;
			sleep(1);
			pthread_cond_broadcast(&cond);
		}
        pthread_mutex_unlock(&mut);
	}
}
void *getuser(){
	while((new_server_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_length))){
		pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_server_socket_fd;
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0){
            perror("could not create thread");
            //return 1;
        }
		else{
			printf("%d user connect\n",++newuser);
		}
	}
}
/* Signal handler that simply resets a flag to cause termination */
void signal_handler (int signum){
	keep_running = 0;
}
void listfiles(char *dir,struct Queue* res)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    if((dp = opendir(dir)) == NULL) {
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return;
    }
    chdir(dir);
    while((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name,&statbuf);
		if(!S_ISDIR(statbuf.st_mode)){
			printf("%d->%s\n",id,entry->d_name);
			enQueue(res,entry->d_name,0);
		}
    }
    chdir("..");
    closedir(dp);
}