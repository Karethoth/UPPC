#include "callbacks.hh"

#include "client.hh"
//#include "sha1.hh"

using std::string;


void ErrorCB( struct bufferevent*, short, void* );
void ReadCB( struct bufferevent*, void* );



void ErrorCB( struct bufferevent *bev, short error, void *ctx )
{
  std::cout << "ERRRORORORORO\n";
  Client *client = (Client*)ctx;
  //std::cout << "Error by '" << client->id << "'\n";
  client->state = ERROR;
  if( error & BEV_EVENT_EOF )
  {
    /* connection has been closed, do any clean up here */
    /* ... */
  } else if (error & BEV_EVENT_ERROR) {
    /* check errno to see what error occurred */
    /* ... */
  } else if (error & BEV_EVENT_TIMEOUT) {
    /* must be a timeout event handle, handle it */
    /* ... */
  }

  bufferevent_free( bev );
}



void ReadCB( struct bufferevent *bev, void *ctx )
{
  Client *client = (Client*)ctx;

  char *line;
  size_t n;
  int i;

  unsigned char hash[20];
  char hex[41];
  int len;

  while( (line = evbuffer_readln( client->input, &n, EVBUFFER_EOL_LF )) )
  {
    //len = strlen( line )-1;
    //sha1::calc( line, len, hash );
    //sha1::toHexString( hash, hex );
    //evbuffer_add( client->output, "\n", 1 );
    //evbuffer_add( client->output, hex, 40 );
    //evbuffer_add( client->output, "\n", 1 );

    string msg( line );
    client->HandleMessage( msg );

    free( line );
  }

  /*
  if( evbuffer_get_length( client->input ) >= 16000 )
  {
    char buf[1024];
    while( evbuffer_get_length( client->input ) )
    {
      int n = evbuffer_remove( client->input, buf, sizeof(buf) );
      evbuffer_add( client->output, buf, n );
    }
    evbuffer_add( client->output, "\n", 1 );
  }
  */
}

