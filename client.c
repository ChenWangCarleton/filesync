#include<netinet/in.h>  // sockaddr_in 
#include<sys/types.h>  // socket 
#include<sys/socket.h>  // socket 
#include<stdio.h>    // printf 
#include<stdlib.h>    // exit 
#include<string.h>    // bzero 
#include <unistd.h>


#define SERVER_PORT 8001 
#define BUFFER_SIZE 1024 
#define FILE_NAME_MAX_SIZE 512 
  
int main() 
{ 
  
	char fn[100];
	char serverip[20];
	char path[512];
	char buffer[BUFFER_SIZE]; 
	char choice;
	int flag=-1;
	int length = 0; 
	char filename[512];
	char* fail="/fail";
	char* success="/success";
	char* delete="/delete";
	bzero(buffer, BUFFER_SIZE); 
	bzero(fn,100);
	bzero(path,512);
	bzero(serverip,20);
	bzero(filename,512);
	printf("please input the directory path for saving files\nto save files to the directory where this program is, type:./\n");
	scanf("%512[^\n]",buffer);
	while(flag<0){
		if (0 != access(buffer, F_OK)) {
			perror("errors occurs when finding the directory\n");
			while(getchar()!='\n');
			printf("please input the directory path for saving files\nto save files to the directory where this program is, type:./\n");
			scanf("%512[^\n]",buffer);
		}
		else{
			while(getchar()!='\n');
			printf("directory:\n%s\nexists\n",buffer);
			realpath(buffer, path);
			path[strlen(path)]='/';
			bzero(buffer,BUFFER_SIZE);
			printf("files from server will be saved to\n%s\n",path);
			flag=1;
		}
	}
	printf("This program will receive files from the server and save to directory\n%s\nIf a file in that directory have the same name as a file from the server, You will lose the orginal file\npress y or Y to continue, input other characters to quit\n",path);
	scanf("%c",&choice);
	if(choice!='y'&&choice!='Y'){
		printf("program exits\n");
		return 0;
	}
	struct sockaddr_in client_addr; 
	bzero(&client_addr, sizeof(client_addr)); 
	client_addr.sin_family = AF_INET; 
	printf("please input the server ip, like:192.168.56.101\n");
	scanf("%s",serverip);
	//strcpy(serverip,"192.168.56.101");
	while(getchar()!='\n');
	client_addr.sin_addr.s_addr =  inet_addr(serverip); 
	client_addr.sin_port = htons(8001); 

	int client_socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
	if(client_socket_fd < 0) 
	{ 
		perror("Create Socket Failed:"); 
		exit(1); 
	} 

  
  // declare a server address, use server ip to initialize it for later connection
	struct sockaddr_in server_addr; 
	bzero(&server_addr, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET; 
	if(inet_pton(AF_INET, serverip, &server_addr.sin_addr) == 0) 
	{ 
		perror("Server IP Address Error:"); 
		exit(1); 
	} 
	server_addr.sin_port = htons(SERVER_PORT); 
	socklen_t server_addr_length = sizeof(server_addr); 
  
  // connect with server.
	if(connect(client_socket_fd, (struct sockaddr*)&server_addr, server_addr_length) < 0) 
	{ 
		perror("Can Not Connect To Server IP:"); 
		exit(0); 
	} 
	while(1){ 
		bzero(buffer,BUFFER_SIZE);
		length = recv(client_socket_fd, buffer, BUFFER_SIZE, 0);
		printf("%s\n",buffer);
		if(strcmp(buffer,delete)==0){
			bzero(buffer,BUFFER_SIZE);
			length = recv(client_socket_fd, buffer, BUFFER_SIZE, 0);
			bzero(fn,100);
			strcpy(fn,buffer);
			bzero(filename,512);
			strcat(filename,path);
			strcat(filename,fn);
			if (remove(filename) == 0){
				printf("File: %s is successfully deleted.\n",filename);
			}
			else{
				printf("File: %s is successfully deleted.\n",filename);				
			}
		}
		else if(strcmp(buffer,fail)==0){
			printf("file not found on server side:%s\n",buffer);
		}
		else{
  // prepare to write to file
			bzero(fn,100);
			strcpy(fn,buffer);
			bzero(filename,512);
			strcat(filename,path);
			strcat(filename,fn);
			FILE *fp = fopen(filename, "w"); 
			if(NULL == fp){ 
				printf("Client is terminating.\nIf you didn't close this, it might because the server is closed or there is an unknow error occurs\n"); 
				exit(1); 
			} 
  // constantly receive file content from server until receive an end signal 
			while(1){
 // while((length = recv(client_socket_fd, buffer, BUFFER_SIZE, 0)) > 0) 
				length = recv(client_socket_fd, buffer, BUFFER_SIZE, 0);
				if(length==0)break;
				if(length==4&&strcmp(buffer,":EOF")==0)break;
				if(fwrite(buffer, sizeof(char), length, fp) < length){ 
					printf("File:\t%s Write Failed\n", filename); 
					break; 
				} 
			//	printf("%s\n",buffer);
//send(client_socket_fd, &length, sizeof(char), 0);
				printf("%d characters received from %d\n",strlen(buffer)*sizeof(char),length);
				printf("\n");
				bzero(buffer, BUFFER_SIZE); 
			} 
			printf("%d+%s\n",length,buffer);  
  //open file pointer.
			printf("Receive File:\t%s From Server: %s Successful!\n", filename, serverip); 
			fclose(fp); 
		
		}
	}
	close(client_socket_fd); 
	return 0; 
} 