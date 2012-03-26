
#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
// #include <io.h>
#include <assert.h>

#include "redirect.h"

#define READ_FD 0
#define WRITE_FD 1

#define CHECK(a) if ((a)!= 0) return -1;

stdout_redirect::~stdout_redirect()
{
  close(fd_stdout);
  close(fd_pipe[WRITE_FD]);
  close(fd_pipe[READ_FD]);
}

stdout_redirect::stdout_redirect(int bufferSize)
{
  int status;

#ifdef WIN32
  status = _pipe(fd_pipe, bufferSize, O_TEXT);
#else
  status = pipe2(fd_pipe, O_NONBLOCK);
#endif

  assert(status == 0);

  fd_stdout = dup(fileno(stdout));
}

int stdout_redirect::start()
{
  fflush(stdout);
  CHECK(dup2(fd_pipe[WRITE_FD], fileno(stdout)));
  //        ios::sync_with_stdio();
  setvbuf( stdout, NULL, _IONBF, 0 ); // absolutely needed
  return 0;
}

int stdout_redirect::stop()
{
  CHECK(dup2(fd_stdout, fileno(stdout)));
  //        ios::sync_with_stdio();
  return 0;
}

int stdout_redirect::read(char *buffer, int size)
{
  int nOutRead = ::read(fd_pipe[READ_FD], buffer, size);
  return nOutRead;
}
