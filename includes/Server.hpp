#ifndef SERVER_HPP
# define SERVER_HPP

# include "Includes.hpp"

struct Locations
{
	bool						cgi;
	bool						autoindex;
	std::string					root;
	std::string					path;
	std::string					upload_dir;
	std::string					cgi_extension;
	std::vector<std::string>	methods;
	std::vector<std::string>	index_files;

	Locations()
		: cgi(false), autoindex(false),
		  root(RED "none"), path(RED "none"),
		  upload_dir(RED "none"), cgi_extension(RED "none") {}
};

class Server
{
private:
	std::string											root;
	std::vector<std::pair<std::string, int> >			listen;
	std::vector<Locations>								locations;
	std::map<int, std::string>							error_pages;
	size_t												client_max_body_size;

public:
	Server();
	~Server();

	// Getters
	const std::string&									getRoot() const;
	const std::vector<std::pair<std::string, int> >&	getListen() const;
	const std::vector<Locations>&						getLocations() const;
	const std::map<int, std::string>&					getErrorPages() const;
	size_t												getClientMaxBodySize() const;

	// Modifiable getters
	std::vector<std::pair<std::string, int> >&			getListenRef();
	std::vector<Locations>&								getLocationsRef();
	std::map<int, std::string>&							getErrorPagesRef();

	// Setters
	void 												setRoot(const std::string &r);
	void 												setClientMaxBodySize(size_t value);
	void 												setErrorPage(int code, const std::string &path);

	// Utils
	void 												addLocation(const Locations& loc);
	void 												addListen(const std::string& ip, int port);
	void 												addErrorPage(int code, const std::string& path);
};

#endif
