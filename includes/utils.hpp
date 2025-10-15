#ifndef UTILS_HPP
# define UTILS_HPP

# include "Includes.hpp"
# include "Server.hpp"

struct Request
{
	std::string							method;		// HTTP method, POST DELETE
	std::string							uri;		// Client path to the resource (as sent in the HTTP request)
	std::string							path;		// Full local path on the server (uri + location root)
	std::string							body;		// Request content (payload), uploaded file or form data
	std::map<std::string, std::string>	headers;	// HTTP headers, extra infos for handling the request

	Request() {};
};

struct Response
{
	int			status_code;	// HTTP status code, 200, 404, 500
	std::string	content_type;	// MIME content type (cf utils.hpp)
	std::string	body;			// response content (HTML, text, JSON, binary file, …)

	Response() {};
};

std::string	getMimeType(const std::string &path);
std::string	strip_semicolon(const std::string &s);
void		init_default_errors(Server &conf);
inline void	makeResponse(Response &res, int status, const std::string &body, const std::string &content_type = MIME_TEXT_PLAIN)
{
	res.status_code = status;
	res.body = body;
	res.content_type = content_type;
}

#endif
