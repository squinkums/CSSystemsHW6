#include <iostream>
#include "handler.hh"
#include <cpprest/uri_builder.h>
#include <codecvt>

using namespace std;
using namespace web;
using namespace http;
using namespace utility;




std::unique_ptr<handler> g_httpHandler;



void on_shutdown()
{
    g_httpHandler->close().wait();
}


int main(int argc, char* argv[])
{
        // Task: use getopt to get those maxmem, server, port, and threads from command line (instead of me declaring it in the main function)                         

  unsigned int maxmem = 0;
  std::string server;
  std::string port;
  unsigned int threads;

  int c;
  while((c = getopt(argc, argv, "m:s:p:t:")) != -1) {
    switch(c) {
      case 'm':
        if(optarg) maxmem = std::atoi(optarg);
        break;
      case 's':
        if(optarg) server = optarg;
        break;
      case 'p':
        if(optarg) port = optarg;
        break;
      case 't':
        if(optarg) threads = std::atoi(optarg);
        break;
    }

  //auto maxmem = 100;                                                                                                                                                
  //std::string server = "http://127.0.0.1";                                                                                                                          
  //std::string port = "34568";                                                                                                                                       
  //auto threads = 0;                                                                                                                                                 




  using convert_type = codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_type, wchar_t> converter;

  server.append(":");
  server.append(port);
  std::unique_ptr<Cache> my_cache(new Cache(maxmem, 0.75, nullptr, std::hash<key_type>()));
  g_httpHandler = std::unique_ptr<handler>(new handler(server,my_cache));
  g_httpHandler->open().wait();

  ucout << utility::string_t(U("Listening for requests at: ")) << server << std::endl;
  std::cout << "Threads: " << threads << std::endl;
  std::cout << "Press ENTER to exit." << std::endl;

  std::string line;
  std::getline(std::cin, line);

  on_shutdown();
  return 0;
}


