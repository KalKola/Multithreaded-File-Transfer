#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <grp.h>
#include <pthread.h>
#include <ctype.h>

// mutex lock
pthread_mutex_t lock;

void * handle_connection (void* cs);

int main(int argc, char *argv[])
{
	int s; 
	int cs;
	int connSize; 
	int READSIZE; 

	// initialize the lock
	pthread_mutex_init(&lock, NULL);
	
	struct sockaddr_in server, client;

	// create server socket
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1)
	{
		puts("Error Creating Socket\n");
	}
	else
	{
		puts("Socket Created\n");
	}

	// set sockaddr_in variables
	server.sin_port = htons(8081); 
	server.sin_family = AF_INET; 
	server.sin_addr.s_addr = INADDR_ANY;

	// bind socket
	if(bind(s,(struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Bind Issue\n");
		return 1;
	}
	else
	{
		puts("Socket Bound Successfully\n");
	}

	// listen for client connection
	listen(s,3);

	while (1) {
		
		puts("Listening for Client Connection\n");
		connSize = sizeof(struct sockaddr_in);

		// accept connection from client
		cs = accept(s, (struct sockaddr *)&client, (socklen_t*)&connSize);
		if(cs < 0)
		{
			perror("Connection Could not be Established\n");
			return 1;
		}
		else
		{
			puts("Client Connected Successfully\n");
			pthread_t t;
			int *pclient = malloc(sizeof(int));
			*pclient = cs;
			
			// create thread for client handling
			pthread_create(&t, NULL, handle_connection, pclient);
		}
	
	}

	return 0;

}// main()
	
// function for handling client threads
void * handle_connection (void* p_cs) {

	// read in client thread pointer and free after
	int cs = *((int*)p_cs);
	free(p_cs);

	char username[10];
	char file_name[50];
	char dest[50];
	char buff[500];
	char rec_message[100];
	char directory[10];
	uid_t uid;
	int i;
	int permission = 0;

	// receive user ID from Client
	if (recv(cs, &uid, 50, 0) < 0) {
		printf("Error Receiving User ID\n");
		return NULL;
	}
	int user_id = ntohl(uid);

	// receive username from Client
	if (recv(cs, username, 10, 0) < 0) {
		printf("Error Receiving Username\n");	
		return NULL;
	}

	// getting user groups
	gid_t supp_groups[] = {};

	int j, ngroups;
	gid_t *groups;
	struct passwd *pw;
	struct group *gr;
	
	ngroups = 5;
	groups = malloc(ngroups * sizeof (gid_t));

	// getting the groups the user is a member of
	if (getgrouplist(username, user_id, groups, &ngroups) == -1) {
		printf("Error, could not gather user groups\n");	
	}
	
	for (j = 0; j < ngroups; j++) {
		supp_groups[j] = groups[j];	
	}

	// receiving file name
	if (recv(cs, file_name, 50, 0) < 0) {
		printf("Could not receive file name\n");
		return NULL;	
	}
	printf("Received File Name from %s\n", username);


	// receiving destination folder
	if (recv(cs, dest, 50, 0) < 0) {
		printf("Could not receive destination folder\n");
		return NULL;	
	}
	printf("Received Destination Folder from %s\n", username);

	// If statement for checking permissions. Since validation is also done at the 
	// client end to ensure it's a valid directory, simply checking the first letter
	// can distinguish which directory is specified.
	int comp_perm;
	if (dest[0] == 'S') {
		comp_perm = 1009;	
	}
	else if (dest[0] == 'O') {
		comp_perm = 1010;	
	}
	else if (dest[0] == 'P') {
		comp_perm = 1011;	
	}
	else if (dest[0] == 'M') {
		comp_perm = 1012;	
	}
	else {
		printf("Invalid Directory\n");		
	}
	
	// check for permissions
	for(i = 0; i < j + 1; i++) {
		if (supp_groups[i] == comp_perm) {
			permission = 1;		
		}	
	}

	if (permission != 1) {
		strcpy(rec_message, "User does not have permission for directory\n");
		write(cs, rec_message, 100);		
	}
	else {
		// send confirmation message to client
		strcpy(rec_message, "File Transfer Successful\n");
		write(cs, rec_message, 100);

		// creating path variable
		strcpy(directory, dest);
		directory[0] = tolower(directory[0]);
		strcat(dest, "/");
		strcat(dest, file_name);
		char* fr_name = dest;
		
		// creating file in directory
		FILE *fp = fopen(fr_name, "ab+");
		if(fp == NULL) {
			printf("File cannot be opened\n");		
		}
		else {		
			bzero(buff, 500);
			int fr_block_sz = 0;

			// transfering buffer contents to new file
			while(fr_block_sz = recv(cs, buff, 500, 0)) {
				if(fr_block_sz < 0) {
					printf("error receieving file\n"); 				
				}
				// lock file during write
				pthread_mutex_lock(&lock);
				int write_sz = fwrite(buff, sizeof(char), fr_block_sz, fp);
				// unlock file after write				
				pthread_mutex_unlock(&lock);
				fclose(fp);

				printf("Finished\n");
				strcpy(rec_message, "File Transfer Successful\n");
				write(cs, rec_message, 100);	

				// do stuff
				char comm_string[100];

				// change owner of file
				sprintf(comm_string, "sudo chown %d %s", user_id, fr_name);
				system(comm_string); 

				// change group ownership
				sprintf(comm_string, "sudo chgrp %s %s", directory, fr_name);
				system(comm_string);


			}		
					
			return NULL;		
		}

	}
}

