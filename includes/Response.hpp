#ifndef RESPONSE_HANDLER_HPP
# define RESPONSE_HANDLER_HPP

# include "Server.hpp"
# include "Utils.hpp"

class ResponseHandler
{
private:
	const Server	&_server;

	Response	handleGet(const Locations &loc, const Request &req);
	Response	handlePost(const Locations &loc, const Request &req);
	Response	handleDelete(const Locations &loc, const Request &req);
public:
	ResponseHandler(const Server &server);
	~ResponseHandler();

	//handle requests
	Response	handleRequest(const Request &req);
};

#endif

