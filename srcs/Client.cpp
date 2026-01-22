/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <locagnio@student.42perpignan.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 17:31:17 by marvin            #+#    #+#             */
/*   Updated: 2026/01/23 00:12:55 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Client.hpp"

/**
 * @brief
 * Constructs a new Client associated with a server socket.
 *
 * @param the_socket Pointer to the server `Socket`.
 */
Client::Client(Socket *the_socket) : my_socket(the_socket), connected(true), handler(*(my_socket->getBlockServer()))
{
	OffsetBodyCgi = 0;
	request_finish = false;
	addPipeToPoll = false;
	correct_syntax = true;
	offset = 0;
	InCgi = false;
	_pid = 0;
	OffsetCgi = 0;
	this->ResponseGenerate = false;
	fd_pipe_in = -1;
	fd_pipe_out = -1;
	gettimeofday(&this->start, NULL);
	
}

/**
 * @brief
 * Destructor for `Client`.
 */
Client::~Client() {}

/**
 * @brief
 * Copy constructor for `Client`.
 *
 * @param copy The `Client` instance to copy.
 */
Client::Client(const Client &copy) : handler(copy.handler)
{
	if (this != &copy)
		*this = copy;
}

/**
 * @brief
 * Assignment operator for `Client`.
 *
 * @param copy The `Client` instance to assign.
 *
 * @return Reference to the assigned `Client`.
 */
Client &Client::operator=(const Client &copy)
{
	if (this != &copy)
	{
		ip = copy.ip;
		port = copy.port;
		request = copy.request;
		reponse = copy.reponse;
		connected = copy.connected;
		my_socket = copy.my_socket;
		offset = copy.offset;
		start = copy.start;
		request_finish = copy.request_finish;
		handler = copy.handler;
		struct_request = copy.struct_request;
		correct_syntax = copy.correct_syntax;
		InCgi = copy.InCgi;
		_pid = copy._pid;
		_body = copy._body;
		CgiOutput = copy.CgiOutput;
		addPipeToPoll = copy.addPipeToPoll;
		OffsetCgi = copy.OffsetCgi;
		ResponseGenerate = copy.ResponseGenerate;
		fd_pipe_in = copy.fd_pipe_in;
		fd_pipe_out = copy.fd_pipe_out;
		OffsetBodyCgi = copy.OffsetBodyCgi;
		//pas de end car init dans deconnected;
	}
	return (*this);
}

/**
 * @brief
 * Sets the basic client connection information.
 *
 * @param ip_address Client IP address.
 * @param port_address Client port.
 */
void Client::setBasic(std::string ip_address, std::string port_address)
{
	ip = ip_address;
	port = port_address;
}

/**
 * @brief
 * Resets request-related client information.
 */
void	Client::resetRequestState()
{
	this->request_finish = false;
	this->correct_syntax = true;
	this->offset = 0;
}

/**
 * @brief
 * Resets client state after a CGI execution.
 */
void	Client::resetAfterCGI()
{
	this->OffsetBodyCgi = 0;
	// Ne pas remettre request_finish à false ici : la nouvelle requête peut déjà être arrivée
	this->request.clear();
	this->fd_pipe_in = -1;
	this->fd_pipe_out = -1;
	this->_pid = 0;
	// Ne pas effacer _body ici : il sera écrasé par setBody() lors de la prochaine requête
	this->addPipeToPoll = false;
}

/**
 * @brief
 * Sets the request body.
 *
 * @param body Request body content.
 */
void	Client::setBody(std::string body)
{
	_body = body;
}

/**
 * @brief
 * Sets the CGI process PID.
 *
 * @param pid CGI process ID.
 */
void	Client::setCgiPid(pid_t pid)
{
	_pid = pid;
}

/**
 * @brief
 * Sets the response generation state.
 *
 * @param etat Boolean state.
 */
void	Client::setResponseGenerate(bool etat)
{
	this->ResponseGenerate = etat;
}


/**
 * @brief
 * Appends received data to the request buffer and parses it.
 *
 * @param buf Received data buffer.
 */
void	Client::setRequest(std::string buf)
{
	size_t	pos;
	std::string	line;
	if (!this->request_finish)
		this->request += buf;
	else
	{
		this->request = buf;
		resetRequestState();
	}
	Request tmp = ExtractRequest();
	// si GET & autres alors pas de body par contre si POST alors body
	// Le but est de mettre le request_finish a true si la requete est fini
	pos = request.find("\r\n");
	line = request.substr(0, pos);
	if (line.find("  ") != std::string::npos)
		correct_syntax = false;
	if (tmp.method != "POST" && request.find("\r\n\r\n") != std::string::npos)
	{
		request_finish = true;
		this->struct_request = tmp;
	}
	else if (tmp.method == "POST")
	{
		std::map<std::string, std::string>::iterator temp_it = tmp.headers.find("Content-Length");
		size_t pos_end = this->request.find("\r\n\r\n") + 4;
		int	nb_caracters_body = -1;
		if (temp_it != tmp.headers.end())
			nb_caracters_body = std::atoi(temp_it->second.c_str());
		if (nb_caracters_body < 0 || (size_t)nb_caracters_body != this->request.size() - pos_end)
		{
			correct_syntax = false;
			return ;
		}
		request_finish = true;
	}
}

/**
 * @brief
 * Adds a cookie to the client.
 *
 * @param name Cookie name.
 * @param value Cookie value.
 */
void	Client::setCookies(std::string name, std::string value)
{
	this->cookies[name] = value;
}

/**
 * @brief
 * Sets the HTTP response string.
 *
 * @param buf Response content.
 */
void	Client::setResponse(std::string buf)
{
	this->reponse = buf;
	this->offset = 0;
}

/**
 * @brief
 * Marks the client as being in a CGI execution.
 */
void	Client::setInCgi()
{
	if (!this->InCgi)
		this->InCgi = true;
}

/**
 * @brief
 * Marks the client as out of CGI execution.
 */
void	Client::setOutCgi()
{
	if (this->InCgi)
		this->InCgi = false;
}

/**
 * @brief
 * Sets the CGI input pipe file descriptor.
 *
 * @param fd Pipe file descriptor.
 */
void	Client::setPipeIn(int fd)
{
	this->fd_pipe_in = fd;
}

/**
 * @brief
 * Sets the CGI output pipe file descriptor.
 *
 * @param fd Pipe file descriptor.
 */
void	Client::setPipeOut(int fd)
{
	this->fd_pipe_out = fd;
}

/**
 * @brief
 * Stores the CGI start time.
 */
void	Client::setCgiStartTime(void)
{
	gettimeofday(&this->cgi_start_time, NULL);
}

/**
 * @brief
 * Sets whether CGI pipes were added to poll.
 *
 * @param booleen Boolean state.
 */
void	Client::setAddPipeToPoll(bool	booleen)
{
	this->addPipeToPoll = booleen;
}

/**
 * @brief
 * Increments the offset of the CGI body written.
 *
 * @param nb Number of bytes written.
 */
void	Client::addCgiBodyOffset(size_t nb)
{
	this->OffsetBodyCgi += nb;
}

/**
 * @brief
 * Returns the CGI body offset.
 *
 * @return Reference to the offset value.
 */
const size_t	&Client::getOffsetBodyCgi() const
{
	return (this->OffsetBodyCgi);
}

/**
 * @brief
 * Indicates if CGI pipes are registered in poll.
 *
 * @return Reference to the state.
 */
const bool	&Client::getaddPipeToPoll(void) const
{
	return (this->addPipeToPoll);
}

/**
 * @brief
 * Returns the request body.
 *
 * @return Reference to the body string.
 */
const std::string	&Client::getBody(void) const
{
	return (this->_body);
}

/**
 * @brief
 * Returns the CGI process PID.
 *
 * @return Reference to the PID.
 */
const pid_t	&Client::getCgiPid(void) const
{
	return (this->_pid);
}

/**
 * @brief
 * Indicates if a response is ready to be sent.
 *
 * @return Reference to the state.
 */
const bool	&Client::getResponseGenerate() const
{
	return (this->ResponseGenerate);
}

/**
 * @brief
 * Indicates if the client is currently in CGI.
 *
 * @return Reference to the state.
 */
const bool	&Client::getInCGI() const
{
	return (this->InCgi);
}

/**
 * @brief
 * Returns the raw HTTP request.
 *
 * @return Reference to the request string.
 */
const std::string	&Client::getRequest() const 
{
	return (this->request);
}

/**
 * @brief
 * Returns the CGI start time.
 *
 * @return Reference to the timeval structure.
 */
const timeval	&Client::getCgiStartTime(void) const
{
	return (this->cgi_start_time);
}

/**
 * @brief
 * Returns the CGI input pipe FD.
 *
 * @return Reference to the FD.
 */
const int	&Client::getPipeIn() const
{
	return (this->fd_pipe_in);
}

/**
 * @brief
 * Returns the CGI output pipe FD.
 *
 * @return Reference to the FD.
 */
const int	&Client::getPipeOut() const
{
	return (this->fd_pipe_out);
}

/**
 * @brief
 * Returns the response sending offset.
 *
 * @return Reference to the offset.
 */
const size_t &Client::getOffset() const
{
	return (this->offset);
}

/**
 * @brief
 * Indicates if the request is complete.
 *
 * @return Reference to the state.
 */
const bool	&Client::getFinishRequest() const
{
	return (request_finish);
}

/**
 * @brief
 * Indicates if the request syntax is valid.
 *
 * @return Reference to the state.
 */
const bool	&Client::getSyntax() const
{
	return (this->correct_syntax);
}

/**
 * @brief
 * Returns the HTTP response string.
 *
 * @return Reference to the response.
 */
const std::string &Client::getReponse() const
{
	return (this->reponse);
}

/**
 * @brief
 * Returns the client's cookies.
 *
 * @return Reference to the cookie map.
 */
const std::map<std::string, std::string> &Client::getCookies() const
{
	return (this->cookies);
}

/**
 * @brief
 * Returns the server port associated with the client.
 *
 * @return Server port.
 */
size_t	Client::getServerPort() const
{
	return (my_socket->getPort());
}

/**
 * @brief
 * Returns the CGI output buffer.
 *
 * @return Reference to the output string.
 */
const std::string	&Client::getCgiOutput() const
{
	return (this->CgiOutput);
}


/**
 * @brief
 * Appends data to the CGI output buffer.
 *
 * @param morceau Data chunk.
 *
 * @return New CGI output offset.
 */
size_t	Client::addCgiOutput(std::string morceau)
{
	this->CgiOutput += morceau;
	OffsetCgi += morceau.size();
	return (this->OffsetCgi);
}

/**
 * @brief
 * Clears the CGI output buffer.
 */
void	Client::ResetCgiOutput()
{
	this->OffsetCgi = 0;
	this->CgiOutput = "";
}

/**
 * @brief
 * Increments the response sending offset.
 *
 * @param nb Number of bytes sent.
 */
void	Client::addOffset(size_t nb)
{
	this->offset += nb;
}

/**
 * @brief
 * Displays a connection/disconnection log for the client.
 */
void	Client::viewLog()
{
	long	start_h = (start.tv_sec / 3600) % 24 + 1;
	long	start_m = (start.tv_sec / 60) % 60;

	long	end_h = (end.tv_sec / 3600) % 24 + 1;
	long	end_m = (end.tv_sec / 60) % 60;

	std::cout    << RED << "the Client " << this->ip << " connected at " << std::setw(2) << std::setfill('0') << start_h << "h"
                << std::setw(2) << std::setfill('0') << start_m << " and disconnected at " << std::setw(2) << std::setfill('0')
                << end_h << "h" << std::setw(2) << std::setfill('0') << end_m << " on port " << port << RESET << std::endl;
}

/**
 * @brief
 * Marks the client as disconnected and logs it.
 */
void	Client::disconnected()
{
	this->connected = false;
	gettimeofday(&this->end, NULL);
	viewLog();
}

/**
 * @brief
 * Extracts and parses the HTTP request into a `Request` structure.
 *
 * @return Parsed `Request`.
 */
Request	Client::ExtractRequest()
{
	Request	tmp;
	size_t	pos;
	size_t	pos_finish;
	size_t	pos_point;
	std::string line;
	
	//extract request_line
	pos_finish = request.find("\r\n\r\n");//la fin de la requete
	pos = request.find("\r\n");//fin premiere ligne	
	if (pos_finish == std::string::npos || pos == std::string::npos )
		return tmp;
	
	line = request.substr(0, pos);//toute la premiere ligne
	std::stringstream ss(line);//decoupe la premiere ligne
	ss >> tmp.method >> tmp.uri >> tmp.version;
	size_t qmark = tmp.uri.find('?');
	tmp.path = tmp.uri.substr(0, qmark);
	tmp.query = "";
	if (qmark != std::string::npos && tmp.uri.find("filename=", qmark) != std::string::npos)
		qmark += 9;
	if (qmark != std::string::npos)
		tmp.query = tmp.uri.substr(qmark + 1);

	while (pos != pos_finish)
	{
		pos += 2;//on bypass \r\n
		pos_point = request.find(":", pos);
		if (pos_point == std::string::npos || pos == std::string::npos)
			return (tmp);
		tmp.headers[request.substr(pos, (pos_point) - pos)] = request.substr(pos_point + 1, request.find("\r\n", pos_point + 1) - (pos_point + 1));//avant et apres le point;
		pos = request.find("\r\n", pos);
	}
	for (std::map<std::string, std::string>::iterator it = tmp.headers.begin(); it != tmp.headers.end(); it++ )
	{
		int	i = 0;
		
		while (it->second[i] && (it->second[i] == ' ' || it->second[i] == '\t'))
			i++;
		int	j = it->second.length() - 1;
		while (it->second[j] && (it->second[j] == ' ' || it->second[j] == '\t'))
			j--;
		it->second = it->second.substr(i, (j - i) + 1);
	}
	tmp.cookies = this->cookies;
	parseCookies(tmp);
	pos += 4;
	tmp.body = request.substr(pos);
	return (tmp);
}
