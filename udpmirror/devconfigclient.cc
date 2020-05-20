#include "common.h"
#include <curl/curl.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <fstream>

CURL* curl;
static bool quit_app(false);

namespace webCURL {

  struct MemoryStruct {
    char* memory;
    size_t size;
  };

  static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb,
                                    void* userp)
  {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
      /* out of memory! */
      printf("not enough memory (realloc returned NULL)\n");
      return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
  }

} // namespace webCURL

std::string get_device_info( std::string url, const std::string& device, std::string& hash)
{
  CURLcode res;
  std::string retv;
  struct webCURL::MemoryStruct chunk;
  chunk.memory =
      (char*)malloc(1); /* will be grown as needed by the realloc above */
  chunk.size = 0;       /* no data at this point */

  url += "?dev=" + device + "&hash=" + hash;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_USERPWD, "device:device");
  /* send all data to this function  */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, webCURL::WriteMemoryCallback);
  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  /* get it! */
  res = curl_easy_perform(curl);

  /* check for errors */
  if(res == CURLE_OK) {
    retv.insert(0, chunk.memory, chunk.size);
    // printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
  }
  free(chunk.memory);

  std::stringstream ss(retv);
  std::string to;
  bool first(true);
  retv = "";
  while(std::getline(ss, to, '\n')) {
    if( first )
      hash = to;
    else
      retv += to + '\n';
    first = false;
  }
  return retv;
}

static void sighandler(int sig)
{
  quit_app = true;
}

int main(int argc, char** argv)
{
  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);
  try {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(!curl)
      throw ErrMsg("Unable to initialize curl");
    std::string lobby("localhost");
    std::string device("");
    double gracetime(10);
    const char* options = "d:hl:g:";
    struct option long_options[] = {{"device", 1, 0, 'd'},
                                    {"lobbyurl", 1, 0, 'l'},
                                    {"help", 0, 0, 'h'},
				    {"gracetime", 1, 0, 'g' },
                                    {0, 0, 0, 0}};
    int opt(0);
    int option_index(0);
    while((opt = getopt_long(argc, argv, options, long_options,
                             &option_index)) != -1) {
      switch(opt) {
      case 'h':
        app_usage("mplx_client", long_options, "");
        return 0;
      case 'd':
        device = optarg;
        break;
      case 'l':
        lobby = optarg;
        break;
      case 'g':
        gracetime = atof(optarg);
        break;
      }
    }
    std::string hash;
    FILE* h_pipe(NULL);
    while( !quit_app ) {
      std::string tsc = get_device_info(lobby, device, hash);
      if( tsc.size() ){
	// close current TASCAR session:
	if( h_pipe )
	  fclose(h_pipe);
	h_pipe = NULL;
	// write new session file:
	std::ofstream ofh("session.tsc");
	ofh << tsc;
	ofh.close();
	// reopen TASCAR:
	h_pipe = popen( "tascar_cli session.tsc", "w" );
      }
      sleep(gracetime);
    }
    // close TASCAR::
    if( h_pipe )
      fclose(h_pipe);
    h_pipe = NULL;
    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }
  catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
