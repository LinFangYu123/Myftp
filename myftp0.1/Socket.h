#ifndef SOCKET_H
#define SOCKET_H
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>

void perr_exit(const char *str);
int Socket(int family, int type, int protocol);
int Bind(int fd, const struct sockaddr *sa, socklen_t sa_len);
int Listen(int fd, int backlog);
int Accept(int fd, struct sockaddr *sa, socklen_t *sa_lenptr);
int Connect(int fd, const struct sockaddr *sa, socklen_t sa_len);

ssize_t Read(int fd, void *buf, size_t buf_size);
ssize_t Write(int fd, const void *buf, size_t buf_size);
ssize_t Read_one(int fd, char *buf);
ssize_t Read_line(int fd, char *buf, size_t buf_size);


ssize_t Recv(int sockfd, void *buf, size_t len, int flags);
ssize_t Send(int sockfd, const void *buf, size_t len, int flags);
ssize_t Recv_one(int fd, char *buf, int flags);
ssize_t Recv_line(int fd, char *buf, size_t len, int flags);


int Close(int fd);

int initTcpSocket(const int port, char *IP);

int initTcpConn(const int port, char *IP);
#endif