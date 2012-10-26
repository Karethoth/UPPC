#include "client.hh"
#include "callbacks.hh"

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
    perror("connect failed");
    //return false;
  }

  std::cout << "Connected!\n";

  bev = bufferevent_socket_new( base, connection, BEV_OPT_CLOSE_ON_FREE );
  bufferevent_setcb( bev, ReadCB, NULL, ErrorCB, (void*)this );
  bufferevent_enable( bev, EV_READ|EV_WRITE );

  struct event *connection_event;

  input  =  bufferevent_get_input( bev );
  output =  bufferevent_get_output( bev );

  return true;
}


void Client::Run()
{
  std::cout << "Running..\n";
  event_base_loop( base, 0x04 );
  std::cout << "stopped.\n";
}



bool Client::HandleMessage( string &msg )
{
  std::cout << "Handling message: '" << msg << "'\n";
}

