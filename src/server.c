#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

#define DEFAULT_DAYTIME_PORT 13

const char *MSG_ERROR = "Error execution command: ./server [-p port]\nOptional argument [-p port] where port is valid port number, where it will wait for request of the service.";
const int RECV_BUFF_LEN = 1024; // 1KB
const int DATE_BUFF_LEN = 30; // Maximum length of the date string

int main(int argc, char **argv) {

    // Check for input argument
    if (argc > 3 || argc == 2) {
        fprintf(stderr, "%s\n", MSG_ERROR);
        exit(EXIT_FAILURE);
    }

    // Check if there is the correct parameter
    if (argc == 3 && strcmp(argv[1], "-p")) {
        fprintf(stderr, "%s\n", MSG_ERROR);
        exit(EXIT_FAILURE);
    }

    // Get the port number
    uint16_t port;
    if (argc == 3) {
        errno = 0;
        char *end;
        long result = strtol(argv[2], &end, 10);
        if (errno == ERANGE || result < 0 || result > UINT16_MAX || *end != '\0') {
            fprintf(stderr, "%s\n", MSG_ERROR);
            exit(EXIT_FAILURE);
        }
        port = (uint16_t)result;
    } else {
        struct servent *p_data = getservbyname("daytime", NULL);
        if (p_data != NULL) {
            port = ntohs((uint16_t)p_data->s_port);  // Convert port to host byte order
        } else {
            fprintf(stderr, "Error: could not find DAYTIME service. Using default port 13\n");
            port = DEFAULT_DAYTIME_PORT;
        }
    }

    // Create the UDP socket
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("socket()");
		close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Set up the server address struct
    struct sockaddr_in host_addr;
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(port); // Convert to network byte order
    host_addr.sin_addr.s_addr = INADDR_ANY; // Bind to all available interfaces

    if (bind(server_socket, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0) {
        perror("bind()");
		close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Start receiving data
    printf("Daytime UDP server (127.0.0.1) listening on port %u\n\n", port);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    FILE *fd;
    char *date_buffer;
    char buffer_datagram[RECV_BUFF_LEN]; // Data inside will be not used

    // Listening loop
    while (1) {
        // Receive data from the client
        ssize_t recv_len = recvfrom(server_socket, buffer_datagram, sizeof(buffer_datagram) - 1, 0, (struct sockaddr *)&client_addr, &client_len);
        if (recv_len < 0) {
            perror("recvfrom()");
            continue;
        }

        // Convert the IP addr & port to readable format
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        uint16_t client_port = ntohs(client_addr.sin_port);

        // Show message about connection
        printf("> Received request from ==> %s:%u\n", client_ip, client_port);

        // Use system() call => date
        int sys_result = system("date > .date_tmp.txt");
        if (sys_result == -1) {
            perror("system()");
		    close(server_socket);
            exit(EXIT_FAILURE);
        }

        // Open the file and read the data
        fd = fopen(".date_tmp.txt", "r");
        if (fd == NULL) {
            perror("Error opening temporary file");
			close(server_socket);
            exit(EXIT_FAILURE);
        }

        // Allocate buffer for date string and read data
        date_buffer = (char *)malloc(DATE_BUFF_LEN);
        if (fgets(date_buffer, DATE_BUFF_LEN, fd) == NULL) {
            perror("Error reading from temporary date file");
            fclose(fd);
			close(server_socket);
            exit(EXIT_FAILURE);
        }

        // Close the file
        fclose(fd);

        // Send the date back to the client
        ssize_t sent_data_len = sendto(server_socket, date_buffer, strlen(date_buffer), 0, (struct sockaddr *)&client_addr, client_len);
        if (sent_data_len < 0) {
            perror("sendto()");
        } else {
            printf(">>> Sent current time to ==> %s:%u\n\n", client_ip, client_port);
        }

        // Free memory for date buffer
        free(date_buffer);
		fflush(stdout);
    }

	close(server_socket);
    exit(EXIT_SUCCESS);
}

