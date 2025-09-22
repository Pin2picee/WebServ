#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <unistd.h>        // fork, execve, pipe, dup, dup2, _exit
#include <sys/wait.h>      // waitpid
#include <signal.h>        // kill, signal
#include <sys/types.h>     //
#include <sys/time.h>      //
#include <sys/socket.h>    // socket, bind, listen, accept, connect, getsockname, setsockopt, recv, send, socketpair
#include <netdb.h>         // getaddrinfo, freeaddrinfo, getprotobyname, gai_strerror
#include <arpa/inet.h>     // htons, htonl, ntohs, ntohl
#include <netinet/in.h>    // struct sockaddr_in
#include <poll.h>          // poll
#include <sys/select.h>    // select
#include <sys/epoll.h>     // epoll
#include <sys/stat.h>      // stat
#include <fcntl.h>         // open, fcntl
#include <dirent.h>        // opendir, readdir, closedir
#include <string.h>        // strerror
#include <errno.h>         // errno

#include <iostream>

#endif
