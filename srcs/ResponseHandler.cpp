#include "ResponseHandler.hpp"
#include "Client.hpp"
/* constructor */
ResponseHandler::ResponseHandler(const Server &server) : _server(server) {}

/* destructor */
ResponseHandler::~ResponseHandler() {}

/**
 * @brief
 * Handle any `Request`.
 * 
 * @param req The `Request` struct that will be processed.
 * 
 * @return a `Response` structure that will answer in adequation to the `Request`.
 */
Response ResponseHandler::handleRequest(const Request &req, Client *current)
{
	Response	res;
	const std::vector<Locations> &locs = _server.getLocations();
	const Locations *target = NULL;

	for (size_t i = 0; i < locs.size(); ++i)
	{
		if (!req.path.find(locs[i].path))
			target = &locs[i];
	}
	if (!target)
		makeResponse(res, 404, readFile(_server.getErrorPage(404)), getMimeType(req));
	else if (req.method == "GET")
		res = handleGet(*target, req, current);
	else if (req.method == "POST")
		res = handlePost(*target, req, current);
	else if (req.method == "DELETE")
		res = handleDelete(*target, req);
	else
		makeResponse(res, 405, readFile(_server.getErrorPage(405)), getMimeType(req));
	return res;
}

/**
 * @brief
 * Handle the get `Request`.
 * 
 * @param loc The `Request` location that will be found in `req` variable of `handleRequest` method.
 * @param req The `Request` struct of `handleRequest` that will be processed.
 * 
 * @return a `Response` structure that will answer in adequation to the get `Request`.
 */
Response ResponseHandler::handleGet(const Locations &loc, const Request &req, Client *current)
{
	Response	res;
	std::string	full_path =  _server.getRoot() + req.path;
	struct stat	s;

	if (std::find(loc.methods.begin(), loc.methods.end(), "GET") == loc.methods.end())
		makeResponse(res, 405, readFile(_server.getErrorPage(405)), getMimeType(req));
	if (req.path == "/teapot")
	    return makeResponse(res, 418, readFile(_server.getErrorPage(418)), getMimeType(req));
	else if (full_path.empty() || full_path == RED "none")
		makeResponse(res, 400, readFile(_server.getErrorPage(400)), getMimeType(req));
	else if (stat(full_path.c_str(), &s) == 0 && S_ISDIR(s.st_mode))
	{
		if (loc.autoindex)
		{
			std::ostringstream body;
			body << "<html><body><h1>Index of " << req.uri << "</h1><ul>";

			DIR *dir = opendir(full_path.c_str());
			if (dir)
			{
				struct dirent *entry;
				while ((entry = readdir(dir)))
					body << "<li><a href=\"" << entry->d_name << "\">" << entry->d_name << "</a></li>";
				closedir(dir);
			}
			body << "</ul></body></html>";
			makeResponse(res, 200, body.str(), getMimeType(req));
		}
		else
		{
			bool found = false;
			for (size_t i = 0; i < loc.index_files.size(); i++)
			{
				std::string index_path = full_path + "/" + loc.index_files[i];
				struct stat s;
				if (stat(index_path.c_str(), &s) == 0)
				{
					std::string content = readFile(index_path); // utilisation de la nouvelle fonction
					makeResponse(res, 200, content, getMimeType(req));
					found = true;
					break;
				}
			}
			if (!found)
				makeResponse(res, 403, readFile(_server.getErrorPage(403)), getMimeType(req));
		}
		return res;
	}
	else
	{
		std::ifstream end_ifs(full_path.c_str(), std::ios::binary);
		//std::cerr << full_path.c_str() << std::endl;//AFFICHAGE DU PATH
		if (!end_ifs)
			makeResponse(res, 404, readFile(_server.getErrorPage(404)), getMimeType(req));
		else if (loc.cgi && full_path.size() >= loc.cgi_extension.size() &&
			full_path.substr(full_path.size() - loc.cgi_extension.size()) == loc.cgi_extension)
				res = _server.handleCGI(req, loc, current);
		else
		{
			std::ostringstream buf;
			buf << end_ifs.rdbuf();
			makeResponse(res, 200, buf.str(), getMimeType(req));
		}
	}
	return res;
}

/**
 * @brief
 * Manage the POST request of a file.
 * 
 * @param boundary The start and end point of the file body.
 * @param res The `Response` structure that will be returned.
 * @param loc The `Request` location that will be found in `req` variable of `handleRequest` method.
 * @param req The `Request` struct of `handleRequest` that will be processed.
 * 
 * @return a `Response` structure that will answer in adequation to the post `Request`.
 */
Response &ResponseHandler::handleFile(std::string &boundary, Response &res, const Locations &loc, const Request &req)
{
	if (boundary.empty())
		return makeResponse(res, 400, readFile(_server.getErrorPage(400)), getMimeType(req));
	std::size_t fileStart = req.body.find("filename=\"");
	if (fileStart == std::string::npos)
		return makeResponse(res, 400, readFile(_server.getErrorPage(400)), getMimeType(req));
	std::string filename = getFileName(req.body);
	if (filename.empty())
		return makeResponse(res, 204, "", getMimeType(req));
	filename =  _server.getRoot() + loc.upload_dir + "/" + filename;
	fileStart = req.body.find("\r\n\r\n", fileStart);
	if (fileStart == std::string::npos)
		return makeResponse(res, 400, readFile(_server.getErrorPage(400)), getMimeType(req));
	fileStart += 4;
	std::size_t fileEnd = req.body.find(boundary, fileStart);
	if (fileEnd == std::string::npos)
		return makeResponse(res, 400, readFile(_server.getErrorPage(400)), getMimeType(req));
	fileEnd -= 2;
	if (fileEnd - fileStart > _server.getClientMaxBodySize())
		return makeResponse(res, 413, readFile(_server.getErrorPage(413)), getMimeType(req));
	std::cout << "filename" << " \"" << filename << "\"." << std::endl;
	std::ofstream ofs(filename.c_str(), std::ios::binary);
	if (!ofs)
		return makeResponse(res, 500, readFile(_server.getErrorPage(500)), getMimeType(req));
	ofs.write(req.body.c_str() + fileStart, fileEnd - fileStart);
	ofs.close();
	if (loc.cgi && filename.size() >= loc.cgi_extension.size())
	{
		std::string file_ext = filename.substr(filename.size() - loc.cgi_extension.size());
		if (file_ext == loc.cgi_extension)
			return res;		
	}
	return makeResponse(res, 201, readFile(_server.getErrorPage(201)), getMimeType(req));
}

/**
 * @brief
 * Manage a POST request by finding if the content is a file or not and produce a response accordingly.
 * 
 * @param res The `Response` structure that will be returned.
 * @param loc The `Request` location that will be found in `req` variable of `handleRequest` method.
 * @param req The `Request` struct of `handleRequest` that will be processed.
 * 
 * @return a `Response` structure that will answer in adequation to the post `Request`.
 */
Response &ResponseHandler::getContentType(Response &res, const Locations &loc, const Request &req, Client *current)
{
	std::string contentType;
	std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Type");

	if (it != req.headers.end())
		contentType = it->second;
	else
		contentType = "application/octet-stream";
	std::string boundary;
	std::size_t pos = contentType.find("boundary=");
	if (pos != std::string::npos)
	{
		boundary = "--" + contentType.substr(pos + 9);
		return handleFile(boundary, res, loc, req);
	}
	else
	{
		std::cout << GREEN << "dans " << RESET << std::endl;
		if (req.body.size() > _server.getClientMaxBodySize())
			return makeResponse(res, 413, readFile(_server.getErrorPage(413)), getMimeType(req));
	}
	size_t	size_path = req.path.size();
	size_t	point = req.path.rfind('.');
	if (point == std::string::npos)
		return makeResponse(res, 405, readFile(_server.getErrorPage(405)), getMimeType(req));
	std::string	extension_path = req.path.substr(req.path.rfind('.'));
	std::vector<std::string>	pack_extension_CGI;
	pack_extension_CGI.push_back(".cgi");
	pack_extension_CGI.push_back(".py");
	pack_extension_CGI.push_back(".php");
	pack_extension_CGI.push_back(".pl");
	pack_extension_CGI.push_back(".rb");
	pack_extension_CGI.push_back(".sh");

	std::vector<std::string>::iterator itt = std::find(pack_extension_CGI.begin(), pack_extension_CGI.end(), extension_path);
	if (size_path >= 5 && pack_extension_CGI.end() == itt)
		return makeResponse(res, 405, readFile(_server.getErrorPage(405)), getMimeType(req));
	res = _server.handleCGI(req, loc, current);
	return (res);
}

/**
 * @brief
 * Handle the post `Request`.
 * 
 * @param loc The `Request` location that will be found in `req` variable of `handleRequest` method.
 * @param req The `Request` struct of `handleRequest` that will be processed.
 * 
 * @return a `Response` structure that will answer in adequation to the post `Request`.
 */
Response ResponseHandler::handlePost(const Locations &loc, const Request &req, Client *current)
{
	Response res;

	if (std::find(loc.methods.begin(), loc.methods.end(), "POST") == loc.methods.end())
		return makeResponse(res, 405, readFile(_server.getErrorPage(405)), getMimeType(req));
	if (req.body.size() > _server.getClientMaxBodySize())
		return makeResponse(res, 413, readFile(_server.getErrorPage(413)), getMimeType(req));
	return getContentType(res, loc, req, current);
}

/**
 * @brief
 * Handle the delete `Request`.
 * 
 * @param loc The `Request` location that will be found in `req` variable of `handleRequest` method.
 * @param req The `Request` struct of `handleRequest` that will be processed.
 * 
 * @return a `Response` structure that will answer in adequation to the delete `Request`.
 */

Response ResponseHandler::handleDelete(const Locations &loc, const Request &req)
{
	Response res;

	if (std::find(loc.methods.begin(), loc.methods.end(), "DELETE") == loc.methods.end())
		return makeResponse(res, 405, makeJsonError("DELETE not allowed on this location"), getMimeType(req));
	std::string filename = urlDecode(req.query);
	std::string::size_type pos = filename.rfind('/');
	if (pos != std::string::npos)
		filename = filename.substr(pos + 1);
	std::cout << filename << std::endl;
	if (filename.empty())
		return makeResponse(res, 400, makeJsonError("Filename is required"), getMimeType(req));
	std::cout << "Deleting file: " << filename << std::endl;
	std::string deletePath = _server.getRoot() + "/" + loc.upload_dir + "/" + filename;
	std::ifstream file(deletePath.c_str());
	if (!file || std::remove(deletePath.c_str()) != 0)
	{
		std::cerr << "Failed to delete file: " << deletePath << std::endl;
		return makeResponse(res, 404, makeJsonError("File not found"), getMimeType(req));
	}
	return makeResponse(res, 200, makeJsonError("File deleted successfully"), getMimeType(req));
}


//utils
/**
 * @brief
 * Get the reason phrase of a corresponding status code.
 * 
 * @param status_code The status code that will get a corresponding reason phrase.
 * 
 * @return
 * The corresponding reason phrase.
 */

static std::string getReasonPhrase(int status_code)
{
	switch (status_code)
	{
		case 100: return "Continue";
		case 101: return "Switching Protocols";
		case 200: return "OK";
		case 201: return "Created";
		case 202: return "Accepted";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 304: return "Not Modified";
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 408: return "Request Timeout";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		default:  return "Unknown";
	}
}

/**
 * @brief
 * Convert a Response struct into a string in standard HTTP format.
 * 
 * @param res The Response struct that will be converted.
 * 
 * @return
 * The string HTTP message.
 */
std::string	ResponseHandler::responseToString(const Response &res)
{
	std::ostringstream oss;

	oss << res.version << " " << res.status_code << " " << getReasonPhrase(res.status_code) << "\r\n";
	if (!res.content_type.empty())
		oss << "Content-Type: " << res.content_type << "\r\n";
	oss << "Content-Length: " << res.body.size() << "\r\n";
	oss << "\r\n" << res.body;
	return oss.str();
}

/**
 * @brief
 * Convert a Request struct into a string in standard HTTP format.
 * 
 * @param req The Request struct that will be converted.
 * 
 * @return
 * The string HTTP message.
 */
std::string	ResponseHandler::requestToString(const Request &req)
{
	std::ostringstream oss;

	oss << req.method << " " << req.path << " " << req.version << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = req.headers.begin();
		 it != req.headers.end(); ++it)
		oss << it->first << ": " << it->second << "\r\n";
	oss << "\r\n" << req.body;
	return oss.str();
}

ResponseHandler::ResponseHandler(const ResponseHandler &copy) : _server(copy._server)
{
	if (this != &copy)
		*this = copy;
}

ResponseHandler &ResponseHandler::operator=(const ResponseHandler &assignement)
{
	(void)assignement;
	return (*this);// rien a mettre egal a l'assignement car la seule variable est const donc deja init
}
	//handle requests

	/**
 * @param path The request path
 *  
 * @return
 * Return the MIME (Multipurpose Internet Mail Extensions) content type that have `Response` when responding to a `Request`.
 */
std::string ResponseHandler::getMimeType(const Request &req)
{
	std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Type");
	if (it != req.headers.end())
	{
		std::string contentType = it->second;
		if (contentType.find("text/html") != std::string::npos)
			return MIME_TEXT_HTML;
		if (contentType.find("text/css") != std::string::npos)
			return MIME_TEXT_CSS;
		if (contentType.find("application/javascript") != std::string::npos || contentType.find("text/javascript") != std::string::npos)
			return MIME_TEXT_JAVASCRIPT;
		if (contentType.find("image/jpeg") != std::string::npos)
			return MIME_IMAGE_JPEG;
		if (contentType.find("image/png") != std::string::npos)
			return MIME_IMAGE_PNG;
		if (contentType.find("image/gif") != std::string::npos)
			return MIME_IMAGE_GIF;
	}
	if (req.path.find(".html") != std::string::npos)
		return MIME_TEXT_HTML;
	if (req.path.find(".css") != std::string::npos)
		return MIME_TEXT_CSS;
	if (req.path.find(".js") != std::string::npos)
		return MIME_TEXT_JAVASCRIPT;
	if (req.path.find(".jpg") != std::string::npos || req.path.find(".jpeg") != std::string::npos)
		return MIME_IMAGE_JPEG;
	if (req.path.find(".png") != std::string::npos)
		return MIME_IMAGE_PNG;
	if (req.path.find(".gif") != std::string::npos)
		return MIME_IMAGE_GIF;
	return MIME_TEXT_HTML;
}
