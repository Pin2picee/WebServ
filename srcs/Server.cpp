#include "Server.hpp"
#include "Config.hpp"

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
 * Get the locations.
 * 
 * @return the locations.
 */
const std::map<int, std::string>& Server::getErrorPages() const
{
	return error_pages;
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

//getters modifiables
/**
 * @brief
 * Get the canals to listen on, the variable can be modifiable.
 * 
 * @return the canals to listen on.
 */
std::vector<std::pair<std::string, int> >&	Server::getListenRef()
{
	return listen;
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
/**
 * @brief
 * Get the locations, the variable can be modifiable.
 * 
 * @return the locations.
 */
std::vector<Locations>&	Server::getLocationsRef()
{
	return locations;
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
 * @param code the error page code.
 * @param path the path that will be assigned to `error_pages`[`code`].
 */
void	Server::addErrorPage(int code, const std::string& path)
{
	error_pages[code] = path;
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
 * Handle CGI.
 * 
 * @param req The corresponding request.
 * @param loc The location struct of the corresponding server.
 */
std::string handleCGI(const Request &req, const Locations &loc)
{
	std::string script_path = loc.root + req.uri.substr(loc.path.size());
	std::string output;

	int pipe_out[2] /* read CGI output */, pipe_in[2] /* send body to CGI if POST */;

	if (pipe(pipe_out) == -1 || pipe(pipe_in) == -1)
		throw std::runtime_error("Pipe creation failed");

	pid_t pid = fork();
	if (pid == -1)
		throw std::runtime_error("Fork failed");

	if (pid == 0) {
		// --- Processus enfant ---
		close(pipe_out[0]); // on n’écoute pas la sortie
		close(pipe_in[1]);  // on n’écrit pas dans le body

		dup2(pipe_out[1], STDOUT_FILENO); // stdout -> pipe_out
		dup2(pipe_in[0], STDIN_FILENO);   // stdin  <- pipe_in
		close(pipe_out[1]);
		close(pipe_in[0]);

		// --- Variables d’environnement CGI ---
		std::vector<std::string> env_vars;

		env_vars.push_back("REQUEST_METHOD=" + req.method);
		env_vars.push_back("SCRIPT_FILENAME=" + script_path);
		env_vars.push_back("QUERY_STRING=" + (req.uri.find('?') != std::string::npos
											   ? req.uri.substr(req.uri.find('?') + 1)
											   : ""));
		env_vars.push_back("SERVER_PROTOCOL=HTTP/1.1");
		env_vars.push_back("CONTENT_LENGTH=" + std::to_string(req.body.size()));
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

		// Conversion en char**
		std::vector<char*> envp;
		for (size_t i = 0; i < env_vars.size(); ++i)
			envp.push_back(const_cast<char*>(env_vars[i].c_str()));
		envp.push_back(NULL);

		// Arguments du CGI
		char *argv[] = {
			const_cast<char*>(loc.cgi_path.c_str()),  // ex: /usr/bin/python3
			const_cast<char*>(script_path.c_str()),   // script à exécuter
			NULL
		};

		execve(loc.cgi_path.c_str(), argv, envp.data());
		exit(1); // si exec échoue
	}
	else {
		// --- Processus parent ---
		close(pipe_out[1]);
		close(pipe_in[0]);

		// Si méthode POST → envoyer le body au CGI
		if (req.method == "POST" && !req.body.empty())
			write(pipe_in[1], req.body.c_str(), req.body.size());
		close(pipe_in[1]);

		// Lire la sortie CGI
		char buffer[4096];
		ssize_t bytes;
		while ((bytes = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
			output.append(buffer, bytes);
		close(pipe_out[0]);

		waitpid(pid, NULL, 0);
	}

	return output;
}
