`This project has been created as part of the 42 curriculum by abelmoha, locagnio.`

# Webserv

## <a name="table-of-contents"></a>📑 Table of Contents
- [Description](#description)
  - [Socket Programming](#socket-programming)
  - [Multiplexing I/O](#multiplexing-io)
  - [HTTP Request Parsing](#http-request-parsing)
  - [Configuration File Parsing](#configuration-file-parsing)
  - [CGI Execution](#cgi-execution)
- [Features](#features)
- [Task Distribution](#task-distribution)
- [Instructions](#instructions)
  - [Requirements](#requirements)
  - [Compilation](#compilation)
  - [Execution](#execution)
- [Authorized System Calls](#authorized-system-calls)
- [Resources](#resources)
- [Notes](#notes)

---

## <a name="description"></a>📌 Description

**Webserv** is a project from the **42 school curriculum** whose objective is to build a fully functional **HTTP web server from scratch** using **C++98**.

The project focuses on low-level programming concepts such as:

### <a name="socket-programming"></a>🔹 Socket Programming
| Function | Description |
|----------|-------------|
| socket | Creates a new socket |
| bind | Binds a socket to an IP address and port |
| listen | Marks a socket as passive to accept connections |
| accept | Accepts an incoming client connection |
| connect | Connects a socket to a remote server |
| setsockopt | Sets options on a socket |
| getsockname | Retrieves the local address of a socket |

---

### <a name="multiplexing-io"></a>🔹 Multiplexing I/O
| Function | Description |
|----------|-------------|
| poll | Monitors multiple file descriptors |
| select | Monitors sets of file descriptors |
| epoll_create | Creates an epoll instance (Linux) |
| epoll_ctl | Controls an epoll instance |
| epoll_wait | Waits for epoll events |
| kqueue | Creates an event queue (BSD/macOS) |
| kevent | Registers and waits for events |

---

### <a name="http-request-parsing"></a>🔹 HTTP Request Parsing
| Function | Description |
|----------|-------------|
| recv | Receives data from a socket |
| send | Sends data through a socket |
| read | Reads data from a file or socket |
| write | Writes data to a file or socket |
| strerror | Converts error codes to human-readable strings |

---

### <a name="configuration-file-parsing"></a>🔹 Configuration File Parsing
| Function | Description |
|----------|-------------|
| open | Opens a configuration file |
| read | Reads the configuration file |
| close | Closes a file descriptor |
| stat | Retrieves file information |
| access | Checks file permissions |

---

### <a name="cgi-execution"></a>🔹 CGI Execution
| Function | Description |
|----------|-------------|
| fork | Creates a child process |
| execve | Executes a CGI program |
| waitpid | Waits for a CGI process to terminate |
| pipe | Creates a communication pipe |
| dup | Duplicates a file descriptor |
| dup2 | Redirects standard input/output |
| chdir | Changes the working directory |

No external libraries or modern C++ abstractions are allowed, requiring careful memory management and strict adherence to system-level APIs.

The final goal is to create a **robust, performant, and standards-compliant web server** capable of serving static content, handling dynamic requests via CGI, and managing multiple clients simultaneously.

---

## <a name="features"></a>✨ Features

- HTTP/1.1 request handling  
- Multiple server blocks via configuration file  
- Static file serving  
- CGI support (e.g. PHP)  
- Non-blocking I/O with `poll()`  
- Proper error handling and HTTP status codes  

---

## <a name="task-distribution"></a>👥 Task Distribution

### abelmoha (Lead)
- Server connection handling using `poll()`, sockets, and client management  
- HTTP request parsing based on server blocks from the configuration file  
- CGI (Common Gateway Interface) handling and pipes

### locagnio
- Configuration file parsing
- autoindex
- Requests handling
- Cookies
- CGI (Common Gateway Interface) handling  

---

## <a name="instructions"></a>⚙️ Instructions

### <a name="requirements"></a>Requirements
- Linux or macOS  
- C++ compiler supporting **C++98**  
- `make`  

### <a name="compilation"></a>Compilation
From the root of the repository:

```bash
make
```
```bash
make clean
```
```bash
make fclean
```
```bash
make re
```

### <a name="Execution"></a>Execution

```bash
./webserv (path/to/config_file.conf)
```

Example:

```bash
./webserv
```
```bash
./webserv configs/default.conf
```

The server will start listening on the ports defined in the configuration file.

---

## <a name="authorized-system-calls"></a>📖 Authorized System Calls

This project strictly follows the list of authorized system calls provided by the 42 subject, including (but not limited to):

- Process management: `fork`, `execve`, `waitpid`  
- Networking: `socket`, `bind`, `listen`, `accept`, `send`, `recv`  
- Multiplexing: `poll`, `select`  
- File handling: `open`, `read`, `write`, `close`  
- CGI-related calls: `pipe`, `dup`, `dup2`  

(See subject PDF for the complete list.)

---

## <a name="resources"></a>📚 Resources

### Technical References
- RFC 7230–7235 — HTTP/1.1 Specification  
- Linux manual pages (`man 2`, `man 3`)  
- Beej’s Guide to Network Programming  
- GNU libc documentation  

### AI Usage Disclosure
AI tools (ChatGPT) were used **as an assistance tool only**, specifically for:
- Understanding error messages and debugging compilation issues  
- Clarifying C++98 language rules and system call behavior  
- Affining knowledges on RFC norm
- Improving documentation clarity (README wording and structure)  

All architectural decisions, implementation, and code writing were performed by the project authors.

---

## <a name="notes"></a>🏁 Notes

This project is educational and aims to deepen understanding of how web servers work at a low level, without relying on frameworks or high-level abstractions.
