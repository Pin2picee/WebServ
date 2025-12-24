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
}

/**
 * @brief
 * Initialise the errors in conf (can be changed by extracting the datas of HTML error files)
 * 
 * @param conf The `Server` configuration.
 */
void init_default_errors(Server& conf)
{
    std::map<int, std::string>& errors = conf.getErrorPagesRef();
    const std::string& root = conf.getRoot();
    std::string errorDir = root + "/errors";
    DIR* dir = opendir(errorDir.c_str());
    if (!dir)
        return;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string filename = entry->d_name;
        if (filename == "." || filename == "..")
            continue;
        if (filename.size() < 8 || filename.substr(filename.size() - 5) != ".html")
            continue;
        std::string codeStr = filename.substr(0, filename.size() - 5);
        int code = std::atoi(codeStr.c_str());
        if (code < 100 || code > 599)
            continue;
        std::string fullPath = errorDir + "/" + filename;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0 && S_ISREG(st.st_mode))
            errors[code] = fullPath;
    }
    closedir(dir);
}

bool removeDirectoryRecursive(const std::string &path)
{
    DIR *dir = opendir(path.c_str());
    if (!dir)
        return false;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;

        std::string fullPath = path + "/" + name;

        struct stat entryStat;
        if (stat(fullPath.c_str(), &entryStat) == -1)
            continue;

        if (S_ISDIR(entryStat.st_mode))
        {
            removeDirectoryRecursive(fullPath);
            rmdir(fullPath.c_str());
        }
        else
        {
            unlink(fullPath.c_str());
        }
    }
    closedir(dir);

    return (rmdir(path.c_str()) == 0);
}


void resetUploadsDir(const std::string &uploadsPath)
{
	if (pathExists(uploadsPath))
	{
		if (!removeDirectoryRecursive(uploadsPath))
			std::cerr << "Failed to remove " << uploadsPath << std::endl;
	}

	if (mkdir(uploadsPath.c_str(), 0755) == -1)
		std::cerr << "Failed to recreate " << uploadsPath << std::endl;
}

std::vector<Socket *> all_sockets;
volatile sig_atomic_t	on = 1;

void	handle_sigint(int signum)
{
	(void)signum;
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

size_t convertSize(const std::string &input)
{
	if (input.empty())
		throw std::invalid_argument("Empty size string");
	std::string str = strip_semicolon(input);

	while (!str.empty() && std::isspace(static_cast<unsigned char>(str[str.size() - 1])))
		str.erase(str.size() - 1);
	while (!str.empty() && std::isspace(static_cast<unsigned char>(str[0])))
		str.erase(0, 1);
	if (str.empty())
		throw std::invalid_argument("Invalid size string");
	size_t multiplier = 1;
	if (str.size() > 2)
	{
		std::string suffix2 = str.substr(str.size() - 2);
		for (size_t i = 0; i < suffix2.size(); ++i)
			suffix2[i] = std::toupper(static_cast<unsigned char>(suffix2[i]));

		if (suffix2 == "KB")
			multiplier = 1000;
		else if (suffix2 == "MB")
			multiplier = 1000 * 1000;
		else if (suffix2 == "GB")
			multiplier = 1000 * 1000 * 1000;

		if (multiplier != 1)
			str = str.substr(0, str.size() - 2);
	}
	if (str.size() > 1)
	{
		char upper = std::toupper(static_cast<unsigned char>(str[str.size() - 1]));
		if (upper == 'K')
			multiplier = 1024;
		else if (upper == 'M')
			multiplier = 1024 * 1024;
		else if (upper == 'G')
			multiplier = 1024 * 1024 * 1024;

		if (upper == 'K' || upper == 'M' || upper == 'G')
			str = str.substr(0, str.size() - 1);
	}
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (!std::isdigit(static_cast<unsigned char>(str[i])))
			throw std::invalid_argument("Invalid number part in size");
	}
	char *end;
	size_t base = std::strtoul(str.c_str(), &end, 10);
	if (*end != '\0')
		throw std::invalid_argument("Invalid number conversion");

	return static_cast<size_t>(base) * multiplier;
}

void displayRequestInfo(const Request &req)
{
	std::cout << "------- displayRequestInfo :" << std::endl;
    std::cout << RED "Version: " RESET << req.version << std::endl;
	std::cout << RED "Method: " RESET << req.method << std::endl;
	std::cout << RED "URI: " RESET << req.uri << std::endl;
	std::cout << RED "Path: " RESET << req.path << std::endl;
	std::cout << RED "Headers:" RESET << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it)
		std::cout << "  " CYAN << it->first << ": " RESET << it->second << std::endl;
	if (!req.cookies.empty())
	{
		std::cout << RED "Cookies:" RESET << std::endl;
		for (std::map<std::string, std::string>::const_iterator it = req.cookies.begin(); it != req.cookies.end(); ++it)
			std::cout << "  " MAGENTA << it->first << ": " RESET << it->second << std::endl;
	}
	return ;
	std::cout << RED "Body: " RESET << std::endl;
	std::cout << req.body << std::endl;
}

void displayResponseInfo(const Response &res)
{
	std::cout << "------- displayResponseInfo :" << std::endl;
	std::cout << RED "Version: " RESET << res.version << std::endl;
	std::cout << RED "Status Code: " RESET << res.status_code << std::endl;
	std::cout << RED "Content-Type: " RESET << res.content_type << std::endl;
	if (!res.headers.empty())
	{
		std::cout << RED "Headers:" RESET << std::endl;
		for (std::vector<std::string>::const_iterator it = res.headers.begin(); it != res.headers.end(); ++it)
			std::cout << "  " CYAN << *it << RESET << std::endl;
	}
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

time_t getCurrentTime()
{
	return std::time(NULL);
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
