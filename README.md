# Webserv

Ce projet **Webserv** fait partie du cursus de l'école 42.  
Il consiste à créer un serveur web fonctionnel à partir de zéro, en **C++98**, ce qui implique de gérer la mémoire et les structures de données de manière manuelle.

Nous sommes **trois étudiants** à travailler sur ce projet. La collaboration et la répartition des tâches sont essentielles pour atteindre les objectifs, qui incluent la gestion des requêtes HTTP, le service de fichiers statiques et dynamiques, et la création d’un serveur performant et robuste.


# 📖 Référence des fonctions système autorisées (Webserv)

## 🔹 Gestion des processus
| Prototype | Description |
|-----------|-------------|
| `pid_t fork(void);` | Crée un nouveau processus en dupliquant le courant |
| `int execve(const char *pathname, char *const argv[], char *const envp[]);` | Exécute un programme en remplaçant le processus courant |
| `pid_t waitpid(pid_t pid, int *status, int options);` | Attend la fin d’un processus enfant |
| `int kill(pid_t pid, int sig);` | Envoie un signal à un processus |
| `void (*signal(int signum, void (*handler)(int)))(int);` | Définit un gestionnaire pour un signal |

---

## 🔹 Gestion des erreurs
| Prototype | Description |
|-----------|-------------|
| `char *strerror(int errnum);` | Retourne une chaîne décrivant un code d’erreur |
| `const char *gai_strerror(int errcode);` | Retourne une chaîne d’erreur de `getaddrinfo` |
| `extern int errno;` | Contient le dernier code d’erreur système |

---

## 🔹 Réseau & sockets
| Prototype | Description |
|-----------|-------------|
| `int socket(int domain, int type, int protocol);` | Crée une socket (TCP/UDP) |
| `int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);` | Attache une socket à une IP/port |
| `int listen(int sockfd, int backlog);` | Met une socket en mode écoute |
| `int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);` | Accepte une connexion entrante |
| `int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);` | Connecte une socket à un serveur |
| `ssize_t send(int sockfd, const void *buf, size_t len, int flags);` | Envoie des données sur une socket |
| `ssize_t recv(int sockfd, void *buf, size_t len, int flags);` | Reçoit des données d’une socket |
| `int socketpair(int domain, int type, int protocol, int sv[2]);` | Crée deux sockets connectées entre elles |
| `int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);` | Configure une option de socket |
| `int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);` | Récupère l’adresse locale de la socket |
| `struct protoent *getprotobyname(const char *name);` | Donne des infos sur un protocole (TCP/UDP) |
| `int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);` | Résout un nom en adresse IP/port |
| `void freeaddrinfo(struct addrinfo *res);` | Libère la mémoire de `getaddrinfo` |

---

## 🔹 Conversion d’octets (endianess)
| Prototype | Description |
|-----------|-------------|
| `uint16_t htons(uint16_t hostshort);` | Convertit un entier 16 bits en ordre réseau |
| `uint32_t htonl(uint32_t hostlong);` | Convertit un entier 32 bits en ordre réseau |
| `uint16_t ntohs(uint16_t netshort);` | Convertit un entier 16 bits réseau → hôte |
| `uint32_t ntohl(uint32_t netlong);` | Convertit un entier 32 bits réseau → hôte |

---

## 🔹 Multiplexage & I/O
| Prototype | Description |
|-----------|-------------|
| `int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);` | Surveille plusieurs descripteurs |
| `int poll(struct pollfd *fds, nfds_t nfds, int timeout);` | Alternative moderne à `select` |
| `int epoll_create(int size);` | Crée un objet epoll (Linux) |
| `int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);` | Ajoute/supprime/modifie un fd dans epoll |
| `int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);` | Attend des événements sur epoll |
| `int kqueue(void);` | Crée une file d’événements (BSD/macOS) |
| `int kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout);` | Gère et attend des événements avec kqueue |

---

## 🔹 Fichiers & répertoires
| Prototype | Description |
|-----------|-------------|
| `int open(const char *pathname, int flags, mode_t mode);` | Ouvre/crée un fichier |
| `ssize_t read(int fd, void *buf, size_t count);` | Lit des données d’un fichier/socket |
| `ssize_t write(int fd, const void *buf, size_t count);` | Écrit des données dans un fichier/socket |
| `int close(int fd);` | Ferme un descripteur |
| `int access(const char *pathname, int mode);` | Vérifie les permissions d’un fichier |
| `int stat(const char *pathname, struct stat *statbuf);` | Infos sur un fichier (taille, type, etc.) |
| `int chdir(const char *path);` | Change le répertoire courant |
| `DIR *opendir(const char *name);` | Ouvre un répertoire |
| `struct dirent *readdir(DIR *dirp);` | Lit une entrée de répertoire |
| `int closedir(DIR *dirp);` | Ferme un répertoire |

---

## 🔹 Duplication & contrôle des descripteurs
| Prototype | Description |
|-----------|-------------|
| `int dup(int oldfd);` | Duplique un descripteur |
| `int dup2(int oldfd, int newfd);` | Duplique un fd vers un numéro précis |
| `int fcntl(int fd, int cmd, ...);` | Manipule un descripteur (ex: non bloquant) |
| `int pipe(int pipefd[2]);` | Crée un canal de communication (lecture/écriture) |

---
