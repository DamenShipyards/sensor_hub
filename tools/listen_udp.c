#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 

#define MAXLINE 1024 

int main(int argc, char* argv[]) { 
  int e, n, sockfd, port; 
  char buffer[MAXLINE]; 
  struct sockaddr_in servaddr; 

  if (argc < 2) {
    printf("Usage: listen_udp <port>\n");
    exit(EXIT_FAILURE); 
  }

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)  {
    printf("Failed to create socket: %d\n", sockfd);
    exit(EXIT_FAILURE); 
  }

  memset(&servaddr, 0, sizeof(servaddr)); 

  port = atoi(argv[1]);
  printf("Binding to port: %d\n", port);

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY; 
  servaddr.sin_port = htons(port); 

  if ((e = bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) {
    printf("Failed to bind socket: %d\n", e);
    exit(EXIT_FAILURE); 
  }

  while (1) {
    n = recv(sockfd, (char *)buffer, MAXLINE, 0);
    if (n >= 0) {
      buffer[n] = '\0'; 
      printf("Client : %s\n", buffer); 
    }
  }
} 

