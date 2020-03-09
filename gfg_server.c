#include <unistd.h>
#include  <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT_NUM 9002

int main()
{
	int server_fd = socket(AF_INET , SOCK_STREAM , 0);// socket file descriptor
	if(server_fd  == 0)
	{
		perror("Error, Creation of the socket failed");
		exit(EXIT_FAILURE);
	}
	// server address 
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT_NUM);
	// specific address
	int inet_pton_status = inet_pton(AF_INET,"192.168.0.119", &address.sin_addr);
	if(inet_pton_status < 0)
	{
		perror("Error! Invalid IP address");
		exit(EXIT_FAILURE);
	}
	// bind the socket to the server address  
	int bind_stat = bind(server_fd , (struct sockaddr*) &address , sizeof(address));
	if(bind_stat < 0)
	{
		perror("Error! Binding failed");
		exit(EXIT_FAILURE);
	}
	puts("Listening for connection from 192.168.0.119:9002");
	// listen for connection at the socket
	int listen_stat = listen(server_fd , 3);// queue 3
	if(listen_stat < 0)
	{
		perror("Error! Failed to Listen");
		exit(EXIT_FAILURE);
	}
	// accept  connection to the socket 
	int address_len = sizeof(address);// for acccept
	int new_socket = accept(server_fd , (struct sockaddr*) &address , (socklen_t*) &address_len);
	if(new_socket < -1)
	{
		perror("Error! Accepting connection ");
		exit(EXIT_FAILURE);
	}
	char server_message[500] = "Hello from the server";
	while(1)
	{
		// send the server message
		send(new_socket, server_message, sizeof(server_message), 0);
		// read from the client
		char buffer[500] = {0};
		read(new_socket, buffer, sizeof(buffer));
		printf("%s\n", buffer);
		for(int i =0; i < 500; i++)
		{
			server_message[i] = buffer[i];
		}
	}
	return 0;
}
