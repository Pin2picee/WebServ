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
	return MIME_TEXT_HTML;
}

std::vector<Socket *>all_socket;
volatile sig_atomic_t	on = 1;

void	handle_sigint(int signum)
{
	(void)signum;
	for (std::vector<Socket * >::iterator it = all_socket.begin(); it != all_socket.end(); it++)
	{
		if (*it)
		{
			close((*it)->getFd());
			delete((*it));
		}
	}
	on = 0;
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

std::string parseMultipartFormData(const std::string &body, std::string &filename)
{
	std::string content;

	std::istringstream stream(body);
	std::string line;

	if (!std::getline(stream, line))
		return "";
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
	std::string boundary = line;
	if (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line == boundary + "--")
			return "";
		std::string contentDisposition;
		while (std::getline(stream, line) && line != "\r" && !line.empty())
		{
			if (!line.empty() && line.find("Content-Disposition:") != std::string::npos)
				contentDisposition = line;
			if (contentDisposition[contentDisposition.size() - 1] == '\r')
				contentDisposition.erase(contentDisposition.size() - 1);
		}
		size_t fnamePos = contentDisposition.find("filename=\"");
		if (fnamePos != std::string::npos)
		{
			fnamePos += 10;
			size_t endPos = contentDisposition.find("\"", fnamePos);
			if (endPos != std::string::npos)
				filename = contentDisposition.substr(fnamePos, endPos - fnamePos);
		}
		if (filename.empty())
			return "";
		std::ostringstream contentStream;
		while (std::getline(stream, line))
		{
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);
			if (line == boundary || line == boundary + "--")
				break;
			contentStream << line << "\n";
		}
		content = contentStream.str();
		if (!content.empty() && content[content.size() - 1] == '\n')
			content.erase(content.size() - 1);
	}
	return content;
}
