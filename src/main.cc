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

  client.SetPool( "0dcd10df6f295b8c3ef65e42fb4921962de4962a" );
  client.Run();

  return 0;
}

