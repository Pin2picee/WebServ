#ifndef UTILS_HPP
# define UTILS_HPP

# include "ConfigParser.hpp"

std::string strip_semicolon(const std::string &s);
void        init_default_errors(ServerConf &conf);

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
