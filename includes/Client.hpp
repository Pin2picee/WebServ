/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <locagnio@student.42perpignan.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 01:40:16 by abelmoha          #+#    #+#             */
/*   Updated: 2026/01/23 00:14:43 by marvin           ###   ########.fr       */
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

/**
 * @brief
 * Represents a connected client.
 * 
 * This class stores all client-related data such as:
 * - The socket used for communication
 * - The current HTTP request and response
 * - CGI execution state
 * - Connection timing and cookies
 * 
 * @param my_socket        Pointer to the Socket used by the client.
 * @param request          Raw HTTP request received from the client.
 * @param _body            Body content of the request (POST data, form data).
 * @param CgiOutput        Accumulated output from a CGI execution.
 * @param struct_request   Parsed Request structure for this client.
 * @param reponse          HTTP response content to be sent to the client.
 * @param ip               Client IP address.
 * @param port             Client port as string.
 * @param cookies          Cookies associated with this client session.
 * @param start            Timestamp when the client connected.
 * @param end              Timestamp when the client disconnected.
 * @param cgi_start_time   Timestamp when a CGI execution started.
 * @param connected        True if the client is still connected.
 * @param request_finish   True if the HTTP request has been fully received.
 * @param ResponseGenerate True if a response has been generated for this client.
 * @param correct_syntax   True if the HTTP request syntax is valid.
 * @param InCgi            True if the client is currently being processed by CGI.
 * @param addPipeToPoll      True if the CGI pipes have been added to the poll loop.
 * @param offset           Tracks the response body send offset for partial sends.
 * @param OffsetCgi        Tracks how much CGI output has been read/sent.
 * @param OffsetBodyCgi    Tracks how much request body has been read by CGI.
 * @param fd_pipe_out      File descriptor for the write end of the CGI pipe.
 * @param fd_pipe_in       File descriptor for the read end of the CGI pipe.
 * @param _pid             PID of the CGI process handling this client.
 */
class Client
{
private :
	Socket								*my_socket;
	std::string							request;
	std::string							_body;
	std::string							CgiOutput;
	Request								struct_request;
	std::string							reponse;
	std::string							ip;
	std::string							port;
	timeval								start;//Connexion client
	std::map<std::string, std::string>	cookies;
	timeval								end;//deco client
	timeval								cgi_start_time;
	bool								connected;
	bool								request_finish;
	bool								ResponseGenerate;
	bool								correct_syntax;
	bool								InCgi;
	size_t								offset;
	bool								addPipeToPoll;
	int									fd_pipe_out;//lecture a la sortie du fils donc 0 car sortie du pipe
	int									fd_pipe_in;//ecriture a l'entre du fils pour le body donc 1 car entre du pipe
	pid_t								_pid;
	size_t								OffsetCgi;
	size_t								OffsetBodyCgi;

public :
	ResponseHandler	handler;

	// Constructors / Destructor

	Client(Socket *the_socket);
	Client(const Client &copy);
	Client &operator=(const Client &assignement);
	~Client();

	// Base

	void	setBasic(std::string ip_address, std::string port_address);// assign les valeurs basic d'un nouveau client
		
	// Setters

	void			setRequest(std::string buf);
	void			setCookies(std::string name, std::string value);
	void			setResponse(std::string buf);
	void			setPipeIn(int fd);
	void			setPipeOut(int fd);
	void			setInCgi(void);
	void			setOutCgi(void);
	void			setBody(std::string body);
	void			setCgiPid(pid_t pid);
	void			setAddPipeToPoll(bool	booleen);
	void			setResponseGenerate(bool etat);
	void			setCgiStartTime(void);

	// Getters

	const std::map<std::string, std::string>	&getCookies() const;
	const std::string							&getRequest() const;
	const std::string							&getReponse() const;
	const size_t								&getOffset() const;
	const size_t								&getOffsetBodyCgi() const;
					
	const bool									&getFinishRequest() const;
	const bool									&getResponseGenerate() const;
	const bool									&getSyntax() const;
	const bool									&getInCGI() const;
	const int									&getPipeIn() const;
	const int									&getPipeOut() const;
	const std::string							&getBody(void) const;
	const pid_t									&getCgiPid(void) const;
	const std::string							&getCgiOutput(void) const;
	const bool									&getaddPipeToPoll(void) const;
	const timeval								&getCgiStartTime(void) const;
	size_t										getServerPort() const;
		
	// Methods

	void			ResetCgiOutput();
	void			resetRequestState();
	void			resetAfterCGI();
	Request			ExtractRequest();
	void			addOffset(size_t nb);
	void			addCgiBodyOffset(size_t nb);


	size_t			addCgiOutput(std::string morceau);
	void			viewLog();//utiliser seulement par deconected car end pas encore init;affiche les temps de connexions avec l'ip et port + socket serveur
	void			disconnected();//met a false + view_log()
};

#endif
