#include "Server.hpp"
#include "Config.hpp"

Server::Server() : client_max_body_size(0) {}

Server::~Server() {}

//getters
const std::string&	Server::getRoot() const
{
	return root;
}

const std::map<int, std::string>& Server::getErrorPages() const
{
	return error_pages;
}

const std::vector<std::pair<std::string, int> >&	Server::getListen() const
{
	return listen;
}

size_t	Server::getClientMaxBodySize() const
{
	return client_max_body_size;
}

const std::vector<Locations>&	Server::getLocations() const
{
	return locations;
}

//getters modifiables
std::vector<std::pair<std::string, int> >&	Server::getListenRef()
{
	return listen;
}

std::map<int, std::string>&	Server::getErrorPagesRef()
{
	return error_pages;
}

std::vector<Locations>&	Server::getLocationsRef()
{
	return locations;
}

//setters
void	Server::setRoot(const std::string &r)
{
	root = r;
}

void	Server::setErrorPage(int code, const std::string &path)
{
	error_pages[code] = path;
}

void	Server::setClientMaxBodySize(size_t value)
{
	client_max_body_size = value;
}

//useful methods
void	Server::addListen(const std::string& ip, int port)
{
	listen.push_back(std::make_pair(ip, port));
}

void	Server::addErrorPage(int code, const std::string& path)
{
	error_pages[code] = path;
}

void	Server::addLocation(const Locations& loc)
{
	locations.push_back(loc);
}
