#include "Cluster.hpp"

Cluster::Cluster( void )
{
	return ;
}

// Cluster::Cluster( std::string name )
// {
// 	return ;
// }

Cluster::Cluster( Cluster const & src )
{
	*this = src;

	return ;
}

Cluster::~Cluster( void )
{
	return ;
}

Cluster & Cluster::operator=( Cluster const & rhs)
{
	(void)rhs;
    // this->??? = rhs.???;

	return ( *this );
}

int								Cluster::initialization( std::string fileName)
{
	this->_maxFd = 0;
	this->_config.parseFile(fileName);

	this->_serverList = this->_config.getServerVector();
	this->_nbServ = this->_serverList.size();

	FD_ZERO(&this->_master_fd);										// create a master file descriptor set and initialize it to zero

	for (int i = 0; i < this->_nbServ; i++)
	{
		Logger::Write(Logger::INFO, GRN, "server[" + std::to_string(i) + "] : creation");
		if (this->_serverList[i].initialization(i))
			return (1);
		FD_SET(this->_serverList[i].getFd(), &this->_master_fd);	// adding our first fd socket, the server one.
		if(this->_serverList[i].getFd() > this->_maxFd)				// ternaire ?? a faire
			this->_maxFd = this->_serverList[i].getFd();
	}

	this->logCluster();

	return (0);
}

int								Cluster::lanchServices( void )
{
	fd_set	copyMasterSet;
	fd_set	writingSet;

	while(1)
	{
		// We have to make a copy of the master fd set because select() will change bits
		// of moving fds  (( man 2 select ))
		copyMasterSet = this->_master_fd;
		setWritingSet(&writingSet);

			// std::cout << "\nclient[" << it->getSocket() << "] on serv " << it->getServerNb() << "\n\n";

		int	ret = select(this->_maxFd + 1, &copyMasterSet, &writingSet, 0, 0);

		if (ret < 0)
		{
			Logger::Write(Logger::ERROR, RED, "Error : select have deconné");
			throw (std::exception()); // to do : exception
			return (1); // test ???
		}
		
		std::vector<Client> listR = this->_readyClients;
		for (std::vector<Client>::iterator it = listR.begin(); it != listR.end(); it++)
		{
			if (FD_ISSET(it->getSocket(), &writingSet))
			{
				this->_serverList[it->getServerNb()].sendResponse( it->getSocket(), *it );
				it->setFinishWrite(true);
				if (it->getFinishWrite())
				{
					deleteInReadyClients(it->getSocket());
					break;
				}
			}
			else
			{
				// std::cout << it->getSocket() << " " << it->getServerNb() << " "<< it->getFinishRead();
				Logger::Write(Logger::ERROR, RED, "server[" + std::to_string(it->getServerNb()) + "] : error with send, fd[" + std::to_string(it->getSocket()) + "]");
				if (close(it->getSocket()) < 0)
					return (1);
				FD_CLR(it->getSocket(), &this->_master_fd);
				FD_CLR(it->getSocket(), &writingSet);
				this->_readyClients.erase(it);
			}
		}

		for(int i = 0; i < this->_nbServ; i++)							// We go throught every server fds to see if we have a new connection
			if (FD_ISSET(this->_serverList[i].getFd(), &copyMasterSet))	// if serv fd changed -> new connection
			{
				int	sock;
				Logger::Write(Logger::INFO, GRN, "server[" + std::to_string(i) + "] : new connection");
				if ((sock = this->_serverList[i].acceptConexion()) < 0)
					return (1); // test ???
				
			// std::cout << "sock = " << sock << "\n";

				FD_SET(sock, &this->_master_fd);	// add the new fd in the master fd set
				if (sock > this->_maxFd)			// check until where we have to select
					this->_maxFd = sock;

			// std::cout << "maxfd = " << this->_maxFd << "\n";

				this->_clients.push_back(Client(sock, i));
				break ;													// no need to check any more serv
			}

		std::vector<Client> list = this->_clients;
		for (std::vector<Client>::iterator it = list.begin() ; it != list.end() ; it++)
		{
			// std::cout << "sock and webnb = " << it->getSocket() << " " << it->getServerNb() << "\n";
			if (FD_ISSET(it->getSocket(), &copyMasterSet))
			{
				if ( !it->myRecv() )
				{
					if (close(it->getSocket()) < 0)
						return (1);
					FD_CLR(it->getSocket(), &this->_master_fd);
					deleteInClients(it->getSocket());
				}
				else
				{
					if (it->getFinishRead())
					{
						this->_readyClients.push_back(*it);
						// std::cout << *it;
					}
				}
			}
		}

	// 	for(int i = 0; i < this->_nbServ; i++)
	// 	{
	// 		std::vector<int> list = this->_serverList[i].getFdList();
	// 		for (std::vector<int>::iterator it = list.begin() ; it != list.end() ; it++)
	// 		{
	// 			if (FD_ISSET(*it, &copyMasterSet))
	// 			{
	// 				if (!this->_serverList[i].handleRequest( *it ))
	// 				{
	// 					if (close(*it) < 0)
	// 						return (1);
	// 					FD_CLR(*it, &this->_master_fd);
	// 					FD_CLR(*it, &writingSet);
	// 					deleteInFdList(*it);
	// 					this->_serverList[i].deleteSocket(*it);
	// 				}
	// 				else
	// 				{
	// 					if (FD_ISSET(*it, &writingSet))
	// 						this->_serverList[i].sendResponse( *it );
	// 					else
	// 					{
	// 						Logger::Write(Logger::ERROR, RED, "server[" + std::to_string(i) + "] : error with reading fd[" + std::to_string(*it) + "]");
	// 						if (close(*it) < 0)
	// 							return (1);
	// 						FD_CLR(*it, &copyMasterSet);
	// 						FD_CLR(*it, &writingSet);
	// 						deleteInFdList(*it);
	// 						this->_serverList[i].deleteSocket(*it);
	// 					}
	// 				}
	// 			}
	// 		}
	// 	}
	
	
	}

	return (0);
}

void								Cluster::deleteInReadyClients( int socket )
{
	for (std::vector<Client>::iterator it = this->_readyClients.begin() ; it != this->_readyClients.end() ; it++)
		if (socket == it->getSocket())
		{
			this->_readyClients.erase(it);
			break ;
		}
	
	return ;
}

void								Cluster::deleteInClients( int socket )
{
	for (std::vector<Client>::iterator it = this->_clients.begin() ; it != this->_clients.end() ; it++)
		if (socket == it->getSocket())
		{
			this->_clients.erase(it);
			break ;
		}
	
	return ;
}

void							Cluster::setWritingSet( fd_set *writefds )
{
	FD_ZERO(writefds);
	std::vector<Client> list = this->_readyClients;
	for (std::vector<Client>::iterator it = list.begin() ; it != list.end() ; it++)
		FD_SET(it->getSocket(), writefds);

	return ;
}

std::map<std::string, std::string>	Cluster::getMap( void )
{
	return (this->_config.getConfigMap());
}

fd_set								Cluster::getMasterSet( void )
{
	return (this->_master_fd);
}

std::vector<Webserv>				Cluster::getServerList( void )
{
	return (this->_serverList);
}

int									Cluster::getMaxFd( void )
{
	return (this->_maxFd);
}

std::vector<int>					Cluster::getFdList( void )
{
	return (this->_fdList);
}

void								Cluster::logCluster(void)
{
	for (std::vector<Webserv>::iterator it = this->_serverList.begin(); it != this->_serverList.end(); ++it)
		it->logWebserv();
}