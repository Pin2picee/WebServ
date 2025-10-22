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
