#include "Server.hpp"
#include "Config.hpp"
#include "ResponseHandler.hpp"
#include "Client.hpp"

/**
 * @brief
 * Default constructor for Server.
 */
Server::Server() : client_max_body_size(0) {}

/**
 * @brief
 * Destructor for Server.
 */
Server::~Server() {}

/**
 * @brief
 * Get the root path of the server.
 *
 * @return The root path string.
 */
const std::string&	Server::getRoot() const
{
	return root;
}

/**
 * @brief
 * Get the error page for a specific error code.
 *
 * @param code The HTTP error code.
 * @param session The current session (will be updated with current_page).
 *
 * @return The error page content string. Returns a default HTML if not found.
 */
const std::string& Server::getErrorPage(int code, Session &session) const
{
	std::map<int, std::string>::const_iterator it = error_pages.find(code);
   	if (it != error_pages.end())
	{
		session.current_page = it->second;
		return it->second;
	}
	else
	{
		static std::string default_page = "<html><body><h1>Error</h1></body></html>";
		return default_page;
	}
}


/**
 * @brief
 * Get the list of IP and port pairs (canals) the server listens on.
 *
 * @return Vector of (IP, port) pairs.
 */
const std::vector<std::pair<std::string, int> >&	Server::getListen() const
{
	return listen;
}

/**
 * @brief
 * Get the maximum allowed client body size.
 *
 * @return Maximum body size in bytes.
 */
size_t	Server::getClientMaxBodySize() const
{
	return client_max_body_size;
}

/**
 * @brief
 * Get the list of configured locations for this server.
 *
 * @return Vector of Locations.
 */
const std::vector<Locations>&	Server::getLocations() const
{
	return locations;
}

/**
 * @brief
 * Get the error directory path relative to the server root.
 *
 * @return Error directory path.
 */
const std::string&	Server::getErrorDir() const
{
	return error_dir;
}


/**
 * @brief
 * Get the map of error pages (modifiable reference).
 *
 * @return Map of error codes to error page paths.
 */
std::map<int, std::string>&	Server::getErrorPagesRef()
{
	return error_pages;
}

/**
 * @brief
 * Set the root path of the server.
 *
 * @param r The root path string.
 */
void	Server::setRoot(const std::string &r)
{
	root = r;
}

/**
 * @brief
 * Set the maximum client body size.
 *
 * @param value Maximum body size in bytes.
 *
 * @note If value is negative, it will be converted to a positive size_t.
 */
void	Server::setClientMaxBodySize(size_t value)
{
	client_max_body_size = value;
}

/**
 * @brief
 * Add a listening canal (IP + port) to the server.
 *
 * @param ip The IP address.
 * @param port The port number.
 */
void	Server::addListen(const std::string& ip, int port)
{
	listen.push_back(std::make_pair(ip, port));
}

/**
 * @brief
 * Set the error directory for this server.
 *
 * @param dir Directory path relative to the server root.
 */
void	Server::addErrorDir(const std::string& dir)
{
	error_dir = dir;
}

/**
 * @brief
 * Add a location configuration to the server.
 *
 * @param loc The Locations struct to add.
 */
void	Server::addLocation(const Locations& loc)
{
	locations.push_back(loc);
}

/**
 * @brief
 * Parse the output of a CGI script into a Response object.
 *
 * @param output The raw CGI output string.
 *
 * @return A Response struct containing headers and body.
 */
Response parseCGIOutput(const std::string &output)
{
	Response res;
	res.status_code = 200;
	res.content_type = "text/html";

	size_t header_end = output.find("\r\n\r\n");
	if (header_end == std::string::npos)
		header_end = output.find("\n\n");

	std::string header_part = output.substr(0, header_end);
	std::string body_part;
	if (header_end != std::string::npos)
	{
		if (!output.compare(header_end, 4, "\r\n\r\n"))
			body_part = output.substr(header_end + 4);
		else
			body_part = output.substr(header_end + 2);
	}

	std::istringstream headers(header_part);
	std::string line;
	while (std::getline(headers, line))
	{
		if (!line.find("Content-Type:", 0))
		{
			res.content_type = line.substr(13);
			res.content_type.erase(0, res.content_type.find_first_not_of(" \t"));
		}
		else if (!line.find("Status:", 0))
		{
			std::string status_str = line.substr(7);
			res.status_code = std::atoi(status_str.c_str());
		}
	}

	res.body = body_part;
	return res;
}

/**
 * @brief
 * Execute a CGI script for a given request and location, and prepare the Client object.
 *
 * @param req The incoming Request.
 * @param loc The corresponding Locations configuration.
 * @param current Pointer to the Client object handling this request.
 *
 * @throws std::runtime_error on pipe creation, fork, or unsupported CGI extension.
 */
void Server::handleCGI(const Request &req, const Locations &loc, Client *current) const
{
	std::string script_path = this->root + req.path;
	std::string output;
	int pipe_out[2], pipe_in[2];
	if (pipe(pipe_out) == -1 || pipe(pipe_in) == -1)
		throw std::runtime_error("Pipe creation failed");
	pid_t pid = fork();
	if (pid == -1)
		throw std::runtime_error("Fork failed");
	if (!pid)
	{
		close(pipe_out[0]);
		close(pipe_in[1]);

		signal(SIGPIPE, SIG_IGN);
		signal(SIGINT, SIG_IGN); 
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_in[0], STDIN_FILENO);
		close(pipe_out[1]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_in[0]);
		std::vector<std::string> env_vars;

		env_vars.push_back("REQUEST_METHOD=" + req.method);
		env_vars.push_back("SCRIPT_FILENAME=" + script_path);
		env_vars.push_back("QUERY_STRING=" + req.query);
		env_vars.push_back("SERVER_PROTOCOL=HTTP/1.1");
		std::ostringstream oss;
		oss << req.body.size();
		env_vars.push_back("CONTENT_LENGTH=" + oss.str());
		env_vars.push_back("CONTENT_TYPE=" + (req.headers.count("Content-Type")
												? req.headers.at("Content-Type")
												: "text/plain"));
		env_vars.push_back("GATEWAY_INTERFACE=CGI/1.1");
		for (std::map<std::string, std::string>::const_iterator it = req.headers.begin();
			 it != req.headers.end(); ++it)
		{
			std::string key = "HTTP_" + it->first;
			for (size_t i = 0; i < key.size(); ++i)
				if (key[i] == '-') key[i] = '_';
			env_vars.push_back(key + "=" + it->second);
		}
		std::vector<char*> envp;
		for (size_t i = 0; i < env_vars.size(); ++i)
			envp.push_back(const_cast<char*>(env_vars[i].c_str()));
		envp.push_back(NULL);
		std::string cgi_path;

		if (loc.cgi_extension == ".py")
			cgi_path = "/usr/bin/python3";
		else if (loc.cgi_extension == ".php")
			cgi_path = "/usr/bin/php-cgi";
		else
			throw std::runtime_error("Unsupported CGI extension");
		char *argv[] = {
			const_cast<char*>(cgi_path.c_str()),
			const_cast<char*>(script_path.c_str()),
			NULL
		};
		for (int fd = 3; fd < 1024; fd++)
        	close(fd);
		execve(cgi_path.c_str(), argv, envp.data());
		exit(1);
	}
	else
	{
		close(pipe_out[1]);
		close(pipe_in[0]);

		current->setPipeIn(pipe_out[0]);
		current->setPipeOut(pipe_in[1]);
		current->setBody(req.body);
		current->setCgiPid(pid);
		current->setCGiStartTime();
		current->setInCGI();
	}
}

/**
 * @brief
 * Copy constructor for Server.
 *
 * @param copy The Server object to copy.
 */
Server::Server(const Server &copy)
{
	if (this != &copy)
		*this = copy;
}

/**
 * @brief
 * Assignment operator for Server.
 *
 * @param assignement The Server object to assign from.
 *
 * @return Reference to this Server object.
 */
Server &Server::operator=(const Server &assignement)
{
	if (this != &assignement)
	{
		root = assignement.root;
		listen = assignement.listen;
		locations = assignement.locations;
		error_pages = assignement.error_pages;
		client_max_body_size = assignement.client_max_body_size;
	}
	return (*this);
}

/**
 * @brief
 * Retrieve or create a Session object for a given request and port.
 *
 * @param g_sessions Map of all sessions.
 * @param req The incoming Request.
 * @param res The Response to set cookies if needed.
 * @param port The server port.
 *
 * @return Reference to the Session object for this request.
 */
Session &getSession(std::map<std::string, Session> &g_sessions, const Request &req, Response &res, size_t port)
{
    std::string cookie_name = "User_" + ft_to_string(port);
    std::map<std::string, std::string>::const_iterator it = req.cookies.find(cookie_name);
    std::map<std::string, Session>::iterator sess_it = g_sessions.end();
    std::string session_key;
    if (it != req.cookies.end())
    {
        session_key = ft_to_string(port) +  "_" + it->second;
        sess_it = g_sessions.find(session_key);
    }
    if (it == req.cookies.end() || sess_it == g_sessions.end())
    {
        std::string id = generateSessionId();
        session_key = ft_to_string(port) + "_" + id;
        g_sessions[session_key].ID = id;
       	g_sessions[session_key].expiryTime = getCurrentTime() + setCookie(id, res, cookie_name, req.cookies);
        return g_sessions[session_key];
    }
    return g_sessions[session_key];
}

/**
 * @brief
 * Delete expired sessions and remove associated upload directories.
 *
 * @param g_sessions Map of all sessions.
 */
void	deleteSession(std::map<std::string, Session> &g_sessions)
{
	time_t now = getCurrentTime();
	for (std::map<std::string, Session>::iterator it = g_sessions.begin();
		it != g_sessions.end(); )
	{
		if (it->second.expiryTime <= now)
		{
			removeDirectoryRecursive("./config/www/uploads/" + it->second.ID);			
			std::map<std::string, Session>::iterator toDelete = it;
			++it;
			g_sessions.erase(toDelete);
		}
		else
			++it;
	}
}

/**
 * @brief
 * Remove a specific uploaded file from a Session.
 *
 * @param session The Session object.
 * @param deletePath Path of the file to remove from uploaded_files.
 */
void removeUploadFileSession(Session &session, std::string deletePath)
{
	std::vector<std::string>::iterator it =
	std::find(session.uploaded_files.begin(), session.uploaded_files.end(), deletePath);

	if (it != session.uploaded_files.end())
		session.uploaded_files.erase(it);
}
