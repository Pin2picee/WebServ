#include "Response.hpp"

ResponseHandler::ResponseHandler(const Server &server) : _server(server) {}

ResponseHandler::~ResponseHandler() {}

Response ResponseHandler::handleRequest(const Request &req)
{
	Response	res;
	const std::vector<Locations> &locs = _server.getLocations();
	const Locations *target = NULL;

	for (size_t i = 0; i < locs.size(); ++i)
	{
		if (!req.uri.find(locs[i].path))
			target = &locs[i];
	}
	if (!target)
		makeResponse(res, 404, "No matching location", getMimeType(req.path));
	else if (req.method == "GET")
		res = handleGet(*target, req);
	else if (req.method == "POST")
		res = handlePost(*target, req);
	else if (req.method == "DELETE")
		res = handleDelete(*target, req);
	else
		makeResponse(res, 405, "Method not allowed", getMimeType(req.path));
	return res;
}


#include <algorithm>

Response ResponseHandler::handleGet(const Locations &loc, const Request &req)
{
	Response	res;
	std::string	full_path = req.path;
	struct stat	s;

	if (std::find(loc.methods.begin(), loc.methods.end(), "GET") == loc.methods.end())
		makeResponse(res, 405, "GET not allowed on this location", getMimeType(full_path));
	else if (full_path.empty() || full_path == RED "none")
		makeResponse(res, 400, "Invalid file path", getMimeType(full_path));
	else if (stat(full_path.c_str(), &s) == 0 && S_ISDIR(s.st_mode))
	{
		if (loc.autoindex)
		{
			std::ostringstream body;
			body << "<html><body><h1>Index of " << req.uri << "</h1><ul>";

			DIR *dir = opendir(full_path.c_str());
			if (dir)
			{
				struct dirent *entry;
				while ((entry = readdir(dir)))
					body << "<li><a href=\"" << entry->d_name << "\">" << entry->d_name << "</a></li>";
				closedir(dir);
			}
			body << "</ul></body></html>";
			makeResponse(res, 200, body.str(), getMimeType(full_path));
		}
		else
		{
			bool found = false;
			for (size_t i = 0; i < loc.index_files.size(); i++)
			{
				std::string index_path = full_path + "/" + loc.index_files[i];
				if (stat(index_path.c_str(), &s) == 0)
				{
					std::ifstream ifs(index_path.c_str());
					if (ifs)
					{
						std::ostringstream buf;
						buf << ifs.rdbuf();
						makeResponse(res, 200, buf.str(), getMimeType(full_path));
						found = true;
						break;
					}
				}
			}
			if (!found)
				makeResponse(res, 403, "Directory listing denied", getMimeType(full_path));
		}
		return res;
	}
	else
	{
		std::ifstream end_ifs(full_path.c_str(), std::ios::binary);
		if (!end_ifs)
			makeResponse(res, 404, "File not found", getMimeType(full_path));
		else if (loc.cgi && full_path.size() >= loc.cgi_extension.size() &&
			full_path.substr(full_path.size() - loc.cgi_extension.size()) == loc.cgi_extension)
			makeResponse(res, 200, "CGI executed", getMimeType(full_path));
		else
		{
			std::ostringstream buf;
	
			buf << end_ifs.rdbuf();
			makeResponse(res, 200, buf.str(), getMimeType(full_path));
		}
	}
	return res;
}

Response ResponseHandler::handlePost(const Locations &loc, const Request &req)
{
	Response res;

	if (std::find(loc.methods.begin(), loc.methods.end(), "POST") == loc.methods.end())
		makeResponse(res, 405, "POST not allowed on this location", getMimeType(req.path));
	else if (req.body.size() > _server.getClientMaxBodySize())
		makeResponse(res, 413, "Request body too large", getMimeType(req.path));
	else if (loc.upload_dir != RED "none")
	{
		std::string filename = loc.upload_dir + "/testfile.txt";
		std::ofstream ofs(filename.c_str(), std::ios::binary);
		if (!ofs)
		{
			makeResponse(res, 500, "Cannot create file", getMimeType(req.path));
			return res;
		}
		ofs.write(req.body.c_str(), req.body.size());
		ofs.close();
		makeResponse(res, 201, "File uploaded successfully", getMimeType(req.path));
	}
	else if (loc.cgi)
		makeResponse(res, 200, "CGI executed", getMimeType(req.path));
	else
		makeResponse(res, 200, "POST handled", getMimeType(req.path));
	return res;
}

Response ResponseHandler::handleDelete(const Locations &loc, const Request &req)
{
	Response	res;

	if (std::find(loc.methods.begin(), loc.methods.end(), "DELETE") == loc.methods.end())
		makeResponse(res, 405, "DELETE not allowed on this location", getMimeType(req.path));
	else if (req.path.empty() || req.path == RED "none")
		makeResponse(res, 400, "Invalid file path", getMimeType(req.path));
	else if (std::remove(req.path.c_str()) != 0)
		makeResponse(res, 404, "File not found or cannot delete", getMimeType(req.path));
	else 
		makeResponse(res, 200, "File deleted successfully", getMimeType(req.path));
	return res;
}
