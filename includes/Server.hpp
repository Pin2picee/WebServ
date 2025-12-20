#ifndef SERVER_HPP
# define SERVER_HPP

# include "Includes.hpp"

class Client;
/**
 * @brief
 * The structure that contains all the locations datas.
 * 
 * @param cgi			`true` if the cgi it's on, `false` if it's off.
 * @param autoindex		`true` if autoindex it's on, `false` if it's off.
 * @param root			The `Location` root path.
 * @param path			The `Location` extra path for datas. 
 * @param upload_dir	The directory where uploads are stored.
 * @param cgi_extension	The CGI file extension handled by this location.
 * @param methods		The list of allowed HTTP methods (e.g., GET, POST, DELETE).
 * @param index_files	The list of index files for this location.
 */
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

struct Response;
struct Request;

/**
 * @brief
 * The class that contains all the `Server` datas.
 * 
 * @param root					The server root path.
 * @param listen				The canals to listen on.
 * @param locations				The `Location` datas.
 * @param error_pages			The error pages. 
 * @param client_max_body_size	The client max body size.
 */
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
	Server &operator=(const Server &assignement);
	Server(const Server &copy);
	// Getters

	const std::string&									getRoot() const;
	const std::vector<std::pair<std::string, int> >&	getListen() const;
	const std::vector<Locations>&						getLocations() const;
	const std::string&									getErrorPage(int code) const;
	const std::map<int, std::string>&					getErrorPages() const;
	size_t												getClientMaxBodySize() const;

	// Modifiable getters

	std::vector<std::pair<std::string, int> >&			getListenRef();
	std::vector<Locations>&								getLocationsRef();
	std::map<int, std::string>&							getErrorPagesRef();

	// Setters

	void 												setRoot(const std::string &r);
	void 												setClientMaxBodySize(size_t value);

	// Utils

	void 												addLocation(const Locations& loc);
	void 												addListen(const std::string& ip, int port);
	void 												addErrorPage(int code, const std::string& path);
	Response 											handleCGI(const Request &req, const Locations &loc, Client *current) const;
};

Response parseCGIOutput(const std::string &output);
#endif
