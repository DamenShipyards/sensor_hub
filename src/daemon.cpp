/**
 * \file daemon.cpp
 * \brief Provide main through a unix daemon
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2022 Orca Software
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include <exception>
#include <chrono>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/asio/signal_set.hpp>

#include <cstdlib>
#include <cstdio>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>

#include "main.h"
#include "configuration.h"

namespace fs = boost::filesystem;


void print_usage(po::options_description& options_description) {
  std::cout << "Usage: sensor_hub [<options>] <command>" << std::endl;
  std::cout << "Start, stop or restart the Sensor Hub daemon." << std::endl;
  std::cout << std::endl;
  std::cout << "Command:" << std::endl;
  std::cout << "  start                 start a daemon" << std::endl;
  std::cout << "  stop                  stop a running daemon" << std::endl;
  std::cout << "  restart               restart a running daemon" << std::endl;
  std::cout << "  update_config         update the configuration file" << std::endl;
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
    (void)write(fd_, pids, strlen(pids));
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


po::variables_map vm;

const po::variables_map& get_program_options() {
  return vm;
}


int main(int argc, char* argv[])
{
  try {
    fs::path program_exe{argv[0]};
    program_exe = fs::canonical(program_exe);

    const std::string pid_file_default = "/var/run/sensor_hub.pid";

    po::options_description desc_args{"Options"};
    desc_args.add_options()
      ("configuration,c",
           po::value<std::string>()->default_value(get_config_file().string()),
           "configuration file")
      ("help,h", "display this help and exit")
      ("pidfile,p",
           po::value<std::string>()->default_value(pid_file_default),
           "alternative to default pid file")
      ("version,v", "display version info and exit");

    po::options_description command_args{"Command"};
    command_args.add_options()
      ("command", po::value<std::vector<std::string> >(), "start|stop|restart|update_config");

    po::positional_options_description pos_args;
    pos_args.add("command", -1);

    po::options_description command_line;
    command_line.add(desc_args).add(command_args);

    po::store(po::command_line_parser(argc, argv).
        options(command_line).positional(pos_args).run(), vm);
    po::notify(vm);

    bool start = false;
    bool stop = false;
    bool update_conf = false;
    if (vm.count("command") > 0) {
      if (vm.count("command") > 1) {
        std::cerr << "More than one command given" << std::endl;
        return INVALID_COMMAND_LINE;
      }
      auto command = vm["command"].as<std::vector<std::string> >();
      start = contains(command, std::string("start"));
      stop = contains(command, std::string("stop"));
      if (contains(command, std::string("restart"))) {
        start = stop = true;
      }
      update_conf = contains(command, std::string("update_config"));
    }

    fs::path pid_file_name = vm["pidfile"].as<std::string>();

    if (vm.count("version") != 0) {
      print_version();
      return PROGRAM_SUCCESS;
    }

    if (vm.count("help") != 0 || argc == 1) {
      print_usage(desc_args);
      return vm.count("help") != 0 ? PROGRAM_SUCCESS : INVALID_COMMAND_LINE;
    }

    if (stop) {
      if (!fs::exists(pid_file_name)) {
        if (!start) {
          std::cerr << "Pid file not found. Daemon not running?" << std::endl;
          return DAEMON_NOT_RUNNING;
        }
      }
      fs::ifstream ifs{pid_file_name};
      pid_t pid;
      ifs >> pid;
      ifs.close();
      if (kill(pid, SIGINT) == 0) {
        if (!start)
          return PROGRAM_SUCCESS;
        // Sleep before restart
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
      }
      else {
        std::cerr << "Failed to stop daemon." << std::endl;
        return STOP_DAEMON_FAILED;
      }
    }

    if (vm.count("configuration") != 0) {
      set_config_file(vm["configuration"].as<std::string>());
    }

    if (update_conf) {
      update_config();
      return PROGRAM_SUCCESS;
    }

    if (!start) {
      if (vm.count("command") > 0)
        std::cerr << "Invalid command." << std::endl;
      else
        std::cerr << "Missing command." << std::endl;
      return INVALID_COMMAND_LINE;
    }

    // Daemonize
    {
      pid_t pid;
      if ((pid = fork()) < 0) {
        std::cerr << "Fork failure." << std::endl;
        return FORK_FAILURE;
      }
      else if (pid != 0) {
        // We're the grand parent -> exit, but wait a little
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        if (fs::exists(pid_file_name)) {
          // Parent -> exit
          return PROGRAM_SUCCESS;
        } else {
          std::cerr << "Failed to start daemon." << std::endl;
          return DAEMON_START_FAILURE;
        }
      }

      // Child continues here: become session leader
      setsid();
      // Change dir to root to prevent any accidental dir locking
      (void)chdir("/");
      // Clear umask so we can create files with any permission
      umask(0);

      // Fork again (paranoid fork) to avoid acquiring a new terminal
      if ((pid = fork()) < 0) {
        std::cerr << "Fork failure." << std::endl;
        return FORK_FAILURE;
      }
      else if (pid != 0) {
        // We're the parent -> exit
        return PROGRAM_SUCCESS;
      }
    }

    {
      // Only grand child gets to here: now running as daemon !
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
      log(level::info, "Daemon started: %", program_exe);
      log(level::info, "Version %, build type: %", STRINGIFY(VERSION), STRINGIFY(BUILD_TYPE));
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
