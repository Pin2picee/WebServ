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
	errors[409] = root + "/errors/409.html";
	errors[413] = root + "/errors/413.html";
	errors[418] = root + "/errors/418.html";
	errors[500] = root + "/errors/500.html";
	errors[501] = root + "/errors/501.html";
	errors[502] = root + "/errors/502.html";
	errors[503] = root + "/errors/503.html";
}


void resetUploadsDir(const std::string &uploadsPath)
{
	std::string rmCmd = "rm -rf " + uploadsPath;
	if (system(rmCmd.c_str()) != 0)
		std::cerr << "Failed to reset " << uploadsPath << std::endl;
	if (mkdir(uploadsPath.c_str(), 0755) == -1)
		std::cerr << "Failed to recreate " << uploadsPath << std::endl;
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
	resetUploadsDir("./config/www/uploads");
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
	print("displayRequestInfo :");
	// Affichage des informations simples
    std::cout << RED "Version: " RESET << req.version << std::endl;
	std::cout << RED "Method: " RESET << req.method << std::endl;
	std::cout << RED "URI: " RESET << req.uri << std::endl;
	std::cout << RED "Path: " RESET << req.path << std::endl;
	// Affichage des en-têtes (headers)
	std::cout << RED "Headers:" RESET << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it)
		std::cout << "  " CYAN << it->first << ": " RESET << it->second << std::endl;
	// Affichage des cookies
	if (!req.cookies.empty())
	{
		std::cout << RED "Cookies:" RESET << std::endl;
		for (std::map<std::string, std::string>::const_iterator it = req.cookies.begin(); it != req.cookies.end(); ++it)
			std::cout << "  " MAGENTA << it->first << ": " RESET << it->second << std::endl;
	}
	return ;
	// Affichage du corps de la requête (body)
	std::cout << RED "Body: " RESET << std::endl;
	std::cout << req.body << std::endl;
}

void displayResponseInfo(const Response &res)
{
	print("displayResponseInfo :");
	// Affichage des informations principales
	std::cout << RED "Version: " RESET << res.version << std::endl;
	std::cout << RED "Status Code: " RESET << res.status_code << std::endl;
	std::cout << RED "Content-Type: " RESET << res.content_type << std::endl;

	// Affichage des headers
	if (!res.headers.empty())
	{
		std::cout << RED "Headers:" RESET << std::endl;
		for (std::vector<std::string>::const_iterator it = res.headers.begin(); it != res.headers.end(); ++it)
			std::cout << "  " CYAN << *it << RESET << std::endl;
	}

	// Affichage du corps de la réponse
	std::cout << RED "Body: " RESET << std::endl;
	std::cout << res.body << std::endl;
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

void parseCookies(Request &req)
{
	std::map<std::string, std::string>::iterator it = req.headers.find("Cookie");
	if (it == req.headers.end())
		return;

	std::string raw = it->second;
	std::stringstream ss(raw);
	std::string part;

	while (std::getline(ss, part, ';'))
	{
		size_t eq = part.find('=');
		if (eq != std::string::npos)
		{
			std::string key = part.substr(0, eq);
			std::string value = part.substr(eq + 1);
			stripe(key, " \t");
			stripe(value, " \t");
			req.cookies[key] = value;
		}
	}
}

std::string ft_to_string(int nb)
{
	std::stringstream ss;
	ss << nb;
	return ss.str();
}

std::string	generateSessionId(void)
{
	static int ID = 0;
	return "sess" + ft_to_string(ID++);
}

int setCookie(std::string &id, Response &res, const std::string &name, const std::map<std::string, std::string> &cookies,
				int maxAgeSeconds = 3600, const std::string &path = "/", bool httpOnly = true, bool secure = true)
{
	std::string value;
	std::map<std::string, std::string>::const_iterator it = cookies.find(name);
	if (it != cookies.end())
		value = it->second;
	else
		value = id;
	std::string cookie = name + "=" + value;
	cookie += "; Path=" + path;
	if (maxAgeSeconds > 0)
		cookie += "; Max-Age=" + ft_to_string(maxAgeSeconds);
	if (httpOnly)
		cookie += "; HttpOnly";
	cookie += "; SameSite=Lax";
	(void)secure;
	/* if (secure)
		cookie += "; Secure"; */
	res.headers.push_back(cookie);
	return maxAgeSeconds;
}

int setCookie(std::string &id, Response &res, const std::string &name, const std::map<std::string, std::string> &cookies)
{
	return setCookie(id, res, name, cookies, 3600, "/", true, true);
}

void print(const std::string msg)
{
	std::cout << "-------" << msg << std::endl;
}

time_t getCurrentTime()
{
	return std::time(NULL);
}

bool canDisplayFile(const std::string mime)
{
	return	mime == MIME_TEXT_HTML || mime == MIME_TEXT_PLAIN || mime == MIME_TEXT_CSS ||
			mime == MIME_TEXT_JAVASCRIPT || mime == MIME_IMAGE_PNG || mime == MIME_IMAGE_JPEG ||
			mime == MIME_IMAGE_GIF || mime == MIME_IMAGE_SVG || mime == MIME_IMAGE_WEBP ||
			mime == MIME_IMAGE_BMP;
}

bool pathExists(const std::string &path)
{
	struct stat info;
	return stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
}

void addAutoindexButton(const std::string &targetDir)
{
	std::string indexPath = "config/www/";
	if (!pathExists(indexPath + targetDir))
		return;
	const char* indexPaths[] = { "index.html", "upload.html", "delete_file.html" };
	for (int i = 0; i < 3; ++i)
	{
		indexPath = "config/www/";
		indexPath += indexPaths[i];
		std::ifstream file(indexPath.c_str());
		if (!file)
		{
			std::cerr << "Can't open " << indexPath << std::endl;
			continue;
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string html = buffer.str();
		file.close();
		std::string buttonHtml = "  <a href=\"/uploads?dir=" + targetDir + "\" class=\"button\">Watch autoindex</a>\n";
		if (html.find(buttonHtml) != std::string::npos)
            return ;
		size_t pos = html.find("</body>");
		if (pos != std::string::npos)
			html.insert(pos, buttonHtml);
		std::ofstream outFile(indexPath.c_str());
		if (outFile)
			outFile << html;
	}
}

void removeAutoindexButton()
{
	const char* indexPaths[] = { "index.html", "upload.html", "delete_file.html" };
	for (int i = 0; i < 3; i++)
	{
		std::string indexPath = "config/www/";
		indexPath += indexPaths[i];

		std::ifstream file(indexPath.c_str());
		if (!file)
		{
			std::cerr << "Can't open " << indexPath << std::endl;
			continue;
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string html = buffer.str();
		file.close();

		std::string startTag = "<a href=\"/autoindex?";
		size_t pos = html.find(startTag);
		while (pos != std::string::npos)
		{
			size_t endPos = html.find("</a>", pos);
			if (endPos != std::string::npos)
			{
				endPos += 5; // longueur de "</a>\n"
				html.erase(pos, endPos - pos);
			}
			pos = html.find(startTag, pos);
		}
		std::ofstream outFile(indexPath.c_str());
		if (outFile)
			outFile << html;
	}
}

std::string cleanPath(const std::string &path)
{
    if (path.empty())
		return "";
    std::string clean;
    clean.reserve(path.size());
    bool lastWasSlash = false;
    for (size_t i = 0; i < path.size(); ++i)
	{
        char c = path[i];
        if (c == '/')
		{
            if (!lastWasSlash)
			{
                clean += c;
                lastWasSlash = true;
            }
        }
		else
		{
            clean += c;
            lastWasSlash = false;
        }
    }
    if (clean.size() > 1 && clean[clean.size() - 1] == '/')
        clean.erase(clean.size() - 1);
    return clean;
}

std::string shortenFileName(const std::string &name, size_t maxLength)
{
    if (name.length() <= maxLength)
        return name;

    const std::string ellipsis = "(...)";
    size_t keep = (maxLength - ellipsis.length()) / 2;

    std::string shortened = name.substr(0, keep) + ellipsis + name.substr(name.length() - keep);
    return shortened;
}

std::string getFileClass(const std::string &name, const struct stat &st)
{
    if (S_ISDIR(st.st_mode))
        return "folder";

    size_t dotPos = name.find_last_of('.');
    if (dotPos != std::string::npos)
	{
        std::string ext = name.substr(dotPos + 1);
        for (size_t i = 0; i < ext.size(); ++i)
            ext[i] = std::tolower(ext[i]);
        if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "gif" || ext == "bmp")
            return "image";
        else if (ext == "mp4" || ext == "avi" || ext == "mkv" || ext == "mov")
            return "video";
        else if (ext == "pdf")
            return "pdf";
        else if (ext == "txt" || ext == "md" || ext == "cpp" || ext == "h")
            return "text";
        else
            return "file";
    }
    return "file";
}

