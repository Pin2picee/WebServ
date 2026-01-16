#ifndef RESPONSE_HANDLER_HPP
# define RESPONSE_HANDLER_HPP

# include "utils.hpp"

struct Locations;
class Client;

/**
 * @brief
 * Handles HTTP requests and generates appropriate responses.
 * 
 * @param _server	The `Server` instance associated with the handler.
 */
class ResponseHandler
{
private:
	const Server	&_server;

	// Request handlers

	void			handleGet(Response &res, const Locations &loc, const Request &req, Session &session, Client *current);
	void			handlePost(Response &res, const Locations &loc, const Request &req, Session &session, Client *current);
	void			handleDelete(Response &res, const Locations &loc, const Request &req, Session &session);

	// Utilities

	std::string		getMimeType(const Request &req);
	std::string		getMimeType(const std::string &path);
	void			getContentType(Response &res, const Locations &loc, const Request &req, Session &session, Client *current);
	void			makeResponseFromFile(Response &res, int status, const std::string &path, const Request &req);
	void			generateAutoindex(const std::string &fullpath, const std::string &locPath, const Request &req, Response &res, Session &session);
	std::string		generateDeleteFileForm(const Session &session, const std::string &uploadRoot = "./config/www/uploads");
	void			handleFile(std::string &boundary, Response &res, const Locations &loc, const Request &req, Session &session);
public:
	~ResponseHandler();
	ResponseHandler(const Server &server);
	ResponseHandler(const ResponseHandler &copy);
	ResponseHandler &operator=(const ResponseHandler &assignement);

	// Main interface

	Response		handleRequest(const Request &req, std::map<std::string, Session> &g_sessions, Client *current);
	std::string		responseToString(const Response &res);
};

#endif

