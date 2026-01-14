# Webserv

The **Webserv** project is part of the **42 school curriculum**.  
Its goal is to build a fully functional **web server from scratch** in **C++98**, which requires manual memory management and data structure handling without relying on modern abstractions.

We are two students collaborating on this project. Clear task distribution and strong communication are essential to meet the objectives: handling HTTP requests, serving static and dynamic files, and developing a performant and robust server.

---

## Task Distribution

### abelmoha (Lead)
- **Server connection handling** using `poll()`, **sockets**, and client management  
- **HTTP request parsing**, based on the different **server blocks** defined in the configuration file

### locagnio
- **Configuration file parsing**  
- **CGI (Common Gateway Interface) handling**

---

# 📖 Authorized System Calls Reference (Webserv)

## 🔹 Process Management
| Prototype | Description |
|----------|-------------|
| `pid_t fork(void);` | Creates a new process by duplicating the current one |
| `int execve(const char *pathname, char *const argv[], char *const envp[]);` | Executes a program, replacing the current process |
| `pid_t waitpid(pid_t pid, int *status, int options);` | Waits for a child process to terminate |
| `int kill(pid_t pid, int sig);` | Sends a signal to a process |
| `void (*signal(int signum, void (*handler)(int)))(int);` | Sets a signal handler |

---

## 🔹 Error Handling
| Prototype | Description |
|----------|-------------|
| `char *strerror(int errnum);` | Returns a string describing an error code |
| `const char *gai_strerror(int errcode);` | Returns an error string for `getaddrinfo` |
| `extern int errno;` | Contains the last system error code |

---

## 🔹 Networking & Sockets
| Prototype | Description |
|----------|-------------|
| `int socket(int domain, int type, int protocol);` | Creates a socket (TCP/UDP) |
| `int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);` | Binds a socket to an IP address and port |
| `int listen(int sockfd, int backlog);` | Puts a socket into listening mode |
| `int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);` | Accepts an incoming connection |
| `int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);` | Connects a socket to a server |
| `ssize_t send(int sockfd, const void *buf, size_t len, int flags);` | Sends data through a socket |
| `ssize_t recv(int sockfd, void *buf, size_t len, int flags);` | Receives data from a socket |
| `int socketpair(int domain, int type, int protocol, int sv[2]);` | Creates a pair of connected sockets |
| `int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);` | Sets socket options |
| `int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);` | Gets the local socket address |
| `struct protoent *getprotobyname(const char *name);` | Retrieves protocol information (TCP/UDP) |
| `int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);` | Resolves a hostname to an IP/port |
| `void freeaddrinfo(struct addrinfo *res);` | Frees memory allocated by `getaddrinfo` |

---

## 🔹 Byte Order Conversion (Endianness)
| Prototype | Description |
|----------|-------------|
| `uint16_t htons(uint16_t hostshort);` | Converts a 16-bit integer to network byte order |
| `uint32_t htonl(uint32_t hostlong);` | Converts a 32-bit integer to network byte order |
| `uint16_t ntohs(uint16_t netshort);` | Converts a 16-bit integer from network to host |
| `uint32_t ntohl(uint32_t netlong);` | Converts a 32-bit integer from network to host |

---

## 🔹 Multiplexing & I/O
| Prototype | Description |
|----------|-------------|
| `int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);` | Monitors multiple file descriptors |
| `int poll(struct pollfd *fds, nfds_t nfds, int timeout);` | Modern alternative to `select` |
| `int epoll_create(int size);` | Creates an epoll instance (Linux) |
| `int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);` | Adds/modifies/removes a file descriptor in epoll |
| `int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);` | Waits for epoll events |
| `int kqueue(void);` | Creates an event queue (BSD/macOS) |
| `int kevent(int kq, const struct kevent *changelist, int nchanges, const struct kevent *eventlist, int nevents, const struct timespec *timeout);` | Manages and waits for events with kqueue |

---

## 🔹 Files & Directories
| Prototype | Description |
|----------|-------------|
| `int open(const char *pathname, int flags, mode_t mode);` | Opens or creates a file |
| `ssize_t read(int fd, void *buf, size_t count);` | Reads data from a file/socket |
| `ssize_t write(int fd, const void *buf, size_t count);` | Writes data to a file/socket |
| `int close(int fd);` | Closes a file descriptor |
| `int access(const char *pathname, int mode);` | Checks file permissions |
| `int stat(const char *pathname, struct stat *statbuf);` | Retrieves file information |
| `int chdir(const char *path);` | Changes the current working directory |
| `DIR *opendir(const char *name);` | Opens a directory |
| `struct dirent *readdir(DIR *dirp);` | Reads a directory entry |
| `int closedir(DIR *dirp);` | Closes a directory |

---

## 🔹 File Descriptor Duplication & Control
| Prototype | Description |
|----------|-------------|
| `int dup(int oldfd);` | Duplicates a file descriptor |
| `int dup2(int oldfd, int newfd);` | Duplicates a file descriptor to a specific number |
| `int fcntl(int fd, int cmd, ...);` | Manipulates a file descriptor (e.g. non-blocking mode) |
| `int pipe(int pipefd[2]);` | Creates a unidirectional communication channel |

---
