/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: user42 <user42@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/11 16:25:26 by sadarnau          #+#    #+#             */
/*   Updated: 2021/04/14 14:33:56 by user42           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <iostream>
# include <map>
# include <vector>
# include <sstream>
# include <colors.hpp>
# include <iomanip>
# include "Logger.hpp"

class Request
{
private:

	int			in_socket;
	std::string	buff;

	std::string method;
	std::string target;
	std::map<std::string, std::string> headers;
	std::vector<std::string> skipped_headers;

public:

	Request( void );								//default constructor
	Request( int in_sock, char *buff );					//constructor
	Request( Request const & src);  				//copy
	~Request( void );								//destructor
	Request & operator=( Request const & rhs );		//overload operator =

	bool	isValidHeader(std::string header);
	bool	isRequestMethod(std::string key);
	void	parseRequest(std::string req);
	void	printRequest( void );
	int		getInSock( void );
};

#endif
