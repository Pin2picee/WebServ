#ifndef RESPONSE_HANDLER_HPP
# define RESPONSE_HANDLER_HPP

# include "utils.hpp"

struct Locations;
class Client;
/**
 * @brief
 * The class that respond to `Requests`.
 * 
 * @param _server	The `Server` that sends a `Request`.
 */
class ResponseHandler
{
private:
	const Server	&_server;
//methods
	Response		handleGet(const Locations &loc, const Request &req, Client *current);
Response 			handlePost(const Locations &loc, const Request &req, Client *current);
	Response		handleDelete(const Locations &loc, const Request &req);

//utils
Response 			&getContentType(Response &res, const Locations &loc, const Request &req, Client *current);
	Response 		&handleFile(std::string &boundary, Response &res, const Locations &loc, const Request &req, Client *current);
	std::string		getMimeType(const Request &req);
public:
	ResponseHandler(const Server &server);
	~ResponseHandler();
	ResponseHandler(const ResponseHandler &copy);
	ResponseHandler &operator=(const ResponseHandler &assignement);

	//handle requests
	Response		handleRequest(const Request &req, Client *current);
	std::string		requestToString(const Request &req);
	std::string		responseToString(const Response &res);
};

#endif

