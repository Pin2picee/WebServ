#include "utils.hpp"

std::string strip_semicolon(const std::string &s)
{
	if (!s.empty() && s[s.size() - 1] == ';')
		return s.substr(0, s.size() - 1);
	return s;
};

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

std::string getMimeType(const std::string &path)
{
	if (path.find(".html") != std::string::npos)
		return "text/html";
	if (path.find(".css") != std::string::npos)
		return "text/css";
	if (path.find(".js") != std::string::npos)
		return "application/javascript";
	if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos)
		return "image/jpeg";
	if (path.find(".png") != std::string::npos)
		return "image/png";
	if (path.find(".gif") != std::string::npos)
		return "image/gif";
	return "text/plain";
}
