/**********************************************************
** File Name:server.c
*  Author:YONGQI
*  Data:2014.11.15
**********************************************************/
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<limits.h>
#include<string.h>
#include<ctype.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netdb.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<dirent.h>
#include <pthread.h>

#define MAX_INPUT_SIZE 254
#define SERVER_PORT 2048
#define FILEBUF_SIZE 1024
#define ACCOUNTFILE "account"
#define ACCOUNT_SIZE 27
#define NAME_SIZE 11
#define PASSWORD_SIZE 11

void init_server(void);
int search_account(char *name, char *account);
int account_confirm(char *name, char *password);
int change_password(char *name, char *new_password);
int change_status(char *name, int i);
void store_file(char *name, char *filename, char *reply, int client_sock);



int server_sock;
struct sockaddr_in server_addr;

pthread_t thread[2];

void *thread1()
{
  while(1)
  {
	int z;
	int client_sock;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len;
	char buffer[FILEBUF_SIZE];
	char name[NAME_SIZE], password[PASSWORD_SIZE];
	int log_status=0;
	client_addr_len = sizeof(client_addr);
	printf("wait for accept...\n");
	//accept an request
	client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
	if(client_sock < 0)
	{
	    //fprintf(stderr,"%s,accept(sock, (struct sockaddr *)&client_addr, &client_len)\n",strerror(errno));
	    //close(sock);
	    //return 5;
	    printf("Server Accept Failed!\n");
	    
	}
	else printf("Server Accept Succeed!New socket %d\n",client_sock);

	char reply[100] = "220 FTP server ready\r\n";
	write(client_sock, reply, strlen(reply));
	printf("--->%s",reply);
	while(1){
	    
	    //deal with commands
	    z = read(client_sock, buffer, sizeof(buffer));
	    buffer[z-1] = 0;
	    printf("z = %d, buffer is '%s'\n",z,buffer);
	    char command[5];
	    strncpy(command, buffer, 3);
	    command[3] = 0;
	    
	    //char username[25] = "anonymous";
	    if(strcmp(command, "USE") == 0)//USER
	    {
		stpcpy(name, &buffer[5]);
		printf("name is %s", name);
		stpcpy(reply, "331 Password required\r\n");
		write(client_sock, reply, strlen(reply));
		printf("%d -->%s", strlen(reply), reply);
	    }
	    else if(strcmp(command, "PAS") == 0)//PASS
	    {
		stpcpy(password, &buffer[5]);
		printf("password is %s", password);
		if(0==account_confirm(name, password))
		{
		    stpcpy(reply, "230 User logged in\r\n");
		    log_status = 1;
		}	
		else
		{
		    stpcpy(reply, "530 User logg failed\r\n");
		    log_status = 0;
		}
		write(client_sock, reply, strlen(reply));
		printf("%d -->%s", strlen(reply), reply);
	    }
	    else if(strcmp(command, "STO") == 0)//STOR
	    {
		if(log_status==1)
		{
		    //breakpoint=check_break(name, &buffer[5]);
		    store_file(name, &buffer[5], reply, client_sock);
		    change_status(name, 1);
		}
		else
		{
		    strcpy(reply, "Please log in\r\n");
		}
		write(client_sock, reply, strlen(reply));
		printf("%d -->%s", strlen(reply), reply);
	    }
	    else if(strcmp(command, "CHA") == 0)//CHANGE PASSWORD
	    {
		if(log_status==1)
		{
		    if(-1==change_password(name, &buffer[5]))
			strcpy(reply, "Change password failed\r\n");
		    else
			strcpy(reply, "Change password succeed\r\n");
		}
		else
		    strcpy(reply, "Please log in\r\n");
		
		write(client_sock, reply, strlen(reply));
		printf("%d -->%s", strlen(reply), reply);
	    }
	    else if(strcmp(command, "QUI") == 0)//QUIT
	    {
		stpcpy(reply, "221 Goodbye.\r\n");
		write(client_sock, reply, strlen(reply));
		printf("%d -->%s", strlen(reply), reply);
		break;
	    }
	}
	
	close(client_sock);
	printf("\n");
    }
}

void bail(const char *on_what)
{
    perror(on_what);
    exit(1);
}


int main(int argc, char **argv)
{
  

    init_server();
    
    memset(&thread, 0, sizeof(thread));
    pthread_create(&thread[0], NULL, thread1, NULL);
    
    //loop and wait for connection
    while(1)
    {
	int z;
	int client_sock;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len;
	char buffer[FILEBUF_SIZE];
	char name[NAME_SIZE], password[PASSWORD_SIZE];
	int log_status=0;
	client_addr_len = sizeof(client_addr);
	printf("wait for accept...\n");
	//accept an request
	client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
	if(client_sock < 0)
	{
	    //fprintf(stderr,"%s,accept(sock, (struct sockaddr *)&client_addr, &client_len)\n",strerror(errno));
	    //close(sock);
	    //return 5;
	    printf("Server Accept Failed!\n");
	    return 1;
	}
	else printf("Server Accept Succeed!New socket %d\n",client_sock);

	char reply[100] = "220 FTP server ready\r\n";
	write(client_sock, reply, strlen(reply));
	printf("--->%s",reply);
	while(1){
	    
	    //deal with commands
	    z = read(client_sock, buffer, sizeof(buffer));
	    buffer[z-1] = 0;
	    printf("z = %d, buffer is '%s'\n",z,buffer);
	    char command[5];
	    strncpy(command, buffer, 3);
	    command[3] = 0;
	    
	    //char username[25] = "anonymous";
	    if(strcmp(command, "USE") == 0)//USER
	    {
		stpcpy(name, &buffer[5]);
		printf("name is %s", name);
		stpcpy(reply, "331 Password required\r\n");
		write(client_sock, reply, strlen(reply));
		printf("%d -->%s", strlen(reply), reply);
	    }
	    else if(strcmp(command, "PAS") == 0)//PASS
	    {
		stpcpy(password, &buffer[5]);
		printf("password is %s", password);
		if(0==account_confirm(name, password))
		{
		    stpcpy(reply, "230 User logged in\r\n");
		    log_status = 1;
		}	
		else
		{
		    stpcpy(reply, "530 User logg failed\r\n");
		    log_status = 0;
		}
		write(client_sock, reply, strlen(reply));
		printf("%d -->%s", strlen(reply), reply);
	    }
	    else if(strcmp(command, "STO") == 0)//STOR
	    {
		if(log_status==1)
		{
		    //breakpoint=check_break(name, &buffer[5]);
		    store_file(name, &buffer[5], reply, client_sock);
		    change_status(name, 1);
		}
		else
		{
		    strcpy(reply, "Please log in\r\n");
		}
		write(client_sock, reply, strlen(reply));
		printf("%d -->%s", strlen(reply), reply);
	    }
	    else if(strcmp(command, "CHA") == 0)//CHANGE PASSWORD
	    {
		if(log_status==1)
		{
		    if(-1==change_password(name, &buffer[5]))
			strcpy(reply, "Change password failed\r\n");
		    else
			strcpy(reply, "Change password succeed\r\n");
		}
		else
		    strcpy(reply, "Please log in\r\n");
		
		write(client_sock, reply, strlen(reply));
		printf("%d -->%s", strlen(reply), reply);
	    }
	    else if(strcmp(command, "QUI") == 0)//QUIT
	    {
		stpcpy(reply, "221 Goodbye.\r\n");
		write(client_sock, reply, strlen(reply));
		printf("%d -->%s", strlen(reply), reply);
		break;
	    }
	}
	
	close(client_sock);
	printf("\n");
    }
    close(server_sock);
    return 0;
}

void init_server(void)
{
    int z;
    printf("\nWelcome to yongqi's server!\n");
    //get the server socket
    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(server_sock == -1)	//create socket failed
    {
	bail("socket() failed");
    }
    else printf("Socket created!\n");
    
    //configure server address,port
    memset(&server_addr, 0 ,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//    server_addr.sin_addr.s_addr = inet_addr("192.168.0.100");
    if(server_addr.sin_addr.s_addr == INADDR_NONE)
    {
	bail("INADDR_NONE");
    }
    
    //bind
    z = bind(server_sock, (struct sockaddr *)&server_addr, sizeof server_addr);
    if(z == -1)
    {
	bail("bind()");
    }
    else printf("Bind Ok!\n");
    
    //listen
    z = listen(server_sock, 5);
    if(z < 0)
    {
	printf("Server Listen Failed!");
	exit(1);
    }
    else printf("listening\n");
}
  
int search_account(char *name, char *account)
{
    FILE *file;
    int i=0;
    char temp[ACCOUNT_SIZE];
    file = fopen(ACCOUNTFILE, "r");
    while(fgets(account,ACCOUNT_SIZE,file)>0)
    {
	i++;
	strcpy(temp,account);
	temp[NAME_SIZE-1]=0;
	if(strcmp(temp,name)==0)
		return i;
    }
    return -1;
}

int account_confirm(char *name, char *password)
{
    char temp[ACCOUNT_SIZE];
    
    if(-1==search_account(name,temp))
	return -1;
	
    strncpy(temp,&temp[11],PASSWORD_SIZE);
    temp[PASSWORD_SIZE-1]=0;
    if(strcmp(password, temp) == 0)
    {
	  //printf("sucess\n");
	return 0;
    }
    
    return -1;
}

int change_password(char *name, char *new_password)
{
    char temp[ACCOUNT_SIZE];
    char cmd[100]="sed -i 's/1107820132.*/1107820132 1107820132 0 0/' account";
    if(-1==search_account(name,temp))
	return -1;
    strncpy(&temp[NAME_SIZE],new_password,PASSWORD_SIZE-1);
    strncpy(&cmd[10],name,NAME_SIZE-1);
    strncpy(&cmd[23],temp,25);
    system(cmd);
    return 0;
}
int change_status(char *name, int i)
{
    char temp[ACCOUNT_SIZE];
    char cmd[100]="sed -i 's/1107820132.*/1107820132 1107820132 0 0/' account";
    if(-1==search_account(name,temp))
	return -1;
    if(i==1)
      temp[22]='1';
    else if(i=0)
      temp[22]='0';
    strncpy(&cmd[10],name,NAME_SIZE-1);
    strncpy(&cmd[23],temp,25);
    system(cmd);
    return 0;
}
/*
int check_break(char *name, char *filename)
{
    FILE *infile;
    char temp[256];
    char cmd[100]="grep 1107820132 tmp |grep ftp.doc";
    int i=0, filelen=0;
    
    strncpy(&cmd[5],name,NAME_SIZE-1);
    filelen=strlen(filename);
    strncpy(&cmd[26],filename,filelen);
    cmd[26+filelen]=0;
    infile = popen(cmd, "r");
    if(fgets(temp,20,infile)>0)
    {
	strncpy(temp, &temp[11], 4);
	temp[4]=0;
	i=atoi(temp);
	pclose(infile);
	//printf("%d",i);
	return i;
    }
    
    return 0;
}*/

void store_file(char *name, char *filename, char *reply, int client_sock)
{
    FILE *outfile;
    unsigned char databuf[FILEBUF_SIZE];
    char filename_new[100];
    struct stat sbuf;
    int bytes = 0, filesize=0;
    long offset=0;
    
    strcpy(filename_new, name);
    strcat(filename_new, "_");
    strcat(filename_new, filename);
    if(stat(filename_new, &sbuf) == -1)
    {
	//stpcpy(reply, "450 Cannot create the file\r\n");
	//return;
	printf("File not exist,try to create a new one.\n");
	outfile = fopen(filename_new, "w");
    }
    else
    {
      outfile = fopen(filename_new, "rb+");
      fseek(outfile, 0L, 2);
      filesize = ftell(outfile);
      sprintf(reply, "213 filesize is %d\n", filesize);
      //printf(reply);
      write(client_sock, reply, strlen(reply));
    }
    
    if(outfile == 0)
    {
	bail("fopen() failed");
	stpcpy(reply, "450 Cannot create the file\r\n");
	return;
    }

    
    while((bytes = read(client_sock, databuf, FILEBUF_SIZE)) > 0)
    {
	fwrite(databuf, 1, bytes, outfile);
	if(bytes<FILEBUF_SIZE) break;
    }
    printf("read finished\n");
    fclose(outfile);
    stpcpy(reply, "226 Transfer complete\r\n");
    return;
}
