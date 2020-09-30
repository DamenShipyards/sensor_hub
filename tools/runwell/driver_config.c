#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
  setuid(0);
  system("python3 /usr/local/bin/driver_config.py");
  return 0;
}
