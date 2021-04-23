#include "Request.hpp"

////////////////////
// COPLIEN'S
////////////////////
Request::Request( void )
{
	return ;
}

Request::Request(Config *config, int inSock, char *buff ) : _inSocket(inSock)
{
	this->_config = config;
	std::string	tmp(buff);	//convert char* to std::string
	this->_buff = tmp;
	this->parseRequest(this->_buff);
	this->createPath();
	this->printRequest();
	return ;
}

Request::Request( Request const & src )
{
	*this = src;
	return ;
}

Request::~Request( void )
{
	return ;
}

Request & Request::operator=( Request const & rhs)
{
    this->_buff = rhs._buff;
	this->_inSocket = rhs._inSocket;

	return ( *this );
}


////////////////////
// PARSE
////////////////////
void	Request::parseRequest(std::string req)
{
	std::string				line;
    std::string				value;
    std::string				key;
	std::istringstream		streamReq(req);

	while (std::getline(streamReq, line))
    {
		std::stringstream	ss(line);
		ss >> key >> value;										// set the variables

		if(ss.fail())											// if value extraction failed, break while loop
			break;

		if (this->isRequestMethod(key))							// handle method line
		{
			this->_method = key;
			this->_target = value;
		}
		else													// handle anything else (= headers)
		{
			key = key.substr(0, key.length() - 1);				// delete char ':' at the end of key
			if (this->isValidHeader(key))
				this->_headers[key] = value;
			else
				this->_skippedHeaders.push_back(key);
		}
	}
}


////////////////////
// UTILS
////////////////////
void	Request::createPath()
{
	// std::string mapRoot = this->_config->getMap()["root"]; //change with conf->location->root
	std::string mapRoot = "files/www"; //change with conf->location->root
	std::string root;

	//Remove '/' from root if exist (because target has already it)
	if (mapRoot.back() == '/')
		root = mapRoot.substr(0, mapRoot.size()-1);
	else
		root = mapRoot;

	// Create relative path
	this->_relativeTargetPath = root + this->_target;

	//Create absolute path
	char cwd[1000];
	getcwd(cwd, sizeof(cwd));
	std::string currentdir = cwd;
	this->_absoluteTargetPath = currentdir + "/" + root + this->_target;
}

bool	Request::isValidHeader(std::string header)
{
	std::string listOfAcceptedHeaders[18] = {"Accept-Charsets", "Accept-Language", "Allow", "Authorization", "Content-Language",
												"Content-Length", "Content-Location", "Content-Type", "Date", "Host", "Last-Modified",
												"Location", "Referer", "Retry-After", "Server", "Transfer-Encoding", "User-Agent",
												"WWW-Authenticate"};
	std::vector<std::string> acceptedHeaders;
	acceptedHeaders.assign(listOfAcceptedHeaders, listOfAcceptedHeaders + 18);

	for (std::vector<std::string>::iterator it = acceptedHeaders.begin(); it != acceptedHeaders.end(); ++it)
		if (header == *it)
			return (true);

	return (false);
}

bool	Request::isRequestMethod(std::string key)
{
	std::string listOfAcceptedMethods[8] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE"}; // see https://tools.ietf.org/html/rfc7231 - RFC 7231
	std::vector<std::string> acceptedMethods;
	acceptedMethods.assign(listOfAcceptedMethods, listOfAcceptedMethods + 8);

	for (std::vector<std::string>::iterator it = acceptedMethods.begin(); it != acceptedMethods.end(); ++it)
		if (key == *it)
			return (true);

	return (false);
}

void	Request::printRequest( void )
{
	std::ostringstream oss;
	
	oss << "\n----------\nREQUEST OBJECT :\n\n" ;
	oss << std::setw(20) << "request->method" << " : " << this->_method << std::endl;
	oss << std::setw(20) << "request->relativeTargetPath " << " : " << this->_relativeTargetPath << std::endl << std::endl;
	oss << std::setw(20) << "request->absoluteTargetPath " << " : " << this->_absoluteTargetPath << std::endl << std::endl;

	oss << "Content of request->headers :" << std::endl << std::endl; 
	oss << std::setw(20) << "KEY" << " : " << "VALUE" << std::endl << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = this->_headers.begin(); it != this->_headers.end(); ++it)
	{
		oss << std::setw(20) << it->first << " : " << it->second << std::endl;
	}

	//SKIPPED HEADERS
	oss << std::endl << std::endl << "Skipped headers : ";
	for (std::vector<std::string>::iterator it = this->_skippedHeaders.begin(); it != this->_skippedHeaders.end(); ++it)
		oss << " " << *it;
	oss << std::endl << std::endl;

	// RAW REQUEST
	oss << "Raw request :" << std::endl << std::endl;
	oss << this->_buff;
	oss << "----------\n\n";
	oss << RESET;
	Logger::Write(Logger::DEBUG, std::string(BLU), oss.str(), true);
	return ;
}


////////////////////
// GETTERS / SETTERS
////////////////////
int		Request::getInSock( void )
{
	return (this->_inSocket);
}

std::string		Request::getMethod()
{
	return (this->_method);
}

std::string		Request::getTarget()
{
	return (this->_target);
}

std::string		Request::getRelativeTargetPath()
{
	return (this->_relativeTargetPath);
}

std::string		Request::getAbsoluteTargetPath()
{
	return (this->_absoluteTargetPath);
}

void			Request::updateTarget(std::string target)
{
	this->_target = target;
	this->createPath();
}
