#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
#elif __linux__
  #include <unistd.h>
  #include <signal.h>
#elif __APPLE__
  #include <mach-o/dyld.h>
#endif

void get_exe_filename(char *buf, int sz) {
#if _WIN32
  int len = GetModuleFileName(NULL, buf, sz - 1);
  buf[len] = '\0';
#elif __linux__
  char path[512];
  sprintf(path, "/proc/%d/exe", getpid());
  int len = readlink(path, buf, sz - 1);
  buf[len] = '\0';
#elif __APPLE__
  /* use realpath to resolve a symlink if the process was launched from one.
  ** This happens when Homebrew installs a cack and creates a symlink in
  ** /usr/local/bin for launching the executable from the command line. */
  unsigned size = sz;
  char exepath[size];
  _NSGetExecutablePath(exepath, &size);
  realpath(exepath, buf);
#else
  strcpy(buf, "./lite");
#endif
}

