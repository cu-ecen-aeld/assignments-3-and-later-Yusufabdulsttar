

// Used http://www.cs.cmu.edu/~prs/15-441-F10/lectures/r01-sockets.pdf as reference
// for setting up sockets.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <syslog.h>

#define SERVER_PORT 9000
#define BUFFER_SIZE 1024

int sockfd, connfd;

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        // Close the socket
        close(sockfd);
        // Delete the file
        remove("/var/tmp/aesdsocketdata");
        // Log the message
        syslog(LOG_INFO, "Caught signal, exiting");
        // Close syslog
        closelog();
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    pid_t pid, sid;
    if ((argc > 1) && (strcmp(argv[1], "-d") == 0)) {
        pid = fork();
        if (pid < 0) {
            exit(-1);
        } else if (pid > 0) {
            exit(0);
        }
        
        sid = setsid();
        if (sid < 0) {
            exit(-1);
        }
        
        if (chdir("/") < 0) {
            exit(-1);
        }
    }
    openlog("aesdsocket", LOG_CONS | LOG_PID, LOG_USER);
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket: Failed to create socket.");
        return -1;
    }
    
    const int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt: (SO_REUSEADDR) failed.");
    }


    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind: Failed to bind name to socket.");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 5) < 0) {
        perror("listen: Failed to listen for connections.");
        close(sockfd);
        return -1;
    }

    while (1) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int connfd = accept(sockfd, (struct sockaddr*)&client, &client_len);

        if (connfd < 0) {
            perror("accept: Failed connecting to client.");
            continue;
        }
        
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        getpeername(connfd, (struct sockaddr *)&addr, &addr_len);
        char *client_ip = inet_ntoa(addr.sin_addr);
        printf("Accepted connection from %s\n", client_ip);
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);
        
        char* buffer = calloc(BUFFER_SIZE, sizeof(char) * BUFFER_SIZE);

        while (recv(connfd, buffer, sizeof(buffer), 0) != 0) {
            FILE *fp = fopen("/var/tmp/aesdsocketdata", "a");
            if (fp == NULL) {
                printf("fopen: Failed opening file.\n");
                perror("fopen: Failed opening file.");
                return -1;
            } else {
                fprintf(fp, "%s", buffer);
                fclose(fp);
            }

            if (strchr(buffer, '\n') != NULL) {
                FILE *fp_out = fopen("/var/tmp/aesdsocketdata", "r");
                if (fp_out == NULL) {
                    printf("fopen: Failed opening file.\n");
                    perror("fopen: Failed opening file.");
                    return -1;
                } else {
                    uint32_t bytes_read = 0;
                    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp_out)) != 0) {
                        send(connfd, buffer, bytes_read, 0);
                    }                
                }
                fclose(fp_out);
            }
            memset(buffer, 0, sizeof(char) * BUFFER_SIZE);
        }
        free(buffer);
        close(connfd);
        syslog(LOG_INFO, "Closed connection from %s", client_ip);

    }
    
    // Close open sockets
    close(sockfd);
    close(connfd);

    // Delete the file
    remove("/var/tmp/aesdsocketdata");

    // Close syslog
    closelog();

    // Exit
    exit(0);
    return 0;
}
