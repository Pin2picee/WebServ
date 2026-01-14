#ifndef UTILS_HPP
# define UTILS_HPP

# include "Server.hpp"
# include "Socket.hpp"
class Socket;

/**
 * @brief
 * Contains all the request infos.
 * 
 * @param version The HTTP version.
 * @param method The HTTP methods allowed (GET, POST, DELETE).
 * @param uri The client path to the resource (as sent in the HTTP request).
 * @param path The full local path on the server (uri + location root).
 * @param body The request content (payload), uploaded file or form data.
 * @param headers The HTTP headers, extra infos for handling the request.
 */
struct Request
{
	std::string							version;
	std::string							method;
	std::string							uri;
	std::string							query;
	std::string							path;
	std::string							body;
	std::map<std::string, std::string>	headers;
	std::map<std::string, std::string>	cookies;

	Request() {};
};

/**
 * @brief
 * Contains all infos to reply to a request.
 * 
 * @param version The HTTP version, deflaut to 1.1 version.
 * @param status_code The HTTP status code (200, 404, 500, ...).
 * @param content_type The MIME content type (cf utils.hpp).
 * @param body The response content (HTML, text, JSON, binary file, …).
 */
struct Response
{
	std::string					version;
	int							status_code;
	std::string					content_type;
	std::string					body;
	std::vector<std::string>	headers;

	Response() : version("HTTP/1.1"), status_code(200), content_type("text/html"){};
};

//dispay infos
void		displayRequestInfo(const Request &req);
void		displayResponseInfo(const Response &res);

time_t		getCurrentTime();
std::string	generateSessionId(void);
void		parseCookies(Request &req);
void		init_default_errors(Server &conf);
std::string urlDecode(const std::string &str);
std::string cleanPath(const std::string &path);
bool		pathExists(const std::string &path);
std::string readFile(const std::string& filepath);
size_t		convertSize(const std::string &input);
std::string	strip_semicolon(const std::string &s);
std::string makeJsonError(const std::string &msg);
std::string getFileName(const std::string &fileBody);
void		resetUploadsDir(const std::string &uploadsPath);
bool		removeDirectoryRecursive(const std::string &path);
std::string shortenFileName(const std::string &name, size_t maxLength);
std::string getFileClass(const std::string &name, const struct stat &st);
void		fill_tokens(std::vector<std::string> &dest, const std::vector<std::string> &tokens, size_t &i);
int			setCookie(std::string &id, Response &res, const std::string &name, const std::map<std::string, std::string> &cookies);
void 		findHtmlFiles(const std::string &action, const std::string &path);

enum StripSide { LEFT, RIGHT, BOTH };

void		stripe(std::string &s, StripSide side = BOTH);
void		stripe(std::string &s, char c, StripSide side = BOTH);
void		stripe(std::string &s, const std::string &set, StripSide side = BOTH);

/**
 * @brief
 * Fill the different variables of a request structure.
 * 
 * @param res The `Response` structure that needs to be filled.
 * @param req The `Request` structure that will be used to check on the cookies.
 * @param status The HTTP status code (200, 404, 500, ...).
 * @param body The response content (HTML, text, JSON, binary file, …).
 * @param content_type The MIME content type (sed by default as text plain).
 */

inline void	makeResponse(Response &res, int status, const std::string &body, const std::string &content_type = MIME_TEXT_HTML)
{
	res.status_code = status;
	res.body = body;
	res.content_type = content_type;
	res.version = "HTTP/1.1";
}

extern std::vector<Socket *> all_socket;
extern volatile sig_atomic_t	on;

void	handle_sigint(int signum);

#endif
