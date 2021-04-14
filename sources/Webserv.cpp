/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: user42 <user42@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/11 16:27:17 by sadarnau          #+#    #+#             */
/*   Updated: 2021/04/13 17:30:46 by user42           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Webserv.hpp"

Webserv::Webserv( void )
{
	return ;
}

// Webserv::Webserv( std::string name )
// {
// 	return ;
// }

Webserv::Webserv( Webserv const & src )
{
	*this = src;
	return ;
}

Webserv::~Webserv( void )
{
	return ;
}

Webserv & Webserv::operator=( Webserv const & rhs)
{
    // this->??? = rhs.???;
	(void)rhs;
	return ( *this );
}

void	Webserv::initialization( void )
{
	this->fd = socket(AF_INET, SOCK_STREAM, 0); // to pprotect
	
	this->fillAddress();

	bind(this->fd, (struct sockaddr *)&this->address, sizeof(this->address));  // to pprotect

	listen(this->fd, 5); // 5 = number of max connections
	
	return ;
}

void	Webserv::fillAddress( void )
{
	this->address.sin_family = AF_INET;
	this->address.sin_addr.s_addr = INADDR_ANY; //htonl ??
	this->address.sin_port = htons(8080);

	memset(this->address.sin_zero, 0, sizeof(this->address.sin_zero));  // to pprotect

	return ;
}

void	Webserv::handleRequest( void )
{
	int in_sock;

	unsigned int addrlen = sizeof(address);
	in_sock = accept(this->fd, (struct sockaddr *)&this->address, (socklen_t*)&addrlen);
	
	char buff[1000];				// GNL maybe better ?
	int ret = read( in_sock , buff, 1000);	// to protect
	
	buff[ret] = 0;

	Request inRequest(in_sock, buff);
	this->inRequest = inRequest;
}

int		Webserv::getInSocket( void )
{
	return (this->inRequest.getInSock());
}

int		Webserv::getFd( void )
{
	return (this->fd);
}