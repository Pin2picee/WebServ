# include "Client.hpp"

Client::Client() : connected(true), handler(*(my_socket->getBlockServ()))
{
	request_finish = false;
	offset = 0;
	gettimeofday(&this->start, NULL);
	this->reponse ="HTTP/1.1 200 OK\r\n"
						"Content-Length: 5\r\n"
						"Content-Type: text/plain\r\n"
						"\r\n"
						"SALUT\r\n\r\n";
}

Client::~Client() {}

Client::Client(const Client &copy)
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
		//pas de end car init dans deconnected;
	}
	return (*this);
}

void Client::setbasic(std::string ip_address, std::string port_address)
{
	ip = ip_address;
	port = port_address;
}

/*SET-GET*/

void	Client::setRequest(std::string buf)
{
	this->request += buf;
	if (request.find("\r\n\r\n") != std::string::npos)
		request_finish = true;
}

void	Client::setReponse(std::string buf)
{
		this->reponse = buf;
}

std::string	&Client::getRequest()
{
	return (this->request);
}

size_t &Client::getOffset()
{
	return (this->offset);
}

bool	&Client::getFinishRequest()
{
	return (request_finish);
}

std::string &Client::getReponse()
{
	return (this->reponse);
}

Socket *Client::getMySocket()
{
	return (this->my_socket);
}

void			Client::AddOffset(size_t nb)
{
	this->offset += nb;
}

void	Client::set_socket(Socket *the_socket)
{
	this->my_socket = the_socket;
}


void	Client::view_log()
{
	long	start_h = (start.tv_sec / 3600) % 24 + 2;
	long	start_m = (start.tv_sec / 60) % 60;

	long	end_h = (end.tv_sec / 3600) % 24 + 2;
	long	end_m = (end.tv_sec / 60) % 60;

	std::cout << RED <<"the Client " << this->ip << " connected at " << start_h << "h" << start_m;
	std::cout << " and disconnected at " << end_h << "h" << end_m << " on port: " << port << RESET << std::endl;
	std::cout << "The request is :\n" << GREEN << request << RESET << std::endl;
}

void	Client::disconnected()
{
	this->connected = false;
	gettimeofday(&this->end, NULL);
	view_log();
}

int	Client::ParseRequest()
{
	if (request.find("  "))
		return(Syntax);
	return (42);
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
	tmp.path = tmp.uri.substr(0, tmp.uri.find("?"));

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
