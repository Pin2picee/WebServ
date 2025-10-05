#ifndef UTILS_HPP
# define UTILS_HPP

# include "ConfigParser.hpp"

std::string	strip_semicolon(const std::string &s);
void		init_default_errors(ServerConf &conf);
inline void makeResponse(Response &res, int status, const std::string &body, const std::string &content_type = MIME_TEXT_PLAIN)
{
	res.status_code = status;
	res.body = body;
	res.content_type = content_type;
}

// Text colors
# define RESET   "\033[0m"
# define RED     "\033[31m"
# define GREEN   "\033[32m"
# define YELLOW  "\033[33m"
# define MAGENTA "\033[35m"
# define CYAN    "\033[36m"

// Text styles (optionnel)
# define BOLD       "\033[1m"
# define UNDERLINE  "\033[4m"

#endif
