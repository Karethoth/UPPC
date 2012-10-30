#ifndef __SUBCLIENT_HH__
#define __SUBCLIENT_HH__

#include <iostream>


class SubClient
{
 private:
  std::string    ipString;
  unsigned short port;

  int connection;

 protected:
  std::string GenerateIPString( unsigned int );

 public:
  SubClient( unsigned int, unsigned short );
  bool Connect();
  void Close();

  bool Write( char*, size_t );
  std::string Read();
};

#endif

