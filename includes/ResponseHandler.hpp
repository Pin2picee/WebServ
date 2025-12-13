#ifndef RESPONSE_HANDLER_HPP
# define RESPONSE_HANDLER_HPP

# include "utils.hpp"

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
	void	handleGet(Response &res, const Locations &loc, const Request &req, Session &session);
	void	handlePost(Response &res, const Locations &loc, const Request &req, Session &session);
	void	handleDelete(Response &res, const Locations &loc, const Request &req, Session &session);

//utils
	std::string		getMimeType(const Request &req);
	void			getContentType(Response &res, const Locations &loc, const Request &req, Session &session);
	void			handleFile(std::string &boundary, Response &res, const Locations &loc, const Request &req, Session &session);
	std::string		generateDeleteFileForm(const Session &session, const std::string &uploadRoot = "./config/www/uploads");
public:
	ResponseHandler(const Server &server);
	~ResponseHandler();
	ResponseHandler(const ResponseHandler &copy);
	ResponseHandler &operator=(const ResponseHandler &assignement);

	//handle requests
	Response		handleRequest(const Request &req);
	std::string		requestToString(const Request &req);
	std::string		responseToString(const Response &res);
};

#endif

