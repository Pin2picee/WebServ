#ifndef SERVER_HPP
# define SERVER_HPP

# include "Includes.hpp"

class Client;
/**
 * @brief
 * A structure that will register the session infos of each client who generate a cookie.
 * 
 * @param ID				The client session id.
 * @param current_page		The last web page the client connect to.
 * @param uploaded_files	The files the client uploaded.
 * @param expiryTime		When the cookie expires.
 */
struct Session
{
	std::string					ID;
	std::string					current_page;
	std::vector<std::string>	uploaded_files;
	time_t						expiryTime;
};

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
	bool						sensitive;
	bool						autoindex;
	std::string					root;
	std::string					path;
	std::string					upload_dir;
	std::string					cgi_extension;
	std::string					cgi_path;
	std::vector<std::string>	methods;
	std::vector<std::string>	index_files;

	Locations()
		: cgi(false), sensitive(false), autoindex(false),
		  root(""), path(""), upload_dir(""), cgi_extension("") {}
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
 * @param error_dir				The error directory. 
 * @param client_max_body_size	The client max body size.
 */
class Server
{
private:
	std::vector<std::pair<std::string, int> >			listen;
	std::string											root;
	size_t												client_max_body_size;
	std::map<int, std::string>							error_pages;
	std::string											error_dir;
	std::vector<Locations>								locations;
public:
	Server();
	~Server();
	Server &operator=(const Server &assignement);
	Server(const Server &copy);

	// Getters

	const std::string&									getRoot() const;
	const std::vector<std::pair<std::string, int> >&	getListen() const;
	const std::string&									getErrorDir() const;
	const std::vector<Locations>&						getLocations() const;
	const std::string&									getErrorPage(int code, Session &session) const;
	size_t												getClientMaxBodySize() const;

	// Modifiable getters

	std::map<int, std::string>&							getErrorPagesRef();

	// Setters

	void												setRoot(const std::string &r);
	void												setClientMaxBodySize(size_t value);

	// Utils

	void												addLocation(const Locations& loc);
	void												addListen(const std::string& ip, int port);
	void												addErrorDir(const std::string& path);
	void												handleCGI(const Request &req, const Locations &loc, Client *current) const;
};

// Session helpers

Session		&getSession(std::map<std::string, Session> &g_sessions, const Request &req, Response &res, size_t port);
void		removeUploadFileSession(Session &session, std::string deletePath);
void		deleteSession(std::map<std::string, Session> &g_sessions);
Response	parseCGIOutput(const std::string &output);
#endif
