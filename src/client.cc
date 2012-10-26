#include "client.hh"
#include "callbacks.hh"
#include "sha1.hh"

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

using std::vector;
using std::string;


bool Client::Connect( string host, int port )
{
  std::cout << "Connecting to " << host << ":" << port << ".\n";
  evutil_socket_t connection;
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
  std::cout << "Handling message: '" << msg << "'\n";

  string cmd, data;
  size_t cmdEnd = msg.find( ":" );

  if( cmdEnd == string::npos )
    return false;

  cmd = msg.substr( 0, msg.find( ":" ) );
  data = msg.substr( cmd.length()+1, msg.length()-cmd.length() );

  if( data[data.length()-1] == '\r' )
    data = data.substr( 0, data.length()-1 );


  if( cmd.compare( string( "RSALT" ) ) == 0 )
  {
    salt = data;
    std::cout << "Got salt: '" << salt << "'.\n";

    string salted = pool;
    pool.append( salt );

    unsigned char hash[20];
    char hex[41];

    sha1::calc( salted.c_str(), salted.length(), hash );
    sha1::toHexString( hash, hex );
    string regCmd = "REG:";
    regCmd.append( hex );
    regCmd.append( "\n" );
    evbuffer_add( output, regCmd.c_str(), regCmd.length() );
  }

  return true;
}

