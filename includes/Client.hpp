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
		Socket  *my_socket;
		std::string request;
		Request		struct_request;
		std::string reponse;
		std::string ip;
		std::string port;
		timeval start;
		timeval end;
		bool	connected;
		bool	request_finish;
		bool	correct_syntax;
		size_t  offset;
		std::map<std::string, std::string>	cookies;

		Client();
	public :// a changer
		ResponseHandler	handler;
	//base
		Client(Socket *the_socket);
		~Client();
		Client(const Client &copy);
		Client &operator=(const Client &assignement);
		void	setbasic(std::string ip_address, std::string port_address);// assign les valeurs basic d'un nouveau client
		
	//set
		void			setRequest(std::string buf);
		void			setCookies(std::string name, std::string value);
		int				ParseSyntaxRequest(void);
		void			setReponse(std::string buf);
	//get
		std::string		&getRequest();
		std::string		&getReponse();
		size_t			&getOffset();
		bool			&getFinishRequest();
		Socket			*getMySocket();
		bool			&getSyntax();
		std::map<std::string, std::string> &getCookies();
	//method
		void			resetInf();
		Request			ExtractRequest();
		void			AddOffset(size_t nb);
		void			view_log();//utiliser seulement par deconected car end pas encore init;affiche les temps de connexions avec l'ip et port + socket serveur
		void			disconnected();//met a false + view_log()
};

#endif