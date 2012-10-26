#ifndef __CLIENT_HH__
#define __CLIENT_HH__

#include <vector>
#include <event2/event.h>
#include <iostream>


struct sPackage
{
  std::string    poolClientID;
  unsigned int   requestID;
  unsigned int   ip;
  unsigned short port;
  unsigned short dataLen;
  unsigned char *data;
};


enum eState
{
  NOT_CONNECTED,
  CONNECTED,
  IN_POOL,
  ERROR
};


class Client
{
 private:
  struct event_base *base;

 public:
  eState state;
  struct evbuffer *input, *output;

  bool Connect( std::string, int );
  //bool JoinPool( std::string );
  void Run();

  bool HandleMessage( std::string& );
};

#endif
