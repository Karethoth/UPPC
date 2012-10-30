#include "client.hh"
#include "callbacks.hh"
#include "sha1.hh"
#include "subClient.hh"

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <errno.h>
#include <sstream>

using std::vector;
using std::string;


bool Client::Connect( string host, int port )
{
  std::cout << "Connecting to " << host << ":" << port << ".\n";
  struct sockaddr_in sin;
  struct bufferevent *bev;
  struct hostent *h;

  base = event_base_new();
  if( !base )
    return false;

  h = gethostbyname( host.c_str() );
  if( !h )
  {
    std::cout << "gethostbyname failed.\n";
    state = ERROR;
    return false;
  }

  std::cout << "Found hostname!\n";

  sin.sin_family = AF_INET;
  sin.sin_addr = *(struct in_addr*)h->h_addr;
  sin.sin_port = htons( port );

  connection = socket( AF_INET, SOCK_STREAM, 0 );
  evutil_make_socket_nonblocking( connection );

#ifndef WIN32
  int one = 1;
  setsockopt( connection, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one) );
#endif

  int status = connect( connection, (struct sockaddr*)&sin, sizeof(sin) );
#ifndef WIN32
  if( status == EINPROGRESS || !status )
#else
  if( status == WSAEINPROGRESS || !status )
#endif
  {
    perror( "Connect failed" );
    state = ERROR;
    return false;
  }

  std::cout << "Connected!\n";
  state = CONNECTED;

  bev = bufferevent_socket_new( base, connection, BEV_OPT_CLOSE_ON_FREE );
  bufferevent_setcb( bev, ReadCB, NULL, ErrorCB, (void*)this );
  bufferevent_enable( bev, EV_READ|EV_WRITE );

  struct event *connection_event;

  input  =  bufferevent_get_input( bev );
  output =  bufferevent_get_output( bev );

  return true;
}



void Client::SetPool( string id )
{
  pool = id;
}



void Client::Run()
{
  event_base_loop( base, 0x04 );
}



bool Client::HandleMessage( string &msg )
{
  string cmd, data;
  size_t cmdEnd = msg.find( ":" );

  if( cmdEnd == string::npos )
    return false;

  cmd = msg.substr( 0, msg.find( ":" ) );
  data = msg.substr( cmd.length()+1, msg.length()-cmd.length() );

  if( data[data.length()-1] == '\r' )
    data = data.substr( 0, data.length()-1 );


  if( cmd.compare( "RSALT" ) == 0 )
  {
    salt = data;
    std::cout << "Got salt: '" << salt << "'.\n";

    string salted = pool;
    salted.append( salt );

    unsigned char hash[20];
    char hex[41];

    sha1::calc( salted.c_str(), salted.length(), hash );
    sha1::toHexString( hash, hex );
    string regCmd = "REG:";
    regCmd.append( hex );
    regCmd.append( "\r\n" );
    evbuffer_add( output, regCmd.c_str(), regCmd.length() );
  }
  else if( cmd.compare( "REGRESP" ) == 0 )
  {
    std::cout << "Received regresp.\n";
    if( data.compare( "POOLED" ) == 0 )
    {
      std::cout << "REG succeeded, we're in the pool now!\n";
      state = IN_POOL;
    }
    else
    {
      std::cout << "REG failed, all aboard the fail boat!\n";
      state = ERROR;
      event_base_loopbreak( base );
      close( connection );
      return false;
    }
  }
  else if( cmd.compare( "PKG" ) == 0 )
  {
    HandlePackage();
  }

  return true;
}



bool Client::HandlePackage()
{
  char buffer[65561];
  int n;

  n = evbuffer_remove( input, buffer, 65560 );

  std::cout << "READ " << n << " bytes!\n";
  std::cout << "READ " << buffer << " bytes!\n";

  struct sPackage pkg;

  char *p = buffer;

  printf( "p = %p\n", p );
  pkg.requestID = *(unsigned int*)p;
  p += 4;
  printf( "p = %p\n", p );
  pkg.ip = *(unsigned int*)p;
  p += 4;
  printf( "p = %p\n", p );
  pkg.port = *(unsigned short*)p;
  p += 2;
  printf( "p = %p\n", p );
  pkg.dataLen = *(unsigned short*)p;
  p += 2;
  printf( "p = %p\n", p );
  pkg.data = p;


  printf( "REQID:   %p\n", pkg.requestID );
  printf( "IP:      %p\n", pkg.ip );
  printf( "PORT:    %p\n", pkg.port );
  printf( "DATALEN: %p\n", pkg.dataLen );
  printf( "DATA:" );

  for( int i=0; i < pkg.dataLen; ++i )
  {
    printf( "\t%p\n", pkg.data[i] );
  }

  SubClient sc( pkg.ip, pkg.port );
  if( sc.Connect() )
  {
    sc.Write( pkg.data, pkg.dataLen );
    string resp = sc.Read();
    sc.Close();

    std::cout << "Got response: '" << resp << "'\n";

    return true;
  }

  return false;
}

