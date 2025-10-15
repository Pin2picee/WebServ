#ifndef INCLUDES_HPP
# define INCLUDES_HPP

# include <unistd.h>        // fork, execve, pipe, dup, dup2, _exit
# include <sys/wait.h>      // waitpid
# include <signal.h>        // kill, signal
# include <sys/types.h>     //
# include <sys/time.h>      //
# include <sys/socket.h>    // socket, bind, listen, accept, connect, getsockname, setsockopt, recv, send, socketpair
# include <netdb.h>         // getaddrinfo, freeaddrinfo, getprotobyname, gai_strerror
# include <arpa/inet.h>     // htons, htonl, ntohs, ntohl
# include <netinet/in.h>    // struct sockaddr_in
# include <poll.h>          // poll
# include <sys/select.h>    // select
# include <sys/epoll.h>     // epoll
# include <sys/stat.h>      // stat
# include <fcntl.h>         // open, fcntl
# include <dirent.h>        // opendir, readdir, closedir
# include <string.h>        // strerror
# include <errno.h>         // errno

# include <iostream>
# include <string>
# include <vector>
# include <map>
# include <utility>
# include <fstream>
# include <sstream>
# include <stdexcept>
# include <algorithm>
# include <cstdlib>


// Text colors
# define RESET   "\033[0m"
# define RED     "\033[31m"
# define GREEN   "\033[32m"
# define YELLOW  "\033[33m"
# define BLUE    "\033[34m"
# define MAGENTA "\033[35m"
# define CYAN    "\033[36m"

// Text styles (optionnel)
# define BOLD       "\033[1m"
# define UNDERLINE  "\033[4m"

//=== MIME (Multipurpose Internet Mail Extensions) content types ===//
// Text types
# define MIME_TEXT_PLAIN       "text/plain"
# define MIME_TEXT_HTML        "text/html"
# define MIME_TEXT_CSS         "text/css"
# define MIME_TEXT_JAVASCRIPT  "application/javascript"

// Data / JSON
# define MIME_APPLICATION_JSON "application/json"
# define MIME_APPLICATION_XML  "application/xml"

// Images
# define MIME_IMAGE_PNG        "image/png"
# define MIME_IMAGE_JPEG       "image/jpeg"
# define MIME_IMAGE_GIF        "image/gif"
# define MIME_IMAGE_SVG        "image/svg+xml"

// Fonts
# define MIME_FONT_WOFF        "font/woff"
# define MIME_FONT_WOFF2       "font/woff2"
# define MIME_FONT_TTF         "font/ttf"

// Binary / Misc
# define MIME_APPLICATION_OCTET_STREAM "application/octet-stream"

#endif
