#include "client.hh"

#include <ctime>
#include <cstdlib>


int main( int c, char *v[] )
{
  srand( time( NULL ) );
  setvbuf( stdout, NULL, _IONBF, 0 );

  Client client;
  if( !client.Connect( "127.0.0.1", 40000 ) )
  {
    std::cout << "Connect() failed, stopping..\n";
    return -1;
  }

  client.SetPool( "62e97cfdff8a7b4642130fef6870a3b5212d32e5" );
  client.Run();

  return 0;
}

