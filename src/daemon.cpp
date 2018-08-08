#include "log.h"
#include "loop.h"

#include <boost/filesystem.hpp>


namespace fs = boost::filesystem;
using pth = boost::filesystem::path;


int main(int argc, char* argv[])
{
  pth p{argv[0]};
  p = fs::canonical(p);

  log(level::info, "Starting %", p);
  
  return enter_loop();
}
