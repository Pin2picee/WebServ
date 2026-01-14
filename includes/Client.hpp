/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <locagnio@student.42perpignan.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 01:40:16 by abelmoha          #+#    #+#             */
/*   Updated: 2026/01/14 16:28:26 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ResponseHandler.hpp"

enum HeaderType
{
	Syntax,
	Method,
	Uri
};

class Client
{
	private :
		Socket  	*my_socket;
		std::string request;
		std::string _body;
		std::string	CgiOutput;
		Request		struct_request;
		std::string reponse;
		std::string ip;
		std::string port;
		std::map<std::string, std::string>	cookies;
		timeval 	start;//Connexion client
		timeval 	end;//deco client
		timeval 	cgi_start_time;
		bool		connected;
		bool		request_finish;
		bool		ResponseGenerate;
		bool		correct_syntax;
		bool		InCgi;
		size_t  	offset;
		bool		PipeAddPoll;
		int			fd_pipe_out;//lecture a la sortie du fils donc 0 car sortie du pipe
		int			fd_pipe_in;//ecriture a l'entre du fils pour le body donc 1 car entre du pipe
		pid_t		_pid;
		size_t		OffsetCgi;
		size_t		OffsetBodyCgi;

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
		void			setReponse(std::string buf);
		void			setPipeIn(int fd);
		void			setPipeOut(int fd);
		void			setInCGI(void);
		void			setOutCGI(void);
		void			setBody(std::string body);
		void			setCgiPid(pid_t pid);
		void			setPipeAddPoll(bool	booleen);
		void			setResponseGenerate(bool etat);
		void			setCGiStartTime(void);
	//get
		const std::map<std::string, std::string> &getCookies() const;
		const std::string		&getRequest() const;
		const std::string		&getReponse() const;
		const size_t			&getOffset() const;
		const size_t			&getOffsetBodyCgi() const;

		const bool				&getFinishRequest() const;
		const bool				&getResponseGenerate() const;
		const bool				&getSyntax() const;
		const bool				&getInCGI() const;
		const int				&getPipeIn() const;
		const int				&getPipeOut() const;
		const std::string		&getBody(void) const;
		const pid_t				&getCgiPid(void) const;
		const std::string		&getCgiOutput(void) const;
		const bool				&getPipeAddPoll(void) const;
		const timeval			&getCgiStartTime(void) const;
		
	//method
		void			ResetCgiOutput();
		void			resetInf();
		void			resetAfterCGI();
		Request			ExtractRequest();
		void			AddOffset(size_t nb);
		void			AddOffsetBodyCgi(size_t nb);


		size_t			AddCgiOutput(std::string morceau);
		void			view_log();//utiliser seulement par deconected car end pas encore init;affiche les temps de connexions avec l'ip et port + socket serveur
		void			disconnected();//met a false + view_log()
};

#endif