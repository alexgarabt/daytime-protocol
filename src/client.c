#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define DEFAULT_DAYTIME_PORT 13
#define RECV_BUFF_LEN 1024 // 1KB

const char *MSG_ERROR =  "Error execution command: ./client ip_addr [-p port]\nOptional argument [-p port] where port is valid port number\nWill send a request to the ip_addr:port DAYTIME service and print the response";

const char *connect_msg = "May you tell me the time, Sir?";

int main(int argc, char **argv) {
    // Check for input argument
    if (argc < 2 || argc > 4 || argc == 3) {
        fprintf(stderr, "%s\n", MSG_ERROR);
        exit(EXIT_FAILURE);
    }

    // Check if there is the correct parameter for port
    if (argc == 4 && strcmp(argv[2], "-p")) {
        fprintf(stderr, "%s\n", MSG_ERROR);
        exit(EXIT_FAILURE);
    }

    // Get the port number from the parameter if not default
    uint16_t port;
    if (argc == 4) {
        errno = 0;
        char *end;
        long result = strtol(argv[3], &end, 10);

        if (errno == ERANGE || result < 0 || result > UINT16_MAX || *end != '\0') {
            fprintf(stderr, "%s\n", MSG_ERROR);
            exit(EXIT_FAILURE);
        }

        port = (uint16_t)result;
    } else {
        // Set the predefined port for DAYTIME
        struct servent *p_data = getservbyname("daytime", NULL);

        if (p_data != NULL) {
            port = ntohs((uint16_t)p_data->s_port);  // Convert port to host byte order
        } else {
            fprintf(stderr, "Error: could not find DAYTIME service. Using default port %d\n", DEFAULT_DAYTIME_PORT);
            port = DEFAULT_DAYTIME_PORT;
        }
    }

    // Check the IP address is correct [0.0.0.0, 255.255.255.255]
    struct in_addr server_in_addr;
    if (inet_aton(argv[1], &server_in_addr) == 0) {
        fprintf(stderr,"Error bad address: %s\n", argv[1]);
        fprintf(stderr, "%s\n", MSG_ERROR);
        exit(EXIT_FAILURE);
    }

    // Open a socket
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("socket()");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Set up the client address struct
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = 0; // OS will automatically choose a random port
    client_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the client socket to an available port (optional but useful for local network setups)
    if (bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
        perror("bind()");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Set up the server address struct for sending data
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port); // Set port number
    server_addr.sin_addr = server_in_addr; // Set the server IP address

    socklen_t server_addr_len = sizeof(server_addr);

    // Send a message to the server
    if (sendto(client_socket, connect_msg, strlen(connect_msg), 0, (struct sockaddr*)&server_addr, server_addr_len) < 0) {
        perror("sendto()");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Wait for the server's response
    char buffer_response[RECV_BUFF_LEN];
    ssize_t recv_len = recvfrom(client_socket, buffer_response, RECV_BUFF_LEN, 0, (struct sockaddr*)&server_addr, &server_addr_len);
    if (recv_len < 0) {
        perror("recvfrom()");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Null terminate the response for printing
    buffer_response[recv_len] = '\0'; 

    // Print the server response
    printf("Received from server: %s\n", buffer_response);

    // Close the socket
    close(client_socket);

    exit(EXIT_SUCCESS);
}
