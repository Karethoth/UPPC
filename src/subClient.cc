#include "subClient.hh"
#include <sstream>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

using std::string;


SubClient::SubClient( unsigned int ip, unsigned short port )
{
  ipString   = GenerateIPString( ip );
  this->port = port;
}



bool SubClient::Connect()
{
  struct sockaddr_in serv_addr;
  struct hostent *server;

  std::cout << "Connecting to " << ipString << ":" << port << "\n";

  connection = socket( AF_INET, SOCK_STREAM, 0 );
  if( connection < 0 )
  {
    std::cout << "Couldn't open socket!\n";
    return false;
  }

  server = gethostbyname( ipString.c_str() );
  if( !server )
  {
    std::cout << "Host not found!\n";
    return false;
  }

  bzero( (char *) &serv_addr, sizeof(serv_addr) );
  serv_addr.sin_family = AF_INET;
  bcopy( (char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length );

  serv_addr.sin_port = htons( port );
  if( connect( connection, (struct sockaddr *) &serv_addr, sizeof(serv_addr) ) < 0 )
  {
    std::cout << "Connecting failed!\n";
    return false;
  }

  return true;
}



void SubClient::Close()
{
  if( connection );
    close( connection );
}



bool SubClient::Write( char *data, size_t len )
{
  write( connection, data, len );
  return true;
}



string SubClient::Read()
{
  std::ostringstream out;
  char buffer[65500];
  int n;

  n = recv( connection, buffer, 65500, 0 );
  out.write( buffer, n );

  return out.str();
}



std::string SubClient::GenerateIPString( unsigned int ip )
{
  std::ostringstream out;
  out << (ip>>24) << ".";
  out << ((ip&0x00ff0000)>>16) << ".";
  out << ((ip&0x0000ff00)>>8) << ".";
  out << ((ip&0x000000ff));

  return out.str();
}

