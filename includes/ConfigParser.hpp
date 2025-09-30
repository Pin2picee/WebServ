#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <vector>
# include <map>
# include <fstream>
# include <sstream>
# include <stdexcept>
# include "Webserv.hpp"
# include <sstream>

# define RED     "\033[31m"

template<typename T>
std::string ft_to_string(T value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

struct LocationConf
{
	bool						cgi;			// Whether CGI scripts are enabled
	bool						autoindex;		// Whether directory listing is enabled
	std::string					root;			// Root directory for this location
	std::string					path;			// URI path of this location (e.g., "/upload")
	std::string					upload_dir;		// Directory where uploaded files are stored
	std::string					cgi_extension;	// File extension for CGI scripts (e.g., ".py")
	std::vector<std::string>	methods;		// Allowed HTTP methods (GET, POST, DELETE, etc.)
	std::vector<std::string>	index_files;	// Default index files for this location (e.g., "index.html")
	
	LocationConf() : cgi(false), autoindex(false),
					 root(RED "none"), path(RED "none"),
					 upload_dir(RED "none"), cgi_extension(RED "none") {};
};


struct ServerConf
{
	std::vector<std::pair<std::string, int> >	listen;					// IPs and ports the server listens on
	std::string									root;					// Server root directory for static files
	std::map<int, std::string>					error_pages;			// Custom HTML error pages, key = HTTP code, value = file path
	size_t										client_max_body_size;	// Max allowed size of request body (uploads, POST data)
	std::vector<LocationConf>					locations;				// List of locations with specific rules (cf LocationConf class)
};

class ConfigParser
{
private:
	std::string					_file;
	std::vector<std::string>	tokenize(std::istream &ifs);

public:
	ConfigParser(const std::string &file);
	ServerConf	 parse();
};

struct Request
{
	std::string							method;		// HTTP method, POST DELETE
	std::string							uri;		// Client path to the resource (as sent in the HTTP request)
	std::string							path;		// Full local path on the server (uri + location root)
	std::string							body;		// Request content (payload), uploaded file or form data
	std::map<std::string, std::string>	headers;	// HTTP headers, extra infos for handling the request

	Request() {};
};

//=== MIME (Multipurpose Internet Mail Extensions) content types ===//
// Text types
# define MIME_TEXT_PLAIN       "text/plain"
# define MIME_TEXT_HTML        "text/html"
# define MIME_TEXT_CSS         "text/css"
# define MIME_TEXT_JAVASCRIPT  "application/javascript"

// Data / JSON
# define MIME_APPLICATION_JSON "application/json"
# define MIME_APPLICATION_XML  "application/xml"

// Images
# define MIME_IMAGE_PNG        "image/png"
# define MIME_IMAGE_JPEG       "image/jpeg"
# define MIME_IMAGE_GIF        "image/gif"
# define MIME_IMAGE_SVG        "image/svg+xml"

// Fonts
# define MIME_FONT_WOFF        "font/woff"
# define MIME_FONT_WOFF2       "font/woff2"
# define MIME_FONT_TTF         "font/ttf"

// Binary / Misc
# define MIME_APPLICATION_OCTET_STREAM "application/octet-stream"

struct Response
{
	int			status_code;	// HTTP status code, 200, 404, 500
	std::string	content_type;	// MIME content type ("text/html", "application/json", …)
	std::string	body;			// response content (HTML, text, JSON, binary file, …)

	Response() {};
};

Response handlePost(const LocationConf &loc, const Request &req, const ServerConf &conf);
Response handleDelete(const LocationConf &loc, const Request &req);

#endif
