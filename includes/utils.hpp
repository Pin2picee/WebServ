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
	std::string	version;
	int			status_code;
	std::string	content_type;
	std::string	body;

	Response() : version("HTTP/1.1"){};
};

std::string	strip_semicolon(const std::string &s);
void		init_default_errors(Server &conf);
void		fill_tokens(std::vector<std::string> &dest, const std::vector<std::string> &tokens, size_t &i);
std::string readFile(const std::string& filepath);
long long	convertSize(const std::string &input);
std::string GetUploadFilename(const std::string &body);
void		displayRequestInfo(const Request &req);
std::string getFileName(const std::string &fileBody);
std::string makeJsonError(const std::string &msg);
std::string urlDecode(const std::string &str);

enum StripSide { LEFT, RIGHT, BOTH };

void stripe(std::string &s, StripSide side = BOTH);
void stripe(std::string &s, char c, StripSide side = BOTH);
void stripe(std::string &s, const std::string &set, StripSide side = BOTH);

/**
 * @brief
 * Fill the different variables of a request structure.
 * 
 * @param res The `Response` structure that needs to be filled.
 * @param status The HTTP status code (200, 404, 500, ...).
 * @param body The response content (HTML, text, JSON, binary file, …).
 * @param content_type The MIME content type (sed by default as text plain).
 */

inline Response &makeResponse(Response &res, int status, const std::string &body, const std::string &content_type = MIME_TEXT_PLAIN)
{
	res.status_code = status;
	res.body = body;
	res.content_type = content_type;
	res.version = "HTTP/1.1";
	return res;
}

extern std::vector<Socket *>all_socket;
extern volatile sig_atomic_t	on;

void	handle_sigint(int signum);
#endif
