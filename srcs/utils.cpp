#include "utils.hpp"

/**
 * @brief
 * Remove the semicolon at the end of the data string.
 * 
 * @param s The token that contains a semicolon.
 * 
 * @return The string without semicolon.
 */
std::string strip_semicolon(const std::string &s)
{
	if (!s.empty() && s[s.size() - 1] == ';')
		return s.substr(0, s.size() - 1);
	return s;
};

/**
 * @brief
 * Initialise the errors in conf (can be changed by extracting the datas of HTML error files)
 * 
 * @param conf The `Server` configuration.
 */
void	init_default_errors(Server &conf)
{
	std::map<int, std::string> &errors = conf.getErrorPagesRef();
	const std::string &root = conf.getRoot();

	errors[400] = root + "/errors/400.html";
	errors[401] = root + "/errors/401.html";
	errors[403] = root + "/errors/403.html";
	errors[404] = root + "/errors/404.html";
	errors[405] = root + "/errors/405.html";
	errors[413] = root + "/errors/413.html";
	errors[500] = root + "/errors/500.html";
	errors[501] = root + "/errors/501.html";
	errors[502] = root + "/errors/502.html";
	errors[503] = root + "/errors/503.html";
};

/**
 * @param path The request path
 *  
 * @return
 * Return the MIME (Multipurpose Internet Mail Extensions) content type that have `Response` when responding to a `Request`.
 */
std::string getMimeType(const std::string &path)
{
	if (path.find(".html") != std::string::npos)
		return MIME_TEXT_HTML;
	if (path.find(".css") != std::string::npos)
		return MIME_TEXT_CSS;
	if (path.find(".js") != std::string::npos)
		return MIME_TEXT_JAVASCRIPT;
	if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos)
		return MIME_IMAGE_JPEG;
	if (path.find(".png") != std::string::npos)
		return MIME_IMAGE_PNG;
	if (path.find(".gif") != std::string::npos)
		return MIME_IMAGE_GIF;
	return MIME_TEXT_PLAIN;
}
