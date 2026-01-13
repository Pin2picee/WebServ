#include "Server.hpp"
#include "Config.hpp"
#include "ResponseHandler.hpp"
#include "Client.hpp"
/* Constructor */
Server::Server() : client_max_body_size(0) {}

/* Destructor */
Server::~Server() {}

//Getters
/**
 * @brief
 * Get the root path.
 * 
 * @return the root path.
 */
const std::string&	Server::getRoot() const
{
	return root;
}

/**
 * @brief
 * Get the error_page you want.
 * 
 * @param code the error code to find.
 * 
 * @return the corresponding error_page.
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
 * Get the canals to listen on.
 * 
 * @return the canals to listen on.
 */
const std::vector<std::pair<std::string, int> >&	Server::getListen() const
{
	return listen;
}

/**
 * @brief
 * Get the max client body size.
 * 
 * @return the max client body size.
 */
size_t	Server::getClientMaxBodySize() const
{
	return client_max_body_size;
}

/**
 * @brief
 * Get the locations.
 * 
 * @return the locations.
 */
const std::vector<Locations>&	Server::getLocations() const
{
	return locations;
}

/**
 * @brief
 * Get the error dir.
 * 
 * @return the error dir.
 */
const std::string&	Server::getErrorDir() const
{
	return error_dir;
}


/**
 * @brief
 * Get the error pages, the variable can be modifiable.
 * 
 * @return the error pages.
 */
std::map<int, std::string>&	Server::getErrorPagesRef()
{
	return error_pages;
}

//setters
/**
 * @brief
 * Set the root path.
 * 
 * @param r the path that will be assigned to `root`.
 */
void	Server::setRoot(const std::string &r)
{
	root = r;
}

/**
 * @brief
 * Set the client max body size file.
 * 
 * @param value the client max body size file value.
 * 
 * @note If the value is negative, it will be converted in a positive size_t value.
 */
void	Server::setClientMaxBodySize(size_t value)
{
	client_max_body_size = value;
}

//useful methods
/**
 * @brief
 * Add a canal to listen on to `listen`.
 * 
 * @param ip The canal ip address.
 * @param port The canal port.
 */
void	Server::addListen(const std::string& ip, int port)
{
	listen.push_back(std::make_pair(ip, port));
}

/**
 * @brief
 * Add an error page.
 * 
 * @param dir the error directory path after server root path.
 */
void	Server::addErrorDir(const std::string& dir)
{
	error_dir = dir;
}

/**
 * @brief
 * Add a location.
 * 
 * @param loc The locations that will be added to `locations`.
 */
void	Server::addLocation(const Locations& loc)
{
	locations.push_back(loc);
}

/**
 * @brief
 * Transform the output of handleCGI in Response struct.
 * 
 * @param output The output of handleCGI.
 * 
 * @return a Response struct.
 */
Response parseCGIOutput(const std::string &output)
{
	Response res;
	res.status_code = 200; // default value
	res.content_type = "text/html"; // default value

	// Separate headers from body
	size_t header_end = output.find("\r\n\r\n");
	if (header_end == std::string::npos)
		header_end = output.find("\n\n"); // case where the script only use \n

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
 * Handle the server's CGI according to the request given.
 * 
 * @param req The corresponding request.
 * @param loc The location struct of the corresponding server.
 * 
 * @return A Response struct.
 */
void Server::handleCGI(const Request &req, const Locations &loc, Client *current) const
{
	std::string script_path = this->root + req.path + req.uri.substr(req.path.size());
	std::string output;
	int pipe_out[2] /* read CGI output */, pipe_in[2] /* send body to CGI if POST */;
	if (pipe(pipe_out) == -1 || pipe(pipe_in) == -1)
		throw std::runtime_error("Pipe creation failed");
	pid_t pid = fork();
	if (pid == -1)
		throw std::runtime_error("Fork failed");
	if (!pid)
	{
		// --- Child process ---
		close(pipe_out[0]);
		close(pipe_in[1]);

		signal(SIGINT, SIG_IGN);   // ← Ignore les signaux 
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_in[0], STDIN_FILENO);
		close(pipe_out[1]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_in[0]);
		std::vector<std::string> env_vars;

		env_vars.push_back("REQUEST_METHOD=" + req.method);
		env_vars.push_back("SCRIPT_FILENAME=" + script_path);
		env_vars.push_back("QUERY_STRING=" + (req.uri.find('?') != std::string::npos
											   ? req.uri.substr(req.uri.find('?') + 1)
											   : ""));
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
		// CGI arguments
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

		current->setPipeIn(pipe_out[0]);//Lis la sortie du CGI
		current->setPipeOut(pipe_in[1]);//ecrit dans l'entre du CGI
		current->setBody(req.body);
		current->setCgiPid(pid);
		current->setCGiStartTime();//demarrage timing CGI
		current->setInCGI();//on le mets a true
	}
}

Server::Server(const Server &copy)
{
	if (this != &copy)
		*this = copy;
}

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

std::map<std::string, Session> g_sessions;

Session &getSession(std::map<std::string, Session> &g_sessions, const Request &req, Response &res)
{
	std::map<std::string, std::string>::const_iterator it = req.cookies.find("User");
	if (it == req.cookies.end())
	{
		std::string id = generateSessionId();
		Session &newSession = g_sessions[id];
		newSession.ID = id;
		newSession.expiryTime = getCurrentTime() + setCookie(id, res, "User", req.cookies);
		return newSession;
	}
	std::string sessionId = it->second;
	setCookie(sessionId, res, "User", req.cookies);
	g_sessions[it->second].ID = sessionId;
	return g_sessions[it->second];
}

void	deleteSession(void)
{
	time_t now = getCurrentTime();
	for (std::map<std::string, Session>::iterator it = g_sessions.begin();
		it != g_sessions.end();)
	{
		if (it->second.expiryTime <= now)
    	{
			std::map<std::string, Session>::iterator toDelete = it;
            g_sessions.erase(toDelete);
		}
		++it;
	}
}

void removeUploadFileSession(Session &session, std::string deletePath)
{
	std::vector<std::string>::iterator it =
	std::find(session.uploaded_files.begin(), session.uploaded_files.end(), deletePath);

	if (it != session.uploaded_files.end())
		session.uploaded_files.erase(it);
}

