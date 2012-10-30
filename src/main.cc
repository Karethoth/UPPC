#include "client.hh"

#include <ctime>
#include <cstdlib>


int main( int c, char *v[] )
{
  srand( time( NULL ) );
  setvbuf( stdout, NULL, _IONBF, 0 );

  Client client;
  if( !client.Connect( "localhost", 40000 ) )
  {
    std::cout << "Connect() failed, stopping..\n";
    return -1;
  }

  client.SetPool( "6b62bfbd3f5931f093fb04f1699312369ad3ef64" );
  client.Run();

  return 0;
}

