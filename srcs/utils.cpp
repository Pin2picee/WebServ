#include "utils.hpp"

std::string strip_semicolon(const std::string &s)
{
    if (!s.empty() && s[s.size() - 1] == ';')
        return s.substr(0, s.size() - 1);
    return s;
};

void	init_default_errors(ServerConf &conf)
{
	conf.error_pages[400] = conf.root + "/errors/400.html";
	conf.error_pages[401] = conf.root + "/errors/401.html";
	conf.error_pages[403] = conf.root + "/errors/403.html";
	conf.error_pages[404] = conf.root + "/errors/404.html";
	conf.error_pages[405] = conf.root + "/errors/405.html";
	conf.error_pages[413] = conf.root + "/errors/413.html";
	conf.error_pages[500] = conf.root + "/errors/500.html";
	conf.error_pages[501] = conf.root + "/errors/501.html";
	conf.error_pages[502] = conf.root + "/errors/502.html";
	conf.error_pages[503] = conf.root + "/errors/503.html";
};
