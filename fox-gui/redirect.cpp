
#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "redirect.h"

#define READ_FD 0
#define WRITE_FD 1

#define CHECK(a) if ((a)!= 0) return -1;

stdout_redirect::stdout_redirect(int bufferSize)
{
    int status;

#ifdef WIN32
    status = _pipe(fd_pipe, bufferSize, O_TEXT);
#else
    status = pipe(fd_pipe);
#endif

    assert(status == 0);

    fd_stdout = dup(fileno(stdout));
}

int stdout_redirect::start()
{
    fflush(stdout);
    CHECK(dup2(fd_pipe[WRITE_FD], STDOUT_FILENO));
#ifdef WIN32
    FILE* of = _fdopen(STDOUT_FILENO, "w");
    *stdout = *of;

    setvbuf(stdout, NULL, _IONBF, 0); // absolutely needed

    HANDLE ofh = (HANDLE) _get_osfhandle(STDOUT_FILENO);
    SetStdHandle(STD_OUTPUT_HANDLE, ofh);
#else
    setvbuf(stdout, NULL, _IONBF, 0); // absolutely needed
#endif
    return 0;
}

int stdout_redirect::stop()
{
    CHECK(dup2(fd_stdout, fileno(stdout)));
    close(fd_stdout);
    close(fd_pipe[WRITE_FD]);
    close(fd_pipe[READ_FD]);
    return 0;
}

int stdout_redirect::read(char *buffer, int size)
{
    int nOutRead = ::read(fd_pipe[READ_FD], buffer, size);
    return nOutRead;
}
