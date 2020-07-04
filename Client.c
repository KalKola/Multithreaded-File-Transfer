#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <libgen.h>
#include <pwd.h>

int main(int argc, char *argv[])
{
	struct sockaddr_in server;
	char buff[500];
	char rec_message[100];
	char file_name[50];
	char file_path[50];
	char dest_path[50];
	char username[10];
	int test;
	int loop;
	int SID;
	

	// get user id and username for permissions
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	strcpy(username, pw->pw_name);

	int int_id = htonl(uid);
	
	//Create socket
	SID = socket(AF_INET, SOCK_STREAM, 0);

	if (SID == -1)
	{
		printf("Error creating socket");
	}
	{
		printf("socket created");
	}

	//Set sockaddr_in variables
	server.sin_port = htons(8081); //Port to connect on
	server.sin_addr.s_addr = inet_addr("127.0.0.2"); //Server IP
	server.sin_family = AF_INET; //IPV4 protocol

	//Connect to server
	if(connect(SID, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		printf("Server Connection Failed");
	 	return 1;
	}
	
	printf("\nServer Connection Established");

	// get file path from user
	printf("\nEnter Path to File: ");
	scanf("%s",  &file_path);

	// check if file exists
	FILE *fs = fopen(file_path, "r");
	if(fs == NULL) {
		printf("\nCould not open file\n");
		exit(1);		
	}
	printf("\nFile accepted\n");

	// send user ID and username
	if (send(SID, &int_id, 50, 0) < 0 ) {
		printf("Error sending user ID, exiting program\n");
		exit(0);
			
	}
	if (send(SID, username, 10, 0) < 0) {
		printf("Error sending usename, exiting program\n");
		exit(0);
	}

	// send file name to server
	printf("\nEnter Name of File: ");
	scanf("%s",  &file_name);
	if (send(SID, file_name, 50, 0) < 0 ) {
		printf("Error sending file name, exiting program\n");
		exit(0);	
	}

	// loop until valid directory specified by user
	while (loop == 0) {
		printf("\nEnter Destination Folder:");
		scanf("%s", &dest_path);
		if (strcmp(dest_path, "Offers") == 0 || 
		    strcmp(dest_path, "Sales") == 0 || 
		    strcmp(dest_path, "Marketing") == 0 || 
		    strcmp(dest_path, "Promotions") == 0 ){
			printf("Directory Accepted\n");	
			loop = 1;	
		}
		else {
			printf("Invalid directory, try again\n");		
		}
	}
	

	// send directory name to server
	if (send(SID, dest_path, 50, 0) < 0) {
		printf("Error sending destination folder, exiting program\n");	
	}
	
	// receive confirmation message back from server
	if (recv(SID, rec_message, 100, 0) < 0) {
		printf("Error receiving server message");	
	}
	printf("%s", rec_message);

	// check if server message confirms adequate permissions
	test = strcmp(rec_message, "File Transfer Successful\n");
	if (test != 0) {
		printf("Closing Connection\n");	
		exit(0);
	}
	else {
		printf("\n---Transfer Processing---");	
		bzero(buff, 500);
		int fs_block_sz;

		// transfer file to server
		while((fs_block_sz = fread(buff, sizeof(char), 500, fs))>0) {
			if(send(SID, buff, fs_block_sz, 0) < 0) {
				printf("\nError didn't send");
				break;			
			}
			bzero(buff, 500);
		}
		printf("\n---Transfer Complete---");
		
		// receive confirmation from server
		printf("\n\nServer Message: ");
		if (recv(SID, rec_message, 100, 0) < 0) {
			printf("Error receiving server message\n");		
		}
		printf(rec_message);
	
		printf("\nClosing Connection");
		close(SID);
		return 0;		
		
	}

}
