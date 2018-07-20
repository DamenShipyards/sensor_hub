#include <iostream>
#include <Windows.h>
#include <tchar.h>

SERVICE_STATUS g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE;
HANDLE g_ServiceEventSource = NULL;

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler (DWORD);
DWORD WINAPI ServiceWorkerThread (LPVOID lpParam);

#define SERVICE_NAME  _T("Damen Sensor Hub")
#define check_error(var) \
  if ((var) == 0) { \
    ret = GetLastError(); \
    goto exit; \
  }
#define check_noerror(retval) \
  if ((retval) != 0) { \
    goto exit; \
  }
#define  kServiceDependencies TEXT("Tcpip\0\0")


void print_usage()
{
  std::cout << "Usage:" << std::endl 
            << " sensor_hub <command>" << std::endl
            << "   Commands:" << std::endl
            << "     install: Install the service" << std::endl
            << "     uninstall: Remove the service" << std::endl
            << "     status: Report service status" << std::endl;                
}


static int ServiceSetupEventLogging()
{
  int ret;
  HKEY key;
  LPCTSTR s;
  DWORD	typesSupported;
  TCHAR path[MAX_PATH];
  DWORD	n;

  key = NULL;

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


int _tmain (int argc, TCHAR *argv[])
{
  OutputDebugString(_T("Damen Sensor Hub: Main: Entry"));

  SERVICE_TABLE_ENTRY ServiceTable[] = 
  {
    {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
    {NULL, NULL}
  };

  if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
  {
    OutputDebugString(_T("Damen Sensor Hub: Main: StartServiceCtrlDispatcher returned error"));
    print_usage();
    return GetLastError ();
  }

  OutputDebugString(_T("Damen Sensor Hub: Main: Exit"));
  return 0;
}


VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv)
{
  DWORD Status = E_FAIL;

  OutputDebugString(_T("Damen Sensor Hub: ServiceMain: Entry"));

  g_StatusHandle = RegisterServiceCtrlHandler (SERVICE_NAME, ServiceCtrlHandler);

  if (g_StatusHandle == NULL) 
  {
    OutputDebugString(_T("Damen Sensor Hub: ServiceMain: RegisterServiceCtrlHandler returned error"));
    goto EXIT;
  }

  // Tell the service controller we are starting
  ZeroMemory (&g_ServiceStatus, sizeof (g_ServiceStatus));
  g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  g_ServiceStatus.dwControlsAccepted = 0;
  g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
  g_ServiceStatus.dwWin32ExitCode = 0;
  g_ServiceStatus.dwServiceSpecificExitCode = 0;
  g_ServiceStatus.dwCheckPoint = 0;

  if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) 
  {
    OutputDebugString(_T("Damen Sensor Hub: ServiceMain: SetServiceStatus returned error"));
  }

  /* 
   * Perform tasks necessary to start the service here
   */
  OutputDebugString(_T("Damen Sensor Hub: ServiceMain: Performing Service Start Operations"));

  // Create stop event to wait on later.
  g_ServiceStopEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (g_ServiceStopEvent == NULL) 
  {
    OutputDebugString(_T("Damen Sensor Hub: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error"));

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = GetLastError();
    g_ServiceStatus.dwCheckPoint = 1;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
      OutputDebugString(_T("Damen Sensor Hub: ServiceMain: SetServiceStatus returned error"));
    }
    goto EXIT; 
  }    

  // Tell the service controller we are started
  g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
  g_ServiceStatus.dwWin32ExitCode = 0;
  g_ServiceStatus.dwCheckPoint = 0;

  if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
  {
    OutputDebugString(_T("Damen Sensor Hub: ServiceMain: SetServiceStatus returned error"));
  }

  // Start the thread that will perform the main task of the service
  HANDLE hThread = CreateThread (NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

  OutputDebugString(_T("Damen Sensor Hub: ServiceMain: Waiting for Worker Thread to complete"));

  // Wait until our worker thread exits effectively signalling that the service needs to stop
  WaitForSingleObject (hThread, INFINITE);

  OutputDebugString(_T("Damen Sensor Hub: ServiceMain: Worker Thread Stop Event signaled"));


  /* 
   * Perform any cleanup tasks
   */
  OutputDebugString(_T("Damen Sensor Hub: ServiceMain: Performing Cleanup Operations"));

  CloseHandle (g_ServiceStopEvent);

  g_ServiceStatus.dwControlsAccepted = 0;
  g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
  g_ServiceStatus.dwWin32ExitCode = 0;
  g_ServiceStatus.dwCheckPoint = 3;

  if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
  {
    OutputDebugString(_T("Damen Sensor Hub: ServiceMain: SetServiceStatus returned error"));
  }

EXIT:
  OutputDebugString(_T("Damen Sensor Hub: ServiceMain: Exit"));

  return;
}


VOID WINAPI ServiceCtrlHandler (DWORD CtrlCode)
{
  OutputDebugString(_T("Damen Sensor Hub: ServiceCtrlHandler: Entry"));

  switch (CtrlCode) 
  {
    case SERVICE_CONTROL_STOP :

      OutputDebugString(_T("Damen Sensor Hub: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request"));

      if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
        break;

      /* 
       * Perform tasks neccesary to stop the service here 
       */

      g_ServiceStatus.dwControlsAccepted = 0;
      g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
      g_ServiceStatus.dwWin32ExitCode = 0;
      g_ServiceStatus.dwCheckPoint = 4;

      if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
      {
        OutputDebugString(_T("Damen Sensor Hub: ServiceCtrlHandler: SetServiceStatus returned error"));
      }

      // This will signal the worker thread to start shutting down
      SetEvent (g_ServiceStopEvent);

      break;

    default:
      break;
  }

  OutputDebugString(_T("Damen Sensor Hub: ServiceCtrlHandler: Exit"));
}


DWORD WINAPI ServiceWorkerThread (LPVOID lpParam)
{
  OutputDebugString(_T("Damen Sensor Hub: ServiceWorkerThread: Entry"));

  //  Periodically check if the service has been requested to stop
  while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
  {        
    /* 
     * Perform main service function here
     */

    //  Simulate some work by sleeping
    Sleep(3000);
  }

  OutputDebugString(_T("Damen Sensor Hub: ServiceWorkerThread: Exit"));

  return ERROR_SUCCESS;
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
  check_error(size);

  // Create the service and start it.
  scm = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
  check_error(scm);

  service = CreateService( scm, inName, inDisplayName, SERVICE_ALL_ACCESS, SERVICE_WIN32_SHARE_PROCESS, 
      SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, fullPath, NULL, NULL, kServiceDependencies, 
      NULL, NULL );
  check_error(service);

  if(inDescription)
  {
    ret = SetServiceInfo(scm, inName, inDescription);
    check_noerror(ret);
  }

  ok = StartService( service, 0, NULL );
  check_error(ok);

  ReportStatus(EVENTLOG_SUCCESS, "Installed service\n" );

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

  ReportStatus(EVENTLOG_SUCCESS, "Removed service\n");
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



