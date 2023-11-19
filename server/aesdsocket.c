#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <syslog.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>

#define buffer_size 1024
void cleanup(int exit_code);
void sig_handler(int signo);

int sockfd, client_sockfd, datafd;

int main(int argc, char *argv[]) {

    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
		pid_t pid, sid;

		// Fork the process
		pid = fork();

		// Error with fork
		if (pid < 0) {
			syslog(LOG_ERR, "ERROR: Failed to fork");
			cleanup(EXIT_FAILURE);
		}

		// stop parent with SUCCESS
		if (pid != 0) {
			exit(EXIT_SUCCESS);
		}

		// Create a new SID for child process
		sid = setsid();
		if (sid < 0) {
			syslog(LOG_ERR, "ERROR: Failed to setsid");
			cleanup(EXIT_FAILURE);
    	}

		// Close out the standard file descriptors
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
    }

    // Set up signal handlers
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        syslog(LOG_ERR, "Failed to create socket");
        cleanup(EXIT_FAILURE);
    }

    // Bind to port 9000
    struct addrinfo hints, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if (getaddrinfo(NULL, "9000", &hints, &res) == -1) {
        syslog(LOG_ERR, "getaddrinfo failed");
		cleanup(EXIT_FAILURE);
    }

	// bind socket to address
	if ((bind(sockfd, res->ai_addr, res->ai_addrlen)) == -1)
	{
        syslog(LOG_ERR, "Failed to bind");
        cleanup(EXIT_FAILURE);
	}

    // Listen for connections
    if (listen(sockfd, 5) == -1) {
        syslog(LOG_ERR, "ERROR: Failed to listen");
        close(sockfd);
        return -1;
    }
    
    //free memory allocation
	freeaddrinfo(res); 

    // Open file for aesdsocketdata
    char *aesddata_file = "/var/tmp/aesdsocketdata";
    datafd = open(aesddata_file, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (datafd == -1){
        syslog(LOG_ERR, "ERROR: Failed to create file - %s", aesddata_file);
        exit(EXIT_FAILURE);
    }

    // Accept connections in a loop
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
        
    while (1) {
        client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_sockfd == -1) {
            syslog(LOG_ERR, "Error accepting connection");
            // Continue accepting connections
            continue; 
        }

        // Log accepted connection
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);

        // Receive and process data
        char buffer[buffer_size];
        memset(buffer, 0, buffer_size * sizeof(char));
        int recv_size;
        while ((recv_size = recv(client_sockfd, buffer, buffer_size, 0)) > 0) {
            // Append data to file
            if (write(datafd, buffer, recv_size) == -1) {
                syslog(LOG_ERR, "ERROR: Failed to write to %s file", aesddata_file);
                cleanup(EXIT_FAILURE);
            }

            // Check for newline to consider the packet complete
            if (memchr(buffer, '\n', buffer_size) != NULL) {
                // Reset file offset to the beginning of the file
                lseek(datafd, 0, SEEK_SET);
                size_t bytes_read = read(datafd, buffer, buffer_size);
                if (bytes_read == -1) {
                    syslog(LOG_ERR, "ERROR: Failed to read from %s file", aesddata_file);
                    cleanup(EXIT_FAILURE);
                }
                while (bytes_read > 0) {
                    send(client_sockfd, buffer, bytes_read, 0);
                    bytes_read = read(datafd, buffer, buffer_size); 
                }
            }
            memset(buffer, 0, buffer_size * sizeof(char));
        }

        // Log closed connection
        syslog(LOG_INFO, "Closed connection from %s", client_ip);
        close(client_sockfd);
    }
    return 0;
}

void cleanup(int exit_code) {

    // Close open sockets
    if (sockfd >= 0) close(sockfd);
    if (client_sockfd >=0) close(client_sockfd);

    // Close file descriptors
    if (datafd >= 0) close(datafd);

    // Delete the file
    remove("/var/tmp/aesdsocketdata");

    // Exit
    exit(exit_code);
}

void sig_handler(int signo) {
   if (signo == SIGINT || signo == SIGTERM) {
       syslog(LOG_INFO, "Caught signal, exiting");
       cleanup(EXIT_SUCCESS);
   }
}
