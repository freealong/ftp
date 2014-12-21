/**********************************************************
** File Name:client.c
*  Author:Wangmeng
*  Data:2010.06.01
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

#define MAX_INPUT_SIZE 254
#define LOCAL_IP "127.0.0.1"
#define SERVER_PORT 2048
#define DATA_PORT 2049
#define FILEBUF_SIZE 1024
#define TYPE_I 0
#define TYPE_A 1

char buffer[FILEBUF_SIZE];
char line_in[MAX_INPUT_SIZE+1];
char *server_ip, *local_ip;
char port_addr[24] = "";
int server_port, local_port;
int server_sock;
int z = 0;
struct sockaddr_in server_addr;
struct hostent *server_host;

//send msg to server
void send_msg(char *command, char *msg, int flag);

//some functions
void user_login();
void command_quit();
void command_put(char *filename);



int main(int argv, char **argc)
{
    if(argv == 1)
    {
	server_ip = LOCAL_IP;
	server_port = SERVER_PORT;
    }
    else if(argv == 2)
    {
	server_ip = argc[1];
	server_port = SERVER_PORT;
    }
    else if(argv == 3)
    {
	server_ip = argc[1];
	server_port = atoi(argc[2]);
    }
    //connect to the ftp server
    server_host = gethostbyname(server_ip);
    if(server_host == (struct hostent *)NULL)
    {
	printf(">gethostbyname failed\n");
	exit(1);
    }
    //setup the port for the connection
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr, server_host->h_addr, server_host->h_length);
    server_addr.sin_port = htons(server_port);
    //get the socket
    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(server_sock < 0)
    {
	printf(">error on socket()\n");
	exit(1);
    }
    //connect to the server
    if(connect(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
	printf(">error on connect()\n");
	close(server_sock);
	exit(1);
    }
    
    //connect to the ftp server successful
    printf(">Connected to %s:%d\n", server_ip, server_port);
    z = read(server_sock, buffer, sizeof(buffer));
    buffer[z-2] = 0;
    printf("%s\n", buffer);
    //printf("z = %d, buffer is '%s'\n",z,buffer);
    //login
    user_login();
    while(1)
    {
	printf(">");
	fgets(line_in, MAX_INPUT_SIZE, stdin);
	line_in[strlen(line_in)-1] = '\0';
	if(strncmp("quit", line_in, 4) == 0)
	{
	    command_quit();
	    break;
	}
	else if(strncmp("put", line_in, 3) == 0)
	{
	    command_put(&line_in[4]);
	}
	else if(strncmp("test", line_in, 4) == 0)
	{
		char test_command[100];
		strcpy(test_command,&line_in[5]);
		write(server_sock, test_command, strlen(test_command)+1);
	}
    }
    close(server_sock);
    return 0;
}


//send msg to server
void send_msg(char *command, char *msg, int flag)
{
    char reply[MAX_INPUT_SIZE+1];
    if(flag == 0)
	sprintf(reply, "%s\n", command);
    else
	sprintf(reply, "%s %s\n", command, msg);
    write(server_sock, reply, strlen(reply));
    printf("%d-->%s",strlen(reply), reply);
    return;
}

void user_login()
{
    printf(">Name:");
    fgets(line_in, MAX_INPUT_SIZE, stdin);
    line_in[strlen(line_in)-1] = '\0';
    send_msg("USER", line_in, 1);
    
    z = read(server_sock, buffer, sizeof(buffer));
    buffer[z-2] = 0;
    printf("%s\n", buffer);
    //printf("z = %d, buffer is '%s'\n",z,buffer);
    if(strncmp("331", buffer, 3) == 0)
    {
	printf(">Password:");
	fgets(line_in, MAX_INPUT_SIZE, stdin);
	line_in[strlen(line_in)-1] = '\0';
	send_msg("PASS", line_in, 1);
	
	z = read(server_sock, buffer, sizeof(buffer));
	buffer[z-2] = 0;
	printf("%s\n", buffer);
	//printf("z = %d, buffer is '%s'\n",z,buffer);
    }
    line_in[0] = 0;
    return;
}

void command_quit()
{
    send_msg("QUIT", "", 0);
    z = read(server_sock, buffer, sizeof(buffer));
    buffer[z-2] = 0;
    printf("%s\n", buffer);
}

void command_put(char *filename)
{
    FILE *infile;
    unsigned char databuf[FILEBUF_SIZE] = "";
    int bytes, bytessend=0;
    
    infile = fopen(filename,"r");
    if(infile == 0)
    {
	perror("fopen() failed");
	//close(data_sock);
	return;
    }
    
    send_msg("STOR", filename, 1);

    while((bytes = read(fileno(infile), databuf, FILEBUF_SIZE)) > 0)
    {
	write(server_sock, (const char *)databuf, bytes);
	bytessend += bytes;
	memset(&databuf, 0, FILEBUF_SIZE);
    }
    printf("read finished\n");
    memset(&databuf, 0, FILEBUF_SIZE);
    
    fclose(infile);
       
    z = read(server_sock, buffer, sizeof(buffer));
    buffer[z-2] = 0;
    printf("%s\n", buffer);
    
    printf("%d bytes send\n", bytessend);
    return;
}












