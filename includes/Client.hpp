#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Socket.hpp"
# include "ResponseHandler.hpp"

enum 
{
	Syntax,
	Method,
	Uri,
};

class Client
{
	private :
		std::string request;
		std::string reponse;
		std::string ip;
		std::string port;
		timeval start;
		timeval end;
		bool	connected;
		bool	request_finish;
		size_t  offset;
		Socket  *my_socket;
		
	public :// a changer
		ResponseHandler	handler;
	//base
		Client();
		~Client();
		Client(const Client &copy);
		Client &operator=(const Client &assignement);
		void	setbasic(std::string ip_address, std::string port_address);// assign les valeurs basic d'un nouveau client
		
	//set
		void			setRequest(std::string buf);
		int				ParseRequest(void);
		void			setReponse(std::string buf);
		void			set_socket(Socket *the_socket);
	//get
		std::string		&getRequest();
		std::string		&getReponse();
		size_t			&getOffset();
		bool			&getFinishRequest();
		Socket			*getMySocket();
	//method
		Request			ExtractRequest();
		void			AddOffset(size_t nb);
		void			view_log();//utiliser seulement par deconected car end pas encore init;affiche les temps de connexions avec l'ip et port + socket serveur
		void			disconnected();//met a false + view_log()
};

#endif