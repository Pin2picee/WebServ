/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/23 20:34:56 by abelmoha          #+#    #+#             */
/*   Updated: 2026/01/09 17:10:31 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Client.hpp"

Client::Client(Socket *the_socket) : my_socket(the_socket), connected(true), handler(*(my_socket->getBlockServ()))
{
	OffsetBodyCgi = 0;
	request_finish = false;
	PipeAddPoll = false;
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

Client::~Client() {}

Client::Client(const Client &copy) : handler(copy.handler)
{
	if (this != &copy)
		*this = copy;
}

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
		PipeAddPoll = copy.PipeAddPoll;
		OffsetCgi = copy.OffsetCgi;
		ResponseGenerate = copy.ResponseGenerate;
		fd_pipe_in = copy.fd_pipe_in;
		fd_pipe_out = copy.fd_pipe_out;
		OffsetBodyCgi = copy.OffsetBodyCgi;
		//pas de end car init dans deconnected;
	}
	return (*this);
}

void Client::setbasic(std::string ip_address, std::string port_address)
{
	ip = ip_address;
	port = port_address;
}

//Reset tout les donnes reutilisable pour la prochaine requete du meme client
void	Client::resetInf()
{
		this->request_finish = false;
		this->correct_syntax = true;
		this->offset = 0;
}

//Reset apres CGI - ne touche pas a correct_syntax, offset, body ni request_finish
void	Client::resetAfterCGI()
{
	this->OffsetBodyCgi = 0;
	// Ne pas remettre request_finish à false ici : la nouvelle requête peut déjà être arrivée
	this->request.clear();
	this->fd_pipe_in = -1;
	this->fd_pipe_out = -1;
	this->_pid = 0;
	this->PipeAddPoll = false;
	// Ne pas effacer _body ici : il sera écrasé par setBody() lors de la prochaine requête
}

/*SET-GET*/

void			Client::setBody(std::string body)
{
	_body = body;
}
void			Client::setCgiPid(pid_t pid)
{
	_pid = pid;
}

void			Client::setResponseGenerate(bool etat)
{
	this->ResponseGenerate = etat;
}

//SET + parsing
void	Client::setRequest(std::string buf)
{
	size_t	pos;
	std::string	line;
	if (!this->request_finish)
		this->request += buf;
	else
	{
		this->request = buf;
		resetInf();
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

void	Client::setReponse(std::string buf)
{
		this->reponse = buf;
		this->offset = 0;
}
//mets en boolen
void	Client::setInCGI()
{
	if (!this->InCgi)
		this->InCgi = true;
}

void	Client::setOutCGI()
{
	if (this->InCgi)
		this->InCgi = false;
}
//je lis dedans
void			Client::setPipeIn(int fd)
{
	this->fd_pipe_in = fd;
}

//j'ecris dedans
void			Client::setPipeOut(int fd)
{
	this->fd_pipe_out = fd;
}

void			Client::setCGiStartTime(void)
{
	gettimeofday(&this->cgi_start_time, NULL);
}

void			Client::setPipeAddPoll(bool	booleen)
{
	this->PipeAddPoll = booleen;
}

void			Client::AddOffsetBodyCgi(size_t nb)
{
	this->OffsetBodyCgi += nb;
}
const size_t			&Client::getOffsetBodyCgi() const
{
	return (this->OffsetBodyCgi);
}

const bool				&Client::getPipeAddPoll(void) const
{
	return (this->PipeAddPoll);
}

const std::string		&Client::getBody(void) const
{
	return (this->_body);
}

const pid_t			&Client::getCgiPid(void) const
{
	return (this->_pid);
}

const bool			&Client::getResponseGenerate() const
{
	return (this->ResponseGenerate);
}

const bool			&Client::getInCGI() const
{
	return (this->InCgi);
}

const std::string	&Client::getRequest() const
{
	return (this->request);
}

const timeval	&Client::getCgiStartTime(void) const
{
	return (this->cgi_start_time);
}

const int				&Client::getPipeIn() const
{
	return (this->fd_pipe_in);
}

const int				&Client::getPipeOut() const
{
	return (this->fd_pipe_out);
}

const size_t &Client::getOffset() const
{
	return (this->offset);
}

const bool	&Client::getFinishRequest() const
{
	return (request_finish);
}

const bool	&Client::getSyntax() const
{
	return (this->correct_syntax);
}

const std::string &Client::getReponse() const
{
	return (this->reponse);
}

const Socket *Client::getMySocket() const
{
	return (this->my_socket);
}

const std::string	&Client::getCgiOutput() const
{
	return (this->CgiOutput);
}

size_t			Client::AddCgiOutput(std::string morceau)
{
	this->CgiOutput += morceau;
	OffsetCgi += morceau.size();
	return (this->OffsetCgi);
}

void			Client::ResetCgiOutput()
{
	this->OffsetCgi = 0;
	this->CgiOutput = "";
}

void			Client::AddOffset(size_t nb)
{
	this->offset += nb;
}



void	Client::view_log()
{
	long	start_h = (start.tv_sec / 3600) % 24 + 1;
	long	start_m = (start.tv_sec / 60) % 60;

	long	end_h = (end.tv_sec / 3600) % 24 + 1;
	long	end_m = (end.tv_sec / 60) % 60;

	std::cout << RED <<"the Client " << this->ip << " connected at " << start_h << "h" << start_m;
	std::cout << " and disconnected at " << end_h << "h" << end_m << " on port: " << port << RESET << std::endl;
	//affichage de la requete qui vient du client :  std::cout << "The request is :\n" << GREEN << request << RESET << std::endl;
}

void	Client::disconnected()
{
	this->connected = false;
	gettimeofday(&this->end, NULL);
	view_log();
}

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
	if (qmark != std::string::npos)
		tmp.query = tmp.uri.substr(qmark + 10);

	//extract HEADERS LINES
	while (pos != pos_finish)
	{
		pos += 2;//on bypass \r\n
		pos_point = request.find(":", pos);
		if (pos_point == std::string::npos || pos == std::string::npos)
			return (tmp);
		tmp.headers[request.substr(pos, (pos_point) - pos)] = request.substr(pos_point + 1, request.find("\r\n", pos_point + 1) - (pos_point + 1));//avant et apres le point;
		pos = request.find("\r\n", pos);//on va a la fin de la ligne
	}
	//extract body
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
	pos += 4;
	tmp.body = request.substr(pos);
	return (tmp);
}
