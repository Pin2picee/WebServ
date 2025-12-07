#include "ResponseHandler.hpp"

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
Response ResponseHandler::handleRequest(const Request &req)
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
		makeResponse(res, 404, readFile(_server.getErrorPage(404)), getMimeType(req.path));
	else if (req.method == "GET")
		res = handleGet(*target, req);
	else if (req.method == "POST")
		res = handlePost(*target, req);
	else if (req.method == "DELETE")
		res = handleDelete(*target, req);
	else
		makeResponse(res, 405, readFile(_server.getErrorPage(405)), getMimeType(req.path));
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
Response ResponseHandler::handleGet(const Locations &loc, const Request &req)
{
	Response	res;
	std::string	full_path =  _server.getRoot() + req.path;
	struct stat	s;

	if (std::find(loc.methods.begin(), loc.methods.end(), "GET") == loc.methods.end())
		makeResponse(res, 405, readFile(_server.getErrorPage(405)), getMimeType(full_path));
	else if (full_path.empty() || full_path == RED "none")
		makeResponse(res, 400, readFile(_server.getErrorPage(400)), getMimeType(full_path));
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
			makeResponse(res, 200, body.str(), getMimeType(full_path));
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
					makeResponse(res, 200, content, getMimeType(full_path));
					found = true;
					break;
				}
			}
			if (!found)
				makeResponse(res, 403, readFile(_server.getErrorPage(403)), getMimeType(full_path));
			
		}
		return res;
	}
	else
	{
		std::ifstream end_ifs(full_path.c_str(), std::ios::binary);
		//std::cerr << full_path.c_str() << std::endl;//AFFICHAGE DU PATH
		if (!end_ifs)
			makeResponse(res, 404, readFile(_server.getErrorPage(404)), getMimeType(full_path));
		else if (loc.cgi && full_path.size() >= loc.cgi_extension.size() &&
			full_path.substr(full_path.size() - loc.cgi_extension.size()) == loc.cgi_extension)
				res = _server.handleCGI(req, loc);
		else
		{
			std::ostringstream buf;
			buf << end_ifs.rdbuf();
			makeResponse(res, 200, buf.str(), getMimeType(full_path));
		}
	}
	return res;
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
Response ResponseHandler::handlePost(const Locations &loc, const Request &req)
{
	Response res;

	std::cout << "path = " << req.path << "\nbody = " << req.body << std::endl;

	if (std::find(loc.methods.begin(), loc.methods.end(), "POST") == loc.methods.end())
		makeResponse(res, 405, readFile(_server.getErrorPage(405)), getMimeType(req.path));
	else if (req.body.size() > _server.getClientMaxBodySize())
		makeResponse(res, 413, readFile(_server.getErrorPage(413)), getMimeType(req.path));
	else if (loc.upload_dir != RED "none")
	{
		// Récupérer le boundary depuis Content-Type
		std::string contentType;
		std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Type");
		if (it != req.headers.end())
			contentType = it->second;
		else
		{
			makeResponse(res, 400, readFile(_server.getErrorPage(400)), getMimeType(req.path));
			return res;
		}
		std::string boundary;
		std::size_t pos = contentType.find("boundary=");
		if (pos != std::string::npos)
			boundary = "--" + contentType.substr(pos + 9); // ajouter "--" comme dans le body

		if (boundary.empty())
		{
			makeResponse(res, 400, readFile(_server.getErrorPage(400)), getMimeType(req.path));
			return res;
		}
		// Chercher le début et la fin du fichier
		std::size_t fileStart = req.body.find("filename=\"");
		if (fileStart == std::string::npos)
		{
			makeResponse(res, 400, readFile(_server.getErrorPage(400)), getMimeType(req.path));
			return res;
		}
		fileStart = req.body.find("\r\n\r\n", fileStart);
		if (fileStart == std::string::npos)
		{
			makeResponse(res, 400, readFile(_server.getErrorPage(400)), getMimeType(req.path));
			return res;
		}
		fileStart += 4; // sauter "\r\n\r\n"

		std::size_t fileEnd = req.body.find(boundary, fileStart);
		if (fileEnd == std::string::npos)
		{
			makeResponse(res, 400, readFile(_server.getErrorPage(400)), getMimeType(req.path));
			return res;
		}
		fileEnd -= 2; // retirer "\r\n" avant le boundary

		std::string filename = loc.upload_dir + "/uploaded_file"; // ou extraire le vrai nom du fichier

		std::ofstream ofs(filename.c_str(), std::ios::binary);
		if (!ofs)
		{
			makeResponse(res, 500, readFile(_server.getErrorPage(500)), getMimeType(req.path));
			return res;
		}
		ofs.write(req.body.c_str() + fileStart, fileEnd - fileStart);
		ofs.close();

		makeResponse(res, 201, readFile(_server.getErrorPage(201)), getMimeType(req.path));
	}
	else if (loc.cgi)
		res = _server.handleCGI(req, loc);
	else
		makeResponse(res, 200, readFile(_server.getErrorPage(200)), getMimeType(req.path));
	return res;
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
	Response	res;

	if (std::find(loc.methods.begin(), loc.methods.end(), "DELETE") == loc.methods.end())
		makeResponse(res, 405, "DELETE not allowed on this location", getMimeType(req.path));
	else if (req.path.empty() || req.path == RED "none")
		makeResponse(res, 400, "Invalid file path", getMimeType(req.path));
	else if (std::remove(req.path.c_str()) != 0)
		makeResponse(res, 404, readFile(_server.getErrorPage(404)), getMimeType(req.path));
	else 
		makeResponse(res, 200, "File deleted successfully", getMimeType(req.path));
	return res;
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