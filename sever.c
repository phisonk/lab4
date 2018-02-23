#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#define BUF_SIZE 100

void header(int handler, int status) {
  char header[1000] = {0};
  if (status == 0) {
    sprintf(header, "HTTP/1.0 200 OK\r\n\r\n");
    printf("HTTP/1.0 200 OK\r\n\r\n");
  } else if (status == 1) {
    sprintf(header, "HTTP/1.0 403 Forbidden\r\n\r\n");
  } else {
    sprintf(header, "HTTP/1.0 404 Not Found\r\n\r\n");
    printf("HTTP/1.0 404 Not Found\r\n\r\n");
  }
  send(handler, header, strlen(header), 0);
}

void resolve(int handler) {
  int status = 0;
  char buf[BUF_SIZE];
  char *method;
  char *filename;

  recv(handler, buf, BUF_SIZE, 0);
  method = strtok(buf, " ");
  if (strcmp(method, "GET") != 0) return;

  filename = strtok(NULL, " ");
  if (filename[0] == '/') filename++;
  if (access(filename, F_OK) != 0) {
    header(handler, 2);
    return;
  } else if (access(filename, R_OK) != 0){
    header(handler, 1);
    return;
  } else {
    header(handler, 0);
  }

  FILE *file = fopen(filename, "r");
  printf("%s\n",filename);
  while(fgets(buf, BUF_SIZE, file)) {
    write(handler, buf, strlen(buf));
    memset(buf, 0, BUF_SIZE);
  }
}

int main(int argc, char** argv) {
    struct addrinfo hints, *server;
    memset(&hints, 0, sizeof hints);
    hints.ai_family =  AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE || SOCK_NONBLOCK;
    getaddrinfo(NULL, "8080", &hints, &server);

    int sockfd = socket(server->ai_family,
            server->ai_socktype, server->ai_protocol);
    bind(sockfd, server->ai_addr, server->ai_addrlen);
    listen(sockfd, 10);
    printf("Listen\n");
 
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof client_addr;
    char headers[] = "HTTP/1.0 200 OK\r\nServer: CPi\r\nContent-type: text/html\r\n\r\n";
    char buffer[2048];
    char html[] = "<html><head><title>Temperature</title></head><body>{\"humidity\":81%,\"airtemperature\":23.5C}</p></body></html>\r\n";
    // FILE* fp = fopen("index.html", "r");
    // char html[];
    // char ch;
    // while( ( ch = fgetc(fp) ) != EOF )
    //   html[] = ch;
 
    // fclose(fp);
    char data[2048] = {0};
    snprintf(data, sizeof data,"%s %s", headers, html);
    printf("GG\n");
    for (;;) {
        int client_fd = accept(sockfd,
        (struct sockaddr *) &client_addr, &addr_size);
        if (client_fd > 0) {
            int n = read(client_fd, buffer, 2048);
            printf("%s", buffer);
            fflush(stdout);
            n = write(client_fd, data, strlen(data));
            close(client_fd); 
        }
        printf("wow\n");
        if (client_fd < 0) {
        perror("[main:82:accept]");
        continue;
        }

        // handle async
        switch (fork()) {
        case -1:
            perror("[main:88:fork]");
            break;
        case 0:
            printf("AW\n");
            close(sockfd);
            resolve(client_fd);
            close(client_fd);
            exit(0);
        default:
            close(client_fd);
        }
    }
    printf("close\n");
    return (EXIT_SUCCESS);
}