/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <locagnio@student.42perpignan.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 01:39:44 by abelmoha          #+#    #+#             */
/*   Updated: 2026/01/16 18:21:07 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseHandler.hpp"
#include "Client.hpp"

/**
 * @brief
 * Constructor for ResponseHandler.
 *
 * @param server The `Server` object to handle requests for.
 */
ResponseHandler::ResponseHandler(const Server &server) : _server(server) {}

/**
 * @brief
 * Destructor for ResponseHandler.
 */
ResponseHandler::~ResponseHandler() {}

/**
 * @brief
 * Find the best matching `Location` for a given `Request`.
 *
 * @param req The `Request` to match.
 * @param locs The vector of `Locations` to search in.
 *
 * @return Pointer to the best matching `Location`, or `NULL` if none matches.
 */
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
 * Handle any `Request` and produce a `Response`.
 *
 * @param req The `Request` struct that will be processed.
 * @param g_sessions Map of all `Session`s.
 * @param current Pointer to the current `Client`.
 *
 * @return A `Response` struct suitable for the `Request`.
 */
Response ResponseHandler::handleRequest(const Request &req, std::map<std::string, Session> &g_sessions, Client *current)
{
	Response	res;
	deleteSession(g_sessions);
	Session &session = getSession(g_sessions, req, res, current->getServerPort());
	const std::vector<Locations> &locs = _server.getLocations();
	const Locations *target = findLocation(req, locs);

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
 * Handle a `GET` request.
 *
 * @param res The `Response` to populate.
 * @param loc The `Location` for this request.
 * @param req The `Request` struct to process.
 * @param session The current `Session`.
 * @param current Pointer to the current `Client`.
 */
void ResponseHandler::handleGet(Response &res, const Locations &loc, const Request &req, Session &session, Client *current)
{
	std::string	full_path = (loc.root[0] == '/')
							? cleanPath(loc.root + "/" + req.path.substr(loc.path.size()))
							: cleanPath(_server.getRoot() + "/" + loc.root + "/" + req.path.substr(loc.path.size()));

	full_path = urlDecode(full_path);
	if (std::find(loc.methods.begin(), loc.methods.end(), "GET") == loc.methods.end())
		return makeResponseFromFile(res, 405, _server.getErrorPage(405, session), req);
	if (req.path == "/teapot")
		return makeResponseFromFile(res, 418, _server.getErrorPage(418, session), req);
	else if (full_path.empty())
		return makeResponseFromFile(res, 400, _server.getErrorPage(400, session), req);
	else if (pathDirectoryExists(full_path))
	{
		if (loc.sensitive)
			return makeResponseFromFile(res, 403, _server.getErrorPage(403, session), req);
		else if (loc.root == "cgi-bin")
		{
			std::string filePath = _server.getRoot() + "/cgi_tester.html";
			return makeResponseFromFile(res, 200, filePath, req);
		}
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
			return (makeResponseFromFile(res, 404, _server.getErrorPage(404, session), req));
		else if (loc.cgi && full_path.size() >= loc.cgi_extension.size() &&
			full_path.substr(full_path.size() - loc.cgi_extension.size()) == loc.cgi_extension)
				return _server.handleCgi(req, loc, current);
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
 * Handle a `POST` request.
 *
 * @param res The `Response` to populate.
 * @param loc The `Location` for this request.
 * @param req The `Request` struct to process.
 * @param session The current `Session`.
 * @param current Pointer to the current `Client`.
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
 * Handle a `DELETE` request.
 *
 * @param res The `Response` to populate.
 * @param loc The `Location` for this request.
 * @param req The `Request` struct to process.
 * @param session The current `Session`.
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
	return makeResponse(res, 200, makeJsonError("File deleted successfully"), getMimeType(req));
}

/**
 * @brief
 * Generate an autoindex HTML page for a directory.
 *
 * @param fullpath The full path of the directory.
 * @param locPath The path relative to the server root.
 * @param req The `Request` struct.
 * @param res The `Response` to populate.
 * @param session The current `Session`.
 */
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
		std::string displayName = shortenFileName(name, 54);
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
 * Get the reason phrase for an HTTP status code.
 *
 * @param status_code The status code.
 *
 * @return The corresponding reason phrase.
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

/**
 * @brief
 * Generate the HTML form to delete files for a `Session`.
 *
 * @param session The `Session` whose files will be listed.
 * @param uploadRoot The upload root directory path.
 *
 * @return HTML string for the delete file form.
 */
std::string ResponseHandler::generateDeleteFileForm(const Session &session, const std::string &uploadRoot)
{
	std::string html = readFile(_server.getRoot() + "/" + "delete_file.html");
	std::string userDir = cleanPath(uploadRoot + "/" + session.ID);
	std::string fileSelectHtml;
	bool hasFiles = false;

	if (!pathDirectoryExists(userDir) || session.uploaded_files.empty())
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
 * Convert a `Response` struct into a string in standard HTTP format.
 *
 * @param res The `Response` struct.
 *
 * @return The HTTP string message.
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

/**
 * @brief
 * Copy constructor for ResponseHandler.
 *
 * @param copy The ResponseHandler to copy.
 */
ResponseHandler::ResponseHandler(const ResponseHandler &copy) : _server(copy._server)
{
	if (this != &copy)
		*this = copy;
}

/**
 * @brief
 * Assignment operator for ResponseHandler.
 *
 * @param assignement The ResponseHandler to assign from.
 *
 * @return Reference to this ResponseHandler.
 */
ResponseHandler &ResponseHandler::operator=(const ResponseHandler &assignement)
{
	(void)assignement;
	return (*this);
}

/**
 * @brief
 * Get the MIME type for a `Request`.
 *
 * @param req The `Request` struct.
 *
 * @return MIME type string.
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

/**
 * @brief
 * Get the MIME type based on a file path.
 *
 * @param path File path string.
 *
 * @return MIME type string.
 */
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

/**
 * @brief
 * Send a `Response` based on a file.
 *
 * @param res The `Response` to populate.
 * @param status The HTTP status code.
 * @param path Path of the file.
 * @param req The `Request` struct.
 */
void ResponseHandler::makeResponseFromFile(Response &res, int status, const std::string &path, const Request &req)
{
	makeResponse(res, status, readFile(path), getMimeType(req));
}

/**
 * @brief
 * Create a directory for uploads.
 *
 * @param root Server root path.
 * @param upload_dir Upload directory path relative to root.
 * @param userId User ID for the directory.
 *
 * @return Path to the created directory, or empty string on error.
 */
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
 * Handle a file upload in a `POST` request.
 *
 * @param boundary The boundary string in the multipart body.
 * @param res The `Response` to populate.
 * @param loc The `Location` of the request.
 * @param req The `Request` struct.
 * @param session The current `Session`.
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
		return makeResponseFromFile(res, 201, _server.getErrorPage(201, session), req);
	}
	return makeResponseFromFile(res, 400, _server.getErrorPage(400, session), req);
}

/**
 * @brief
 * Determine the content type of a `POST` request and handle accordingly.
 *
 * @param res The `Response` to populate.
 * @param loc The `Location` of the request.
 * @param req The `Request` struct.
 * @param session The current `Session`.
 * @param current Pointer to the current `Client`.
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
	if (!access(Path.c_str(), F_OK) && loc.cgi)
		return _server.handleCgi(req, loc, current);
	return makeResponse(res, 404, readFile(_server.getErrorPage(404, session)), getMimeType(req));
}
