#ifndef UTILS_HPP
# define UTILS_HPP

# include "Server.hpp"
# include "Socket.hpp"
class Socket;

/**
 * @brief Contains all the request infos.
 *
 * @param version	The HTTP version.
 * @param method	The HTTP methods allowed (GET, POST, DELETE).
 * @param uri		The client path to the resource (as sent in the HTTP request).
 * @param path		The full local path on the server (uri + location root).
 * @param query		The query string if present.
 * @param body		The request content (payload), uploaded file or form data.
 * @param headers	The HTTP headers, extra infos for handling the request.
 * @param cookies	The cookies sent by the client.
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
 * @brief Contains all info to reply to a request.
 *
 * @param version		The HTTP version, defaults to 1.1.
 * @param status_code	The HTTP status code (200, 404, 500, ...).
 * @param content_type	The MIME content type (e.g., text/html).
 * @param body			The response content (HTML, text, JSON, binary file, …).
 * @param headers		The additional HTTP headers.
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

/* ---------------- Display Infos ---------------- */

void		displayRequestInfo(const Request &req);
void		displayResponseInfo(const Response &res);

/* ---------------- Time / Session Utils ---------------- */

time_t		getCurrentTime();
std::string	generateSessionId(void);
void		parseCookies(Request &req);
void		init_default_errors(Server &conf);

/* ---------------- File / Path Utils ---------------- */

std::string urlDecode(const std::string &str);
std::string cleanPath(const std::string &path);
bool		pathExists(const std::string &path);
std::string readFile(const std::string& filepath);
void		resetUploadsDir(const std::string &uploadsPath);
bool		removeDirectoryRecursive(const std::string &path);

/* ---------------- String / Parsing Utils ---------------- */

std::string ftToString(size_t nb);
size_t		convertSize(const std::string &input);
std::string	stripSemicolon(const std::string &s);
std::string shortenFileName(const std::string &name, size_t maxLength);
enum StripSide { LEFT, RIGHT, BOTH };
void		stripe(std::string &s, const std::string &set, StripSide side = BOTH);
void		fillTokens(std::vector<std::string> &dest, const std::vector<std::string> &tokens, size_t &i);

/* ---------------- Response / HTTP Utils ---------------- */

std::string makeJsonError(const std::string &msg);
std::string getFileName(const std::string &fileBody);
std::string getFileClass(const std::string &name, const struct stat &st);
void 		findHtmlFiles(const std::string &action, const std::string &path);
int			setCookie(std::string &id, Response &res, const std::string &name, const std::map<std::string, std::string> &cookies);

/**
 * @brief
 * Fill a Response structure with status, body, and content type.
 *
 * @param res			The Response structure that will be filled.
 * @param status		The HTTP status code (e.g., 200, 404, 500, ...).
 * @param body			The content of the response (HTML, text, JSON, etc.).
 * @param content_type	The MIME type of the response content, defaults to text/html.
 */
inline void	makeResponse(Response &res, int status, const std::string &body, const std::string &content_type = MIME_TEXT_HTML)
{
	res.status_code = status;
	res.body = body;
	res.content_type = content_type;
	res.version = "HTTP/1.1";
}

/* ---------------- Globals ---------------- */

extern std::vector<Socket *> all_socket;
extern volatile sig_atomic_t	on;

/* ---------------- Signals ---------------- */

void	handle_sigint(int signum);

#endif
