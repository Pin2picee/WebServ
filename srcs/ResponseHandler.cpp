#include "ResponseHandler.hpp"
#include "Client.hpp"
/* constructor */
ResponseHandler::ResponseHandler(const Server &server) : _server(server) {}

/* destructor */
ResponseHandler::~ResponseHandler() {}

const Locations *findLocation(const Request &req, const std::vector<Locations> &locs)
{
	const Locations* bestMatch = NULL;
	size_t longest = 0;

	for (size_t i = 0; i < locs.size(); ++i)
	{
		const std::string &locPath = locs[i].path;
		if (req.path.compare(0, locPath.size(), locPath) == 0 && locPath.size() > longest)
		{
			bestMatch = &locs[i];
			longest = locPath.size();
		}
	}
	return bestMatch;
}

/**
 * @brief
 * Handle any `Request`.
 * 
 * @param req The `Request` struct that will be processed.
 * 
 * @return a `Response` structure that will answer in adequation to the `Request`.
 */
Response ResponseHandler::handleRequest(const Request &req, std::map<std::string, Session> &g_sessions, Client *current)
{
	Response	res;
	Session &session = getSession(g_sessions, req, res);
	const std::vector<Locations> &locs = _server.getLocations();
	const Locations *target = findLocation(req, locs);

	deleteSession();
	if (!session.uploaded_files.size())
		removeAutoindexButton();
	if (session.current_page.empty())
		session.current_page = _server.getRoot() + req.path;
	if (!target)
		makeResponseFromFile(res, 404, _server.getErrorPage(404, session), req);
	else if (req.method == "GET")
		handleGet(res, *target, req, session, current);
	else if (req.method == "POST")
		handlePost(res, *target, req, session, current);
	else if (req.method == "DELETE")
		handleDelete(res, *target, req, session);
	else
		makeResponseFromFile(res, 405, _server.getErrorPage(405, session), req);
	return res;
}

/**
 * @brief
 * Handle the get `Request`.
 * 
 * @param loc The `Request` location that will be found in `req` variable of `handleRequest` method.
 * @param req The `Request` struct of `handleRequest` that will be processed.
 */
void ResponseHandler::handleGet(Response &res, const Locations &loc, const Request &req, Session &session, Client *current)
{
	std::string	full_path = (loc.root[0] == '/')
							? cleanPath(loc.root + "/" + req.uri.substr(loc.path.size()))
							: cleanPath(_server.getRoot() + "/" + loc.root + "/" + req.uri.substr(loc.path.size()));

	full_path = urlDecode(full_path);
	if (std::find(loc.methods.begin(), loc.methods.end(), "GET") == loc.methods.end())
		return makeResponseFromFile(res, 405, _server.getErrorPage(405, session), req);
	if (req.path == "/teapot")
		return makeResponseFromFile(res, 418, _server.getErrorPage(418, session), req);
	else if (full_path.empty())
		return makeResponseFromFile(res, 400, _server.getErrorPage(400, session), req);
	else if (pathExists(full_path))
	{
		if (loc.sensitive)
			return makeResponseFromFile(res, 403, _server.getErrorPage(403, session), req);
		else if (loc.autoindex)
			return generateAutoindex(full_path, full_path.substr(_server.getRoot().size()), req, res, session);
		else
		{
			for (size_t i = 0; i < loc.index_files.size(); i++)
			{
				std::string index_path = cleanPath(full_path + "/" + loc.index_files[i]);
				struct stat s;
				if (stat(index_path.c_str(), &s) == 0)
				{
					std::string content;
					if (req.path == "/delete_file.html")
						content = generateDeleteFileForm(session);
					else
						content = readFile(index_path);
					session.current_page.clear();
					session.current_page = index_path;
					return makeResponse(res, 200, content, getMimeType(req));
				}
			}
			return makeResponseFromFile(res, 403, _server.getErrorPage(403, session), req);
		}
	}
	else
	{
		std::ifstream end_ifs(full_path.c_str(), std::ios::binary);
		if (!end_ifs)
			return makeResponseFromFile(res, 404, _server.getErrorPage(404, session), req);
		else if (loc.cgi && full_path.size() >= loc.cgi_extension.size() &&
			full_path.substr(full_path.size() - loc.cgi_extension.size()) == loc.cgi_extension)
				return _server.handleCGI(req, loc, current);
		else
		{
			std::ostringstream buf;
			if (req.path == "/delete_file.html")
				buf << generateDeleteFileForm(session);
			else
				buf << end_ifs.rdbuf();
			session.current_page = full_path;
			return makeResponse(res, 200, buf.str(), getMimeType(req));
		}
	}
}

/**
 * @brief
 * Handle the post `Request`.
 * 
 * @param loc The `Request` location that will be found in `req` variable of `handleRequest` method.
 * @param req The `Request` struct of `handleRequest` that will be processed.
 */
void ResponseHandler::handlePost(Response &res, const Locations &loc, const Request &req, Session &session, Client *current)
{
	if (std::find(loc.methods.begin(), loc.methods.end(), "POST") == loc.methods.end())
		return makeResponseFromFile(res, 405, _server.getErrorPage(405, session), req);
	if (req.body.size() > _server.getClientMaxBodySize())
		return makeResponseFromFile(res, 413, _server.getErrorPage(413, session), req);
	return getContentType(res, loc, req, session, current);
}

/**
 * @brief
 * Handle the delete `Request`.
 * 
 * @param loc The `Request` location that will be found in `req` variable of `handleRequest` method.
 * @param req The `Request` struct of `handleRequest` that will be processed.
 */
void ResponseHandler::handleDelete(Response &res, const Locations &loc, const Request &req, Session &session)
{
	if (std::find(loc.methods.begin(), loc.methods.end(), "DELETE") == loc.methods.end())
		return makeResponse(res, 405, makeJsonError("DELETE not allowed on this location"), getMimeType(req));
	std::string filename = urlDecode(req.query);
	std::string::size_type pos = filename.rfind('/');
	if (pos != std::string::npos)
		filename = filename.substr(pos + 1);
	if (filename.empty())
		return makeResponse(res, 404, makeJsonError("File not found"), getMimeType(req));
	std::string deletePath = cleanPath(_server.getRoot() + "/" + loc.upload_dir + "/" + session.ID + "/" + filename);
	std::ifstream file(deletePath.c_str());
	if (!file || std::remove(deletePath.c_str()) != 0)
		return makeResponse(res, 404, makeJsonError("File not found"), getMimeType(req));
	removeUploadFileSession(session, deletePath);
	if ((!session.uploaded_files.size() && loc.autoindex))
		removeAutoindexButton();
	return makeResponse(res, 200, makeJsonError("File deleted successfully"), getMimeType(req));
}


//utils
void ResponseHandler::generateAutoindex(const std::string &fullpath, const std::string &locPath, const Request &req, Response &res, Session &session)
{
    const std::string fullPath = fullpath;
    DIR *dirPtr = opendir(fullPath.c_str());
    if (!dirPtr)
        return makeResponse(res, 500, readFile(_server.getErrorPage(500, session)), "text/html");

    std::ostringstream fileList;
    struct dirent *entry;
    const std::string autoindexRoot = "/autoindex";

    while ((entry = readdir(dirPtr)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;
        std::string entryFullPath = fullPath + "/" + name;
        struct stat st;
        if (stat(entryFullPath.c_str(), &st) != 0)
            continue;
        std::string hrefPath = locPath;
        if (hrefPath[hrefPath.size() - 1] != '/')
            hrefPath += "/";
        hrefPath += name;
        if (S_ISDIR(st.st_mode))
            hrefPath += "/";

		std::string liClass = getFileClass(name, st);
        std::string displayName = shortenFileName(name, 30);
        if (S_ISDIR(st.st_mode))
            displayName += "/";

        fileList << "<li class=\"" << liClass << "\"><a href=\"" << hrefPath << "\">" 
                 << displayName << "</a></li>\n";
    }
    closedir(dirPtr);
	if (locPath != autoindexRoot && locPath != autoindexRoot + "/")
	{
		std::string parentPath = locPath;
		if (parentPath[parentPath.size() - 1] == '/')
			parentPath.erase(parentPath.size() - 1);
		size_t pos = parentPath.find_last_of('/');
		if (pos != std::string::npos)
			parentPath = parentPath.substr(0, pos + 1);
		else
			parentPath = autoindexRoot + "/";
		fileList << "<li class=\"go-back\"><a href=\"" << parentPath << "\">⬅️ Go Back</a></li>\n";
	}
    std::ifstream templateFile("./config/www/autoindex.html");
    if (!templateFile)
        return makeResponse(res, 500, readFile(_server.getErrorPage(500, session)), "text/html");
    std::stringstream buffer;
    buffer << templateFile.rdbuf();
    std::string html = buffer.str();
    std::string placeholder = "<!-- FILE_LIST_PLACEHOLDER -->";
    size_t pos = html.find(placeholder.c_str());
    if (pos != std::string::npos)
        html.replace(pos, placeholder.length(), fileList.str());
    std::string buttonHtml = "<a href=\"/\" class=\"button\">Return to Home</a>\n";
    pos = html.find("</body>");
    if (pos != std::string::npos)
        html.insert(pos, buttonHtml);
    makeResponse(res, 200, html, getMimeType(req));
}

/**
 * @brief
 * Get the reason phrase of a corresponding status code.
 * 
 * @param status_code The status code that will get a corresponding reason phrase.
 * 
 * @return
 * The corresponding reason phrase.
 */

static std::string getReasonPhrase(int status_code)
{
	switch (status_code)
	{
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 409: return "Conflict";
		case 413: return "Payload Too Large";
		case 418: return "I'm a teapot";
		case 500: return "Internal Server Error";
		default:  return "Unknown";
	}
}


std::string ResponseHandler::generateDeleteFileForm(const Session &session, const std::string &uploadRoot)
{
	std::string html = readFile(_server.getRoot() + "/" + "delete_file.html");
	std::string userDir = cleanPath(uploadRoot + "/" + session.ID);
	std::string fileSelectHtml;
	bool hasFiles = false;

	if (!pathExists(userDir) || session.uploaded_files.empty())
		fileSelectHtml = "<p>No files to delete.</p>";
	else
	{
		std::vector<std::string> files = session.uploaded_files;
		hasFiles = true;
		fileSelectHtml += "<select name=\"filename\" id=\"filename\">";
		for (size_t i = 0; i < files.size(); ++i)
		{
			std::string filename = files[i];
			std::size_t pos = filename.rfind('/');
			if (pos != std::string::npos)
				filename = filename.substr(pos + 1);
			fileSelectHtml += "<option value=\"" + filename + "\">" + filename + "</option>";
		}
		fileSelectHtml += "</select>";
	}
	std::string searchStr = "<input type=\"text\" id=\"filename\" placeholder=\"Enter filename\" required>";
	size_t pos = html.find(searchStr);
	if (pos != std::string::npos)
		html.replace(pos, searchStr.length(), fileSelectHtml);
	if (!hasFiles)
	{
		std::string buttonStr = "<button type=\"submit\">Delete</button>";
		pos = html.find(buttonStr);
		if (pos != std::string::npos)
			html.erase(pos, buttonStr.length());
	}
	return html;
}

/**
 * @brief
 * Convert a Response struct into a string in standard HTTP format.
 * 
 * @param res The Response struct that will be converted.
 * 
 * @return
 * The string HTTP message.
 */
std::string	ResponseHandler::responseToString(const Response &res)
{
	std::ostringstream oss;

	oss << res.version << " " << res.status_code << " " << getReasonPhrase(res.status_code) << "\r\n";
	if (!res.content_type.empty())
		oss << "Content-Type: " << res.content_type << "\r\n";
	oss << "Content-Length: " << res.body.size() << "\r\n";
	for (size_t i = 0; i < res.headers.size(); ++i)
		oss << "Set-Cookie: " << res.headers[i] << "\r\n";
	oss << "\r\n" << res.body;
	return oss.str();
}

ResponseHandler::ResponseHandler(const ResponseHandler &copy) : _server(copy._server)
{
	if (this != &copy)
		*this = copy;
}

ResponseHandler &ResponseHandler::operator=(const ResponseHandler &assignement)
{
	(void)assignement;
	return (*this);// rien a mettre egal a l'assignement car la seule variable est const donc deja init
}

/**
 * @param path The request path
 *  
 * @return
 * Return the MIME (Multipurpose Internet Mail Extensions) content type that have `Response` when responding to a `Request`.
 */
std::string ResponseHandler::getMimeType(const Request &req)
{
	std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Type");
	if (it != req.headers.end())
	{
		std::string contentType = it->second;
		if (contentType.find(MIME_TEXT_HTML) != std::string::npos)
			return MIME_TEXT_HTML;
		if (contentType.find(MIME_TEXT_CSS) != std::string::npos)
			return MIME_TEXT_CSS;
		if (contentType.find(MIME_TEXT_JAVASCRIPT) != std::string::npos)
			return MIME_TEXT_JAVASCRIPT;
		if (contentType.find(MIME_APPLICATION_JSON) != std::string::npos)
			return MIME_APPLICATION_JSON;
		if (contentType.find(MIME_APPLICATION_XML) != std::string::npos)
			return MIME_APPLICATION_XML;
		if (contentType.find(MIME_APPLICATION_PDF) != std::string::npos)
			return MIME_APPLICATION_PDF;
		if (contentType.find(MIME_APPLICATION_ZIP) != std::string::npos)
			return MIME_APPLICATION_ZIP;
		if (contentType.find(MIME_APPLICATION_GZIP) != std::string::npos)
			return MIME_APPLICATION_GZIP;
		if (contentType.find(MIME_IMAGE_JPEG) != std::string::npos)
			return MIME_IMAGE_JPEG;
		if (contentType.find(MIME_IMAGE_PNG) != std::string::npos)
			return MIME_IMAGE_PNG;
		if (contentType.find(MIME_IMAGE_GIF) != std::string::npos)
			return MIME_IMAGE_GIF;
		if (contentType.find(MIME_IMAGE_SVG) != std::string::npos)
			return MIME_IMAGE_SVG;
		if (contentType.find(MIME_IMAGE_WEBP) != std::string::npos)
			return MIME_IMAGE_WEBP;
		if (contentType.find(MIME_IMAGE_BMP) != std::string::npos)
			return MIME_IMAGE_BMP;
		if (contentType.find(MIME_FONT_WOFF) != std::string::npos)
			return MIME_FONT_WOFF;
		if (contentType.find(MIME_FONT_WOFF2) != std::string::npos)
			return MIME_FONT_WOFF2;
		if (contentType.find(MIME_FONT_TTF) != std::string::npos)
			return MIME_FONT_TTF;
		if (contentType.find(MIME_FONT_OTF) != std::string::npos)
			return MIME_FONT_OTF;
		if (contentType.find(MIME_AUDIO_MP3) != std::string::npos)
			return MIME_AUDIO_MP3;
		if (contentType.find(MIME_AUDIO_WAV) != std::string::npos)
			return MIME_AUDIO_WAV;
		if (contentType.find(MIME_VIDEO_MP4) != std::string::npos)
			return MIME_VIDEO_MP4;
		if (contentType.find(MIME_VIDEO_WEBM) != std::string::npos)
			return MIME_VIDEO_WEBM;
	}
	return getMimeType(req.path);
}

std::string ResponseHandler::getMimeType(const std::string &path)
{
	if (path.find(".html") != std::string::npos)
		return MIME_TEXT_HTML;
	if (path.find(".htm") != std::string::npos)
		return MIME_TEXT_HTML;
	if (path.find(".css") != std::string::npos)
		return MIME_TEXT_CSS;
	if (path.find(".js") != std::string::npos)
		return MIME_TEXT_JAVASCRIPT;
	if (path.find(".json") != std::string::npos)
		return MIME_APPLICATION_JSON;
	if (path.find(".xml") != std::string::npos)
		return MIME_APPLICATION_XML;
	if (path.find(".pdf") != std::string::npos)
		return MIME_APPLICATION_PDF;
	if (path.find(".zip") != std::string::npos)
		return MIME_APPLICATION_ZIP;
	if (path.find(".gz") != std::string::npos)
		return MIME_APPLICATION_GZIP;
	if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos)
		return MIME_IMAGE_JPEG;
	if (path.find(".png") != std::string::npos)
		return MIME_IMAGE_PNG;
	if (path.find(".gif") != std::string::npos)
		return MIME_IMAGE_GIF;
	if (path.find(".svg") != std::string::npos)
		return MIME_IMAGE_SVG;
	if (path.find(".webp") != std::string::npos)
		return MIME_IMAGE_WEBP;
	if (path.find(".bmp") != std::string::npos)
		return MIME_IMAGE_BMP;
	if (path.find(".woff") != std::string::npos)
		return MIME_FONT_WOFF;
	if (path.find(".woff2") != std::string::npos)
		return MIME_FONT_WOFF2;
	if (path.find(".ttf") != std::string::npos)
		return MIME_FONT_TTF;
	if (path.find(".otf") != std::string::npos)
		return MIME_FONT_OTF;
	if (path.find(".mp3") != std::string::npos)
		return MIME_AUDIO_MP3;
	if (path.find(".wav") != std::string::npos)
		return MIME_AUDIO_WAV;
	if (path.find(".mp4") != std::string::npos)
		return MIME_VIDEO_MP4;
	if (path.find(".webm") != std::string::npos)
		return MIME_VIDEO_WEBM;
	if (path.find(".exe") != std::string::npos)
		return MIME_APPLICATION_OCTET_STREAM;
	return MIME_TEXT_HTML;
}

void ResponseHandler::makeResponseFromFile(Response &res, int status, const std::string &path, const Request &req)
{
    makeResponse(res, status, readFile(path), getMimeType(req));
}

std::string createUploadDir(const std::string &root, const std::string &upload_dir, const std::string &userId)
{
	if (userId.empty())
		return "";
	std::string path = cleanPath(root + "/" + upload_dir + "/" + userId);
	if (mkdir(path.c_str(), 0755) == -1)
	{
		if (errno == EEXIST)
			return path;
		else
			return std::cerr << "Error creating directory " << path << ": " << strerror(errno) << std::endl, "";
	}
	return path;
}

/**
 * @brief
 * Manage the POST request of a file.
 * 
 * @param boundary The start and end point of the file body.
 * @param res The `Response` structure that will be returned.
 * @param loc The `Request` location that will be found in `req` variable of `handleRequest` method.
 * @param req The `Request` struct of `handleRequest` that will be processed.
 */
void	ResponseHandler::handleFile(std::string &boundary, Response &res, const Locations &loc, const Request &req, Session &session)
{
	if (boundary.empty())
		return makeResponseFromFile(res, 400, _server.getErrorPage(400, session), req);
	std::size_t fileStart = req.body.find("filename=\"");
	if (fileStart != std::string::npos)
	{
		std::string filename = getFileName(req.body);
		if (filename.empty())
			return makeResponse(res, 204, "", getMimeType(req));
		std::string Userpath = createUploadDir(_server.getRoot(), loc.upload_dir, session.ID);
		if (Userpath.empty())
			return makeResponseFromFile(res, 500, _server.getErrorPage(500, session), req);
		filename = cleanPath(Userpath + "/" + filename);
		fileStart = req.body.find("\r\n\r\n", fileStart);
		if (fileStart == std::string::npos)
			return makeResponseFromFile(res, 400, _server.getErrorPage(400, session), req);
		fileStart += 4;
		std::size_t fileEnd = req.body.find(boundary, fileStart);
		if (fileEnd == std::string::npos)
			return makeResponseFromFile(res, 400, _server.getErrorPage(400, session), req);
		fileEnd -= 2;
		if (fileEnd - fileStart > _server.getClientMaxBodySize())
			return makeResponseFromFile(res, 413, _server.getErrorPage(413, session), req);
		std::ofstream ofs(filename.c_str(), std::ios::binary);
		if (!ofs)
			return makeResponseFromFile(res, 500, _server.getErrorPage(500, session), req);
		ofs.write(req.body.c_str() + fileStart, fileEnd - fileStart);
		ofs.close();
		if (std::find(session.uploaded_files.begin(), session.uploaded_files.end(), filename) != session.uploaded_files.end())
			return makeResponseFromFile(res, 409, _server.getErrorPage(409, session), req);
		session.uploaded_files.push_back(filename);
		if (session.uploaded_files.size() && loc.autoindex)
			addAutoindexButton(cleanPath(loc.upload_dir + "/" + session.ID));
		return makeResponseFromFile(res, 201, _server.getErrorPage(201, session), req);
	}
	return makeResponseFromFile(res, 400, _server.getErrorPage(400, session), req);
}

/**
 * @brief
 * Manage a POST request by finding if the content is a file or not and produce a response accordingly.
 * 
 * @param res The `Response` structure that will be returned.
 * @param loc The `Request` location that will be found in `req` variable of `handleRequest` method.
 * @param req The `Request` struct of `handleRequest` that will be processed.
 */
void	ResponseHandler::getContentType(Response &res, const Locations &loc, const Request &req, Session& session, Client *current)
{
	std::string contentType;
	std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Type");

	if (it != req.headers.end())
		contentType = it->second;
	else
		contentType = "application/octet-stream";
	if (loc.cgi)
	{
		size_t	size_path = req.path.size();
		size_t	point = req.path.rfind('.');
		if (point == std::string::npos)
			return makeResponse(res, 405, readFile(_server.getErrorPage(405, session)), getMimeType(req));
		std::string	extension_path = req.path.substr(point);
		std::vector<std::string>	pack_extension_CGI;
		pack_extension_CGI.push_back(".cgi");
		pack_extension_CGI.push_back(".py");
		pack_extension_CGI.push_back(".php");
		pack_extension_CGI.push_back(".pl");
		pack_extension_CGI.push_back(".rb");
		pack_extension_CGI.push_back(".sh");

		std::vector<std::string>::iterator itt = std::find(pack_extension_CGI.begin(), pack_extension_CGI.end(), extension_path);
		if (size_path >= 5 && pack_extension_CGI.end() == itt)
			return makeResponse(res, 405, readFile(_server.getErrorPage(405, session)), getMimeType(req));
	}
	if (!contentType.empty() && contentType.find("multipart/form-data") != std::string::npos)
	{
		std::size_t pos = contentType.find("boundary=");
        if (pos == std::string::npos || pos + 9 >= contentType.size())
            return makeResponse(res, 400, readFile(_server.getErrorPage(400, session)), getMimeType(req));
        std::string boundary = contentType.substr(pos + 9);
        std::size_t end = boundary.find(';');
        if (end != std::string::npos)
            boundary = boundary.substr(0, end);
		boundary = "--" + boundary;
        return handleFile(boundary, res, loc, req, session);
	}
	if (req.body.size() > _server.getClientMaxBodySize())
		return makeResponse(res, 413, readFile(_server.getErrorPage(413, session)), getMimeType(req));
	std::string Path = _server.getRoot() + req.path;
	if (!access(Path.c_str(), F_OK))
		return _server.handleCGI(req, loc, current);
    return makeResponse(res, 404, readFile(_server.getErrorPage(404, session)), getMimeType(req));
}
