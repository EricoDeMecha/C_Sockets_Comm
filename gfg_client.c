#include <unistd.h> //access to POSIX API
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h> // for inet_pton()

#define PORT_NUM 9002

int main()
{
    // create socket
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(client_sock < 0)
    {
        perror("Error creating the socket!");
        exit(EXIT_FAILURE);
    }
    // socket address
    struct sockaddr_in sock_address;
    sock_address.sin_family = AF_INET;
    sock_address.sin_port = htons(PORT_NUM);// connect to port 8888 on localhost
    // convert IPV4 and IPV6 from text to binary form
    // used instead fo setting server address to any address
    int inet_pton_status =  inet_pton(AF_INET , "192.168.0.119", &sock_address.sin_addr);
    if(inet_pton_status <  0)
    {
        perror("Invalid Address or Address not Supported");
        exit(EXIT_FAILURE);
    }
    int conn_status = connect(client_sock , (struct sockaddr*) &sock_address, sizeof(sock_address));
    if(conn_status < -1)
    {
        perror("Error Connetion failed");
        exit(EXIT_FAILURE);
    }
    
    // at this point, connection to the server has been established
    while(1)
    {
       // read from the server
        char buffer[1024] = {0}; 
        read(client_sock, buffer , sizeof(buffer));
        printf("%s\n", buffer);
        // send to the server
        char client_message[4096];
        printf("\n Enter Message: ");
        fgets(client_message, 4096 , stdin);
    
        send(client_sock , client_message , strlen(client_message) , 0);
    }   
    return 0;
}