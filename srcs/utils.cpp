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

	errors[201] = root + "/errors/201.html";
	errors[204] = root + "/errors/204.html";
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

std::vector<Socket *>all_socket;
volatile sig_atomic_t	on = 1;

void	handle_sigint(int signum)
{
	(void)signum;
	on = 0;
	for (std::vector<Socket*>::iterator it = all_socket.begin(); it != all_socket.end(); ++it)
    {
        if (*it)
        {
            close((*it)->getFd());
            delete *it;
        }
    }
}

void fill_tokens(std::vector<std::string> &dest, const std::vector<std::string> &tokens, size_t &i)
{
	dest.clear();
	while (++i < tokens.size() &&
		   tokens[i][tokens[i].size() - 1] != ';')
		dest.push_back(tokens[i]);
	if (i < tokens.size() &&
		tokens[i][tokens[i].size() - 1] == ';')
		dest.push_back(strip_semicolon(tokens[i]));
}

std::string readFile(const std::string& filepath)
{
	struct stat s;
	if (stat(filepath.c_str(), &s) != 0)
		throw std::runtime_error("File not found: " + filepath);
	std::ifstream ifs(filepath.c_str());
	if (!ifs)
		throw std::runtime_error("Cannot open file: " + filepath);

	std::ostringstream buf;
	buf << ifs.rdbuf();
	return buf.str();
}

long long convertSize(const std::string &input)
{
	if (input.empty())
		throw std::invalid_argument("Empty size string");

	std::string str = strip_semicolon(input);
	while (!str.empty() && isspace(str[str.size() - 1]))
		str.erase(str.size() - 1);
	while (!str.empty() && isspace(str[0]))
		str.erase(0, 1);
	if (str.empty())
		throw std::invalid_argument("Invalid size string");

	long long multiplier = 1;
	if (str.size() > 2)
	{
		std::string suffix2 = str.substr(str.size() - 2);
		for (size_t i = 0; i < suffix2.size(); ++i)
		suffix2[i] = std::toupper(suffix2[i]);
		if (suffix2 == "KB" || suffix2 == "MB" || suffix2 == "GB")
		{
			if (suffix2 == "KB")
				multiplier = 1000LL;
			else if (suffix2 == "MB")
				multiplier = 1000LL * 1000LL;
			else if (suffix2 == "GB")
				multiplier = 1000LL * 1000LL * 1000LL;
			str = str.substr(0, str.size() - 2);
		}
	}
	if (str.size() > 1)
	{
		char upper = std::toupper(str[str.size() - 1]);
		if (upper == 'K' || upper == 'M' || upper == 'G')
		{
			if (upper == 'K')
				multiplier = 1024LL;
			else if (upper == 'M')
				multiplier = 1024LL * 1024LL;
			else if (upper == 'G')
				multiplier = 1024LL * 1024LL * 1024LL;
			str = str.substr(0, str.size() - 1);
		}
	}
	for (size_t i = 0; i < str.size(); ++i)
		if (!std::isdigit(str[i]))
			throw std::invalid_argument("Invalid number part in size");
	long long base = atoll(str.c_str());
	return base * multiplier;
}

std::string GetUploadFilename(const std::string &body)
{
	std::string filename;
	std::string content;

	std::istringstream stream(body);
	std::string line;

	if (!std::getline(stream, line))
		return "";
	if (!line.empty())
		stripe(line, '\r', RIGHT);
	std::string boundary = line;
	if (std::getline(stream, line) && !line.empty())
		stripe(line, '\r', RIGHT);
	if (line == boundary + "--")
		return "";
	std::string contentDisposition;
	while (line != "\r" && !line.empty())
	{
		if (!line.empty() && line.find("Content-Disposition:") != std::string::npos)
		{
			contentDisposition = line;
			stripe(contentDisposition, '\r', RIGHT);
			break ;
		}
		std::getline(stream, line);
	}
	size_t fnamePos = contentDisposition.find("filename=\"");
	if (fnamePos != std::string::npos)
	{
		fnamePos += 10;
		size_t endPos = contentDisposition.find("\"", fnamePos);
		if (endPos != std::string::npos)
			filename = contentDisposition.substr(fnamePos, endPos - fnamePos);
	}
	return filename;
}

void displayRequestInfo(const Request &req)
{
	// Affichage des informations simples
	std::cout << RED "Version: " RESET << req.version << std::endl;
	std::cout << RED "Method: " RESET << req.method << std::endl;
	std::cout << RED "URI: " RESET << req.uri << std::endl;
	std::cout << RED "Path: " RESET << req.path << std::endl;

	// Affichage des en-têtes (headers)
	std::cout << RED "Headers:" RESET << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it)
	{
		std::cout << "  " CYAN << it->first << ": " RESET << it->second << std::endl;
	}

	// Affichage du corps de la requête (body)
	std::cout << RED "Body: " RESET << std::endl;
	std::cout << req.body << std::endl;
}

std::string getFileName(const std::string &fileBody)
{
	std::size_t fileStart = fileBody.find("filename=\"");
	if (fileStart != std::string::npos)
	{
		fileStart += 10;
		size_t endPos = fileBody.find("\"", fileStart);
		if (endPos != std::string::npos)
			return fileBody.substr(fileStart, endPos - fileStart);
	}
	return "";
}

std::string makeJsonError(const std::string &msg)
{
	return std::string("{\"status\":\"error\",\"message\":\"") + msg + "\"}";
}

std::string urlDecode(const std::string &str)
{
    std::string result;
    for (std::string::size_type i = 0; i < str.length(); ++i)
    {
        if (str[i] == '%')
        {
            if (i + 2 < str.length())
            {
                std::string hexStr = str.substr(i + 1, 2);
                char ch = static_cast<char>(std::strtol(hexStr.c_str(), NULL, 16));
                result += ch;
                i += 2;
            }
            else
                result += '%';
        }
        else if (str[i] == '+')
            result += ' ';
        else
            result += str[i];
    }
    return result;
}


