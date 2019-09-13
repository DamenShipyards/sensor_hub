#include <exception>
#include <boost/filesystem.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>

#include <cstdlib>
#include <cstdio>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>

#include "log.h"
#include "loop.h"
#include "tools.h"
#include "version.h"

// Return codes
#define PROGRAM_SUCCESS 0
#define INVALID_COMMAND_LINE 1000
#define UNHANDLED_EXCEPTION 1001
#define UNKNOWN_EXCEPTION 1002
#define DAEMON_ALREADY_RUNNING 1003
#define STOP_DAEMON_FAILED 1004
#define FORK_FAILURE 1005
#define DAEMON_INIT_FAILURE 1006
#define DAEMON_NOT_RUNNING 1007
#define PID_ERROR 1008


namespace fs = boost::filesystem;
namespace po = boost::program_options;

const fs::path pid_file_default_name = "/var/run/sensor_hub.pid";


void print_usage(po::options_description& options_description) {
  std::cout << "Usage: sensor_hub <options> [start|stop]" << std::endl;
  std::cout << "Start or stop the Damen Sensor Hub daemon." << std::endl;
  std::cout << std::endl;
  std::cout << "  start                 start a daemon" << std::endl;
  std::cout << "  stop                  stop a running daemon" << std::endl;
  std::cout << std::endl;
  std::cout << options_description << std::endl;
}


struct Pid_error: public std::runtime_error {
  using runtime_error::runtime_error;
};


struct Pid_file {
  Pid_file(const fs::path& fpath): fpath_(fpath), fd_(0) {
    int flags = O_RDWR | O_NOCTTY;
    if (!fs::exists(fpath_))
      flags |= O_CREAT;
    fd_ = open(fpath_.c_str(), flags, 0660);
    if (fd_ < 0) {
      throw Pid_error("Failed to create/open lock file: " + fpath.string());
    }
    if (flock(fd_, LOCK_EX | LOCK_NB)) {
      int err = errno;
      close(fd_);
      switch (err) {
        case EWOULDBLOCK:
          throw Pid_error("Pid file locked. Daemon already running?"); 
        default:
          throw Pid_error("Failed to get pid file lock."); 
      }
    }
    pid_t pid = getpid();
    char pids[32];
    snprintf(pids, 32, "%d", pid);
    write(fd_, pids, strlen(pids));
  }
  ~Pid_file() {
    if (fd_ > 0) {
      close(fd_);
    }
    boost::system::error_code ec;
    fs::remove(fpath_, ec);
  }
private:
  fs::path fpath_;
  int fd_;
};


int main(int argc, char* argv[])
{
  try {
    fs::path p{argv[0]};
    p = fs::canonical(p);

    po::options_description desc{"Options"};
    desc.add_options()
      ("help,h", "display this help and exit")
      ("version,v", "display version info and exit")
      ("pidfile,p", "alternative to default file")
      ("update-config", "update the configuration file");

    po::positional_options_description pos_desc;
    pos_desc.add("start|stop", -1);


    bool show_version = false;
    bool show_help = false;
    bool start = false;
    bool stop = false;
    fs::path pid_file_name = pid_file_default_name;

    for (int i = 0; i < argc; ++i) {
      if (std::string(argv[i]) == "start") {
        start = true;
      }
      if (std::string(argv[i]) == "stop") {
        stop = true;
      }
      if (std::string(argv[i]) == "--help") {
        show_help = true;
      }
      if (std::string(argv[i]) == "--version") {
        show_version = true;
      }
      if (std::string(argv[i]) == "--pidfile") {
        ++i;
        if (i < argc) {
          pid_file_name = argv[i];
        }
        else {
          print_usage(desc);
          return INVALID_COMMAND_LINE;
        }
      }
    }

    if (show_version) {
      print_version();
      return PROGRAM_SUCCESS;
    }

    if (show_help || (!start && !stop)) {
      print_usage(desc);
      return show_help ? PROGRAM_SUCCESS : INVALID_COMMAND_LINE; 
    }

    if (stop) {
      if (!fs::exists(pid_file_name)) {
        std::cerr << "Pid file not found. Daemon not running?" << std::endl;
        return DAEMON_NOT_RUNNING;
      }
      fs::ifstream ifs{pid_file_name};
      pid_t pid;
      ifs >> pid;
      ifs.close();
      if (kill(pid, SIGINT) == 0) {
        return PROGRAM_SUCCESS;
      }
      else {
        std::cerr << "Failed to stop daemon." << std::endl;
        return STOP_DAEMON_FAILED;
      }
    }

    // Daemonize
    {
      pid_t pid;
      if ((pid = fork()) < 0) {
        std::cerr << "Fork failure." << std::endl;
        return FORK_FAILURE;
      }
      else if (pid != 0)
        // We're the parent -> exit
        return PROGRAM_SUCCESS;

      // Child continues here: become session leader
      setsid();
      // Change dir to root to prevent any accidental dir locking
      chdir("/");
      // Clear umask so we can create files with any permission
      umask(0);

      // Fork again (paranoid fork) to avoid acquiring a new terminal
      if ((pid = fork()) < 0) {
        std::cerr << "Fork failure." << std::endl;
        return FORK_FAILURE;
      }
      else if (pid != 0)
        // Parent -> exit
        return PROGRAM_SUCCESS;
    }

    {
      // Redirect standard file descriptors
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);

      // No standard input
      if(!freopen("/dev/null", "r", stdin)) {
        log(level::error, "Failed to redirect standard input from /dev/null: %", strerror(errno));
        return DAEMON_INIT_FAILURE;
      }
      // ... and standard out/err to tmp
      if(!freopen("/tmp/sensor_hub.stdout", "w", stdout)) {
        log(level::error, "Failed to redirect standard output to file: %", strerror(errno));
        return DAEMON_INIT_FAILURE;
      }
      if(!freopen("/tmp/sensor_hub.stderr", "w", stderr)) {
        log(level::error, "Failed to redirect standard error to file: %", strerror(errno));
        return DAEMON_INIT_FAILURE;
      }
    }

    {
      Pid_file pid_file{pid_file_name};
      log(level::info, "Daemon started: % version %", p, STRINGIFY(VERSION));
      int result = enter_loop();
      log(level::info, "Daemon stopped: result %", result);
      return result;
    } 
  }
  catch(Pid_error& pe) {
    log(level::error, "Pid file error: %", pe.what());
    return PID_ERROR;
  }
  catch(std::exception& e) {
    log(level::error, "Exception: %", e.what());
    std::cerr << "Exception: " <<  e.what() << std::endl;
    return UNHANDLED_EXCEPTION;
  }
  catch(...) {
    std::cerr << "Unknown exception." << std::endl;
    return UNKNOWN_EXCEPTION;
  }
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
