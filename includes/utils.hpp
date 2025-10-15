#ifndef UTILS_HPP
# define UTILS_HPP

# include "Server.hpp"

/**
 * @brief
 * Contains all the request infos.
 * 
 * @param method The HTTP methods allowed (GET, POST, DELETE).
 * @param uri The client path to the resource (as sent in the HTTP request).
 * @param path The full local path on the server (uri + location root).
 * @param body The request content (payload), uploaded file or form data.
 * @param headers The HTTP headers, extra infos for handling the request.
 */
struct Request
{
	std::string							method;
	std::string							uri;
	std::string							path;
	std::string							body;
	std::map<std::string, std::string>	headers;

	Request() {};
};

/**
 * @brief
 * Contains all infos to reply to a request.
 * 
 * @param status_code The HTTP status code (200, 404, 500, ...).
 * @param content_type The MIME content type (cf utils.hpp).
 * @param body The response content (HTML, text, JSON, binary file, …).
 */
struct Response
{
	int			status_code;
	std::string	content_type;
	std::string	body;

	Response() {};
};

std::string	getMimeType(const std::string &path);
std::string	strip_semicolon(const std::string &s);
void		init_default_errors(Server &conf);
/**
 * @brief
 * Fill the different variables of a request structure.
 * 
 * @param res The `Response` structure that needs to be filled.
 * @param status The HTTP status code (200, 404, 500, ...).
 * @param body The response content (HTML, text, JSON, binary file, …).
 * @param content_type The MIME content type (sed by default as text plain).
 */
inline void	makeResponse(Response &res, int status, const std::string &body, const std::string &content_type = MIME_TEXT_PLAIN)
{
	res.status_code = status;
	res.body = body;
	res.content_type = content_type;
}

#endif
