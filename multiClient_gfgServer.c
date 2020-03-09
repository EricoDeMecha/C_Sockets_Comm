#include <stdio.h>
#include <stdlib.h>
#include <string.h>// strlen
#include <errno.h>
#include <unistd.h> // close

#include <arpa/inet.h> // close 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> // FD_SET , FD_ISSET , FD_ZERO

int main()
{
    // create master socket
    int master_socket = socket(AF_INET , SOCK_STREAM , 0);
    if(master_socket == 0)
    {
        perror("Error! Socket creation failed!");
        exit(EXIT_FAILURE);
    }
    // socket address
    struct sockaddr_in sock_address;
    sock_address.sin_family = AF_INET;
    sock_address.sin_port = htons(8888);
    sock_address.sin_addr.s_addr = INADDR_ANY;
    // Good  Habit
    // set the socket to allow multiple connections
    int opt = 1;
    int setsockopt_stat = setsockopt(master_socket , SOL_SOCKET ,SO_REUSEADDR | SO_REUSEPORT, &opt ,sizeof(opt));
    if(setsockopt_stat < 0)
    {
        perror("Error! Erro enabling multiple connections");
        exit(EXIT_FAILURE);
    }
    // bind the socket to the ip address and port specified
    int bind_stat = bind(master_socket , (struct sockaddr*) &sock_address , sizeof(sock_address));
    if(bind_stat < 0)
    {
        perror("Error! Binding Ip and Port to Sock");
        exit(EXIT_FAILURE);
    }
    //listen to sock
    int listen_stat = listen(master_socket , 3);
    if(listen_stat < 0) { perror("Error! Listening failed!"); exit(EXIT_FAILURE); }
    puts("Listening on port 8888");
    // Bringing in select to take care of the multiple clients
    fd_set readfds;
    // clients
    int max_cls = 30;
    int client_sockets[30]; // 30 arbitrary clients
    memset(client_sockets , 0 , 30*sizeof(int)); // initialize all with zeros
    int max_sd, activity , new_socket , valread , sd;
    int addr_len = sizeof(sock_address);
    char *welcome_message = "Hello you have reached the server \n";
    char buffer[1024];
    puts("Waiting for connections...");
    while(1)
    {
        // clear the socket
        FD_ZERO(&readfds);
        // add master socket to the set
        FD_SET(master_socket , &readfds);
        max_sd = master_socket;
        // add child sockets to the set
        for(int i = 0; i < max_cls; i++)
        {
            // socket descriptor
            sd = client_sockets[i];
            // if valid socket descriptor then add to the read list
            if(sd > 0)
            {
                FD_SET(sd , &readfds);
            }
            // highest file descriptor number, need it for the select function
            if(sd > max_sd)
            {
                max_sd = sd;
            }
        }
        // wait for an activity on one of the sockets, TIMEOUT is NULL so wait  indefinately
        activity = select((max_sd+1), &readfds, NULL , NULL , NULL);
        if(( activity < 0) && (errno != EINTR ))
        {
            perror("Error! Selecting");
        }
        // if something on  the master socket then it has an incoming connection
        if(FD_ISSET(master_socket , &readfds))
        {
            new_socket = accept(master_socket , (struct sockaddr*) &sock_address , (socklen_t*) &(addr_len));
            if(new_socket < 0)
            {
                perror("Error! Accepting connection failed \n");
                exit(EXIT_FAILURE);
            }
            // inform user of the socket number - user in send and receive commands
            printf("New Connection: Socket fd is: %d , ip is: %s: port: %d \n", new_socket , inet_ntoa(sock_address.sin_addr), ntohs(sock_address.sin_port));
            // send new connection greeting
            ssize_t send_stat = send(new_socket ,welcome_message , strlen(welcome_message), 0); 
            if(send_stat < 0)
            {
                perror("Error! Sending the message \n");
            }
            puts("Welcome message sent successfully \n");
            // add new socket to array of sockets
            for(int i = 0; i < max_cls; i++)
            {
                if(client_sockets[i] == 0)
                {
                    client_sockets[i] = new_socket;
                    printf("Added to the list of sockets as %d \n", i);
                    break;
                }
            }
        }
        
        
        //else its some IO operation  on some other socket
        for(int i = 0; i < max_cls; i++)
        {
            sd = client_sockets[i];
            if(FD_ISSET(sd, &readfds))
            {
                // check if it was for closing, and also read for incoming message
                if((valread = read(sd ,buffer , 1024)) == 0)
                {
                    // somebody disconnected , get his details and print the out
                    getpeername(sd, (struct sockaddr*) &sock_address , (socklen_t*) &addr_len);
                    printf("Host disconnected, ip: %s , port: %d \n", inet_ntoa(sock_address.sin_addr), ntohs(sock_address.sin_port));
                    // close the socket and reset to zero
                    close(sd);
                    client_sockets[i] = 0;
                }
                // echo back the message that came in
                else
                {
                    // set the string terminating NULL byte on the data read
                    buffer[valread] = '\0';
                    send(sd ,buffer, strlen(buffer), 0);
                }
            }
        }
    }
    return 0;
} 