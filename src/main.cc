#include "client.hh"

#include <ctime>
#include <cstdlib>


int main( int c, char *v[] )
{
  srand( time( NULL ) );
  setvbuf( stdout, NULL, _IONBF, 0 );

  Client client;
  client.Connect( "localhost", 40000 );
  //client.JoinPool( "d4d8f95cc9b655390fdc9f3bc2d695fae36cd1d7" );
  client.Run();

  return 0;
}

