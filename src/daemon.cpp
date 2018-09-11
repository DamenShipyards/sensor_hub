#include "log.h"
#include "loop.h"

#include <exception>
#include <csignal>
#include <cstdlib>

#include <boost/filesystem.hpp>


namespace fs = boost::filesystem;
using pth = boost::filesystem::path;


void segv_handler(int s) {
  std::cout << std::endl << "Caught Segmentation Fault" << std::endl;
  exit(s);
}

void ctrl_c_handler(int s) {
  log(level::info, "Caught CTRL-C");
  stop_loop();
  std::cout << std::endl << "Caught CTRL-C" << std::endl;
  exit(s);
}

int main(int argc, char* argv[])
{
  struct sigaction sig_int_handler;
  struct sigaction sig_segv_handler;

  sig_int_handler.sa_handler = ctrl_c_handler;
  sigemptyset(&sig_int_handler.sa_mask);
  sig_int_handler.sa_flags = 0;

  sigaction(SIGINT, &sig_int_handler, NULL);

  sig_segv_handler.sa_handler = segv_handler;
  sigemptyset(&sig_segv_handler.sa_mask);
  sig_segv_handler.sa_flags = 0;

  sigaction(SIGSEGV, &sig_segv_handler, NULL);

  pth p{argv[0]};
  p = fs::canonical(p);

  log(level::info, "****************************************");
  log(level::info, "Starting %", p);
  
  int result = 0;
  try {
    result = enter_loop();
  }
  catch(std::exception& e) {
    log(level::error, "Exception: %", e.what());
    std::cout << e.what() << std::endl;
    result =  1;
  }
  catch(...) {
    result = 2;
  }
  stop_loop();
  return result;
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
