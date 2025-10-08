#include "ConfigParser.hpp"
#include "utils.hpp"
#include <cstdlib>

ConfigParser::ConfigParser(const std::string &file) : _file(file) {}

LocationConf	parse_log(size_t *i, std::vector<std::string> tokens, std::string root)
{
	LocationConf loc;

	loc.path = tokens[*i + 1];
	*i += 2;
	while (*i < tokens.size() && tokens[*i] != "}")
	{
		if (tokens[*i] == "methods")
		{
			loc.methods.clear();
			while (++(*i) < tokens.size() && tokens[*i][tokens[*i].size() - 1] != ';')
				loc.methods.push_back(tokens[*i]);
			if (tokens[*i][tokens[*i].size() - 1] == ';')
				loc.methods.push_back(strip_semicolon(tokens[*i]));
		}
		else if (tokens[*i] == "index")
		{
			loc.index_files.clear();
			while (++(*i) < tokens.size() && tokens[*i][tokens[*i].size() - 1] != ';')
				loc.index_files.push_back(tokens[*i]);
			if (tokens[*i][tokens[*i].size() - 1] == ';')
				loc.index_files.push_back(strip_semicolon(tokens[*i]));
		}
		else if (tokens[*i] == "autoindex")
			loc.autoindex = (strip_semicolon(tokens[++(*i)]) == "on");
		else if (tokens[*i] == "upload_dir")
			loc.upload_dir = strip_semicolon(tokens[++(*i)]);
		else if (tokens[*i] == "cgi")
			loc.cgi = (strip_semicolon(tokens[++(*i)]) == "on");
		else if (tokens[*i] == "cgi_extension")
			loc.cgi_extension = strip_semicolon(tokens[++(*i)]);
		else if (tokens[*i] == "root")
			loc.root = strip_semicolon(tokens[++(*i)]);
		++(*i);
	}
	if (loc.root.empty())
		loc.root = root;
	return (loc);
}

ServerConf ConfigParser::parse(void)
{
	std::ifstream ifs(_file.c_str());
	if (!ifs.is_open())
		throw std::runtime_error("Cannot open config file : " + _file);
	std::vector<std::string> tokens = tokenize(ifs);
	ServerConf conf;
	LocationConf loc;

	for (size_t i = 0; i < tokens.size() && i + 1 < tokens.size(); ++i)
	{
		if (tokens[i] == "listen")
		{
			std::string ip_port = strip_semicolon(tokens[i + 1]);
			size_t colon = ip_port.find(':');
			if (colon == std::string::npos)
				throw std::runtime_error("Invalid listen format : " + ip_port);

			std::string ip = ip_port.substr(0, colon);
			int port = atoi(ip_port.substr(colon + 1).c_str());
			conf.listen.push_back(std::make_pair(ip, port));
		}
		else if (tokens[i] == "root")
			conf.root = strip_semicolon(tokens[i + 1]);
		else if (tokens[i] == "error_page")
			conf.error_pages[atoi(tokens[i + 1].c_str())] = conf.root + strip_semicolon(tokens[i + 2]);
		else if (tokens[i] == "client_max_body_size")
			conf.client_max_body_size = atoi(tokens[i + 1].c_str());
		else if (tokens[i] == "location")
			conf.locations.push_back(parse_log(&i, tokens, conf.root));
	}
	init_default_errors(conf);
	return conf;
}

std::vector<std::string> ConfigParser::tokenize(std::istream &ifs)
{
	std::vector<std::string> tokens;
	std::string line;

	while (std::getline(ifs, line))
	{
		std::istringstream iss(line);
		std::string tok;

		while (iss >> tok)
			tokens.push_back(tok);
	}
	return tokens;
}

#include <algorithm>

Response handleGet(const LocationConf &loc, const Request &req)
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

Response handlePost(const LocationConf &loc, const Request &req, const ServerConf &server)
{
	Response res;

	if (std::find(loc.methods.begin(), loc.methods.end(), "POST") == loc.methods.end())
		makeResponse(res, 405, "POST not allowed on this location", getMimeType(req.path));
	else if (req.body.size() > server.client_max_body_size)
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

Response handleDelete(const LocationConf &loc, const Request &req)
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
