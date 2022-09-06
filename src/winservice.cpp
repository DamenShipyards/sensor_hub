/**
 * \file winservice.cpp
 * \brief Provide main through a windows service application
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2019 Damen Shipyards
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


#include <iostream>
#include <tchar.h>
#include <exception>
#include <locale>
#include <codecvt>

#include "main.h"
#include "configuration.h"

#include <Windows.h>

typedef std::wstring_convert<std::codecvt_utf8_utf16<TCHAR>, TCHAR> utf_converter;

SERVICE_STATUS g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE g_ServiceEventSource = NULL;

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler (DWORD);
DWORD WINAPI ServiceWorkerThread (LPVOID lpParam);

#define SERVICE_NAME  _T("Sensor Hub")
#define SERVICE_DESCR _T("Processes and redistributes sensor data")
#define check_error(var) \
  if ((var) == 0) { \
    ret = GetLastError(); \
    goto exit; \
  }
#define check_noerror(retval) \
  if ((retval) != 0) { \
    goto exit; \
  }
#define SERVICE_DEPENDENCIES TEXT("Tcpip\0\0")


void print_usage(const po::options_description& desc_args)
{
  std::cout << "Usage: sensor_hub [options] [<command>]" << std::endl
            << "Install or remove service" << std::endl
            << "Command:" << std::endl
            << "  install               Install the service" << std::endl
            << "  uninstall             Remove the service" << std::endl
            << "  update_config         Update configuration file" << std::endl
            << desc_args << std::endl;
}

void print_error(int error)
{
  TCHAR err[256];
  FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error,
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 255, NULL);
  _tprintf(_T("Error: %s\n"), err);
}

static int ServiceSetupEventLogging()
{
  int ret;
  HKEY key = NULL;
  LPCTSTR s;
  DWORD	typesSupported;
  TCHAR path[MAX_PATH];
  DWORD	n;

  // Add/Open source name as a sub-key under the Application key in the EventLog registry key.
  s = TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\") SERVICE_NAME;
  ret = RegCreateKey( HKEY_LOCAL_MACHINE, s, &key );
  check_noerror(ret);

  // Add the name to the EventMessageFile subkey.
  path[0] = '\0';
  GetModuleFileName(NULL, path, MAX_PATH);
  n = (DWORD) ((_tcslen(path) + 1) * sizeof(TCHAR));
  ret = RegSetValueEx(key, TEXT("EventMessageFile"), 0, REG_EXPAND_SZ, (const LPBYTE) path, n);
  check_noerror(ret);

  // Set the supported event types in the TypesSupported subkey.
  typesSupported = 0
  				 | EVENTLOG_SUCCESS
  				 | EVENTLOG_ERROR_TYPE
  				 | EVENTLOG_WARNING_TYPE
  				 | EVENTLOG_INFORMATION_TYPE
  				 | EVENTLOG_AUDIT_SUCCESS
  				 | EVENTLOG_AUDIT_FAILURE;
  ret = RegSetValueEx(key, TEXT("TypesSupported"), 0, REG_DWORD, (const LPBYTE) &typesSupported, sizeof(DWORD));
  check_noerror(ret);

  // Set up the event source.
  g_ServiceEventSource = RegisterEventSource(NULL, SERVICE_NAME);
  check_error(g_ServiceEventSource);

exit:
  if(key)
  {
  	RegCloseKey(key);
  }
  return ret;
}


static void ReportStatus(int inType, const char *inFormat, ...)
{
  va_list args;
  DWORD evtId = 0;  // Could be used to specify more info about the message

  va_start(args, inFormat);

  if(g_ServiceEventSource)
  {
    char s[1024];
    const char *array[1];

    vsprintf_s(s, inFormat, args);
    array[0] = s;
    ReportEventA(g_ServiceEventSource, (WORD) inType, 0, evtId, NULL, 1, 0, array, NULL);
  }
  else
  {
    vfprintf(stderr, inFormat, args);
  }
  va_end(args);
}



VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv)
{
  DWORD Status = E_FAIL;

  g_StatusHandle = RegisterServiceCtrlHandler (SERVICE_NAME, ServiceCtrlHandler);
  if (g_StatusHandle == NULL) {
    return;
  }

  // Tell the service controller we are starting
  ZeroMemory (&g_ServiceStatus, sizeof (g_ServiceStatus));
  g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  g_ServiceStatus.dwControlsAccepted = 0;
  g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
  g_ServiceStatus.dwWin32ExitCode = 0;
  g_ServiceStatus.dwServiceSpecificExitCode = 0;
  g_ServiceStatus.dwCheckPoint = 0;

  log(level::info, "Starting service");
  SetServiceStatus(g_StatusHandle, &g_ServiceStatus);


  // Tell the service controller we are started
  g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
  g_ServiceStatus.dwWin32ExitCode = 0;
  g_ServiceStatus.dwCheckPoint = 0;
  ReportStatus(EVENTLOG_SUCCESS, "Sensor Hub started.\n");
  log(level::info, "Service started");
  SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

  // Start the thread that will perform the main task of the service
  HANDLE hThread = CreateThread (NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

  // Wait until our worker thread exits
  WaitForSingleObject (hThread, INFINITE);

  g_ServiceStatus.dwControlsAccepted = 0;
  g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
  g_ServiceStatus.dwWin32ExitCode = 0;
  g_ServiceStatus.dwCheckPoint = 3;
  ReportStatus(EVENTLOG_SUCCESS, "Sensor Hub stopped.\n");
  log(level::info, "Service stopped");
  SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}


VOID WINAPI ServiceCtrlHandler (DWORD CtrlCode)
{
  switch (CtrlCode)
  {
    case SERVICE_CONTROL_INTERROGATE:
      SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
      break;

    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
      if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
        break;

      // Get ready to stop the service
      g_ServiceStatus.dwControlsAccepted = 0;
      g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
      g_ServiceStatus.dwWin32ExitCode = 0;
      g_ServiceStatus.dwCheckPoint = 4;

      log(level::info, "Stopping service");
      SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

      // This will signal the worker thread to start shutting down
      stop_loop();
      break;

    default:
      break;
  }
}


DWORD WINAPI ServiceWorkerThread (LPVOID lpParam)
{
  DWORD result = ERROR_SUCCESS;
  try {
    result = enter_loop();
  }
  catch (std::exception& e) {
    log(level::error, "Exception from main loop: %", e.what());
    ReportStatus(EVENTLOG_ERROR_TYPE, "Unexpected error in Sensor Hub: %s", e.what());
    result = ERROR_EXCEPTION_IN_SERVICE;
  }

  return result;
}


static int SetServiceInfo(SC_HANDLE inSCM, LPCTSTR inServiceName, LPCTSTR inDescription)
{
  int ret;
  SC_LOCK lock;
  SC_HANDLE service;
  SERVICE_DESCRIPTION description;
  SERVICE_FAILURE_ACTIONS actions;
  SC_ACTION action;
  BOOL ok;

  // Open the database (if not provided) and lock it to prevent other access while re-configuring.
  if( !inSCM )
  {
    inSCM = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    check_error(inSCM);
  }

  lock = LockServiceDatabase(inSCM);
  check_error(lock);

  // Open a handle to the service.
  service = OpenService(inSCM, inServiceName, SERVICE_CHANGE_CONFIG|SERVICE_START);
  check_error(service);

  // Change the description.
  description.lpDescription = (LPTSTR) inDescription;
  ok = ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &description);
  check_error(ok);

  actions.dwResetPeriod = INFINITE;
  actions.lpRebootMsg = NULL;
  actions.lpCommand = NULL;
  actions.cActions = 1;
  actions.lpsaActions =  &action;
  action.Delay =  500;
  action.Type = SC_ACTION_RESTART;

  ok = ChangeServiceConfig2(service, SERVICE_CONFIG_FAILURE_ACTIONS, &actions);
  check_error(ok);

  ret = ERROR_SUCCESS;

exit:
  // Close the service and release the lock.
  if(service)
  {
    CloseServiceHandle(service);
  }
  if(lock)
  {
    UnlockServiceDatabase(lock);
  }
  return ret;
}


static int InstallService(LPCTSTR inName, LPCTSTR inDisplayName, LPCTSTR inDescription, LPCTSTR inPath)
{
  SC_HANDLE scm, service;
  BOOL ok;
  TCHAR fullPath[ MAX_PATH ];
  TCHAR *namePtr;
  DWORD size;
  int ret = ERROR_SUCCESS;

  scm = NULL;
  service = NULL;

  size = GetFullPathName(inPath, MAX_PATH, fullPath, &namePtr);
  if (size == 0)
    return GetLastError();

  std::basic_string<TCHAR> cmd_line = _T("\"");
  cmd_line += fullPath;
  cmd_line += _T("\" service");

  // Create the service and start it.
  scm = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
  check_error(scm);

  service = CreateService( scm, inName, inDisplayName, SERVICE_ALL_ACCESS, SERVICE_WIN32_SHARE_PROCESS,
      SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, cmd_line.c_str(), NULL, NULL, SERVICE_DEPENDENCIES,
      NULL, NULL );
  check_error(service);

  if(inDescription)
  {
    ret = SetServiceInfo(scm, inName, inDescription);
    check_noerror(ret);
  }

  ok = StartService( service, 0, NULL );
  check_error(ok);

  ReportStatus(EVENTLOG_SUCCESS, "Installed Sensor Hub service\n" );

exit:
  if(service)
  {
    CloseServiceHandle(service);
  }
  if(scm)
  {
    CloseServiceHandle(scm);
  }
  return ret;
}


static int RemoveService(LPCTSTR inName)
{
  int ret;
  SC_HANDLE scm, service;
  BOOL ok;
  SERVICE_STATUS status;

  // Open a connection to the service.

  scm = OpenSCManager( 0, 0, SC_MANAGER_ALL_ACCESS );
  check_error(scm);

  service = OpenService(scm, inName, SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
  check_error(service);

  // Stop the service, if it is not already stopped, then delete it.
  ok = QueryServiceStatus(service, &status);
  check_error(ok);

  if(status.dwCurrentState != SERVICE_STOPPED)
  {
    ok = ControlService(service, SERVICE_CONTROL_STOP, &status);
    check_error(ok);
  }

  ok = DeleteService(service);
  check_error(ok);

  ReportStatus(EVENTLOG_SUCCESS, "Removed Sensor Hub service\n");
  ret = ERROR_SUCCESS;

exit:
  if(service)
  {
    CloseServiceHandle(service);
  }
  if(scm)
  {
    CloseServiceHandle(scm);
  }
  return ret;
}


po::variables_map vm;

const po::variables_map& get_program_options() {
  return vm;
}

int run_service() {
  ServiceSetupEventLogging();
  SERVICE_TABLE_ENTRY ServiceTable[] =
  {
    {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
    {NULL, NULL}
  };
  if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
  {
    auto err = GetLastError();
    log(level::error, "Failed to start service control dispatcher: %", err);
    ReportStatus(EVENTLOG_ERROR_TYPE, "Failed to start service control dispatcher: %d", err);
    return err;
  }
  return ERROR_SUCCESS;
}

int _tmain (int argc, TCHAR *argv[])
{
  fs::path program_exe{ argv[0] };
  program_exe = fs::canonical(program_exe);
  program_exe.make_preferred();

  int ret = ERROR_SUCCESS;

  try {
    po::options_description desc_args{ "Options" };
    desc_args.add_options()
      ("help,h", "display this help and exit")
      ("version,v", "display version info and exit");

    po::options_description command_args{ "Commands" };
    command_args.add_options()
      ("command", po::value<std::string>(), "install|uninstall|update_config");

    po::positional_options_description pos_args;
    pos_args.add("command", -1);

    po::options_description command_line;
    command_line.add(desc_args).add(command_args);

    po::store(po::wcommand_line_parser(argc, argv)
      .options(command_line).positional(pos_args).run(), vm);
    po::notify(vm);

    if (vm.empty()) {
      std::cerr << "Missing parameter." << std::endl;
      print_usage(desc_args);
      return INVALID_COMMAND_LINE + 0x4000;
    }

    bool help = vm.count("help") > 0;
    bool version = vm.count("version") > 0;

    if (version) {
      print_version();
      return PROGRAM_SUCCESS;
    }
    else if (help) {
      print_usage(desc_args);
      return PROGRAM_SUCCESS;
    }
    else if (vm.count("command") == 0) {
      std::cerr << "Missing command." << std::endl;
      print_usage(desc_args);
      return INVALID_COMMAND_LINE + 0x4000;
    }
    else if (vm.count("command") > 1) {
      std::cerr << "Too many commands." << std::endl;
      print_usage(desc_args);
      return INVALID_COMMAND_LINE + 0x4000;
    }
    else if (vm["command"].as<std::string>() == "install") {
      ret = InstallService(SERVICE_NAME, SERVICE_NAME, SERVICE_DESCR, argv[0]);
      if (ret != 0) {
        print_error(ret);
        std::cerr << "Failed to install service: " << ret << std::endl;
      }
      else {
        std::cout << "Service installed." << std::endl;
      }
      return ret;
    }
    else if (vm["command"].as<std::string>() == "uninstall") {
      ret = RemoveService(SERVICE_NAME);
      if (ret != 0) {
        print_error(ret);
        std::cerr << "Failed to remove service: " << ret << std::endl;
      }
      else {
        std::cout << "Service removed." << std::endl;
      }
      return ret;
    }
    else if (vm["command"].as<std::string>() == "update_config") {
      update_config();
      std::cout << "Configuration updated." << std::endl;
      return PROGRAM_SUCCESS;
    }
    else if (vm["command"].as<std::string>() == "service") {
      log(level::info, "Starting %", program_exe);
      log(level::info, "Version %, build type: %", STRINGIFY(VERSION), STRINGIFY(BUILD_TYPE));
      ret = run_service();
      log(level::info, "Exiting main: %", ret);
      return ret;
    }
    else {
      std::cerr << "Invalid command: " << vm["command"].as<std::string>() << "." << std::endl;
      print_usage(desc_args);
      return INVALID_COMMAND_LINE + 0x4000;
    }

    return PROGRAM_SUCCESS;
  }
  catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return UNHANDLED_EXCEPTION + 0x4000;
  }
  catch (...) {
    std::cerr << "Unknown error." << std::endl;
    return UNKNOWN_EXCEPTION + 0x4000;
  }
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
