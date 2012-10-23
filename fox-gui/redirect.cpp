
#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "fatal.h"
#include "redirect.h"

#define PIPE_READ 0
#define PIPE_WRITE 1

static void open_pipe(int fd[], int buffer_size)
{
    int status;
#ifdef WIN32
    status = _pipe(fd, buffer_size, O_TEXT);
#else
    status = pipe(fd);
#endif
    if (status)
        fatal_exception("error opening pipe");
}

static void dup_stdout(int fd)
{
    fflush(stdout);

    if (dup2(fd, STDOUT_FILENO) < 0)
        fatal_exception("cannot overwrite the STDOUT file descriptor");
    close(fd);

#ifdef WIN32
    FILE* of = _fdopen(STDOUT_FILENO, "w");
    *stdout = *of;

    setvbuf(stdout, NULL, _IONBF, 0);

    HANDLE ofh = (HANDLE) _get_osfhandle(STDOUT_FILENO);
    SetStdHandle(STD_OUTPUT_HANDLE, ofh);
#else
    setvbuf(stdout, NULL, _IONBF, 0);
#endif
}

static void dup_stdin(int fd)
{
    fflush(stdin);

    if (dup2(fd, STDIN_FILENO) < 0)
        fatal_exception("cannot overwrite the STDIN file descriptor");
    close(fd);

#ifdef WIN32
    FILE* f = _fdopen(STDIN_FILENO, "w");
    *stdin = *f;

    setvbuf(stdin, NULL, _IONBF, 0);

    HANDLE ifh = (HANDLE) _get_osfhandle(STDIN_FILENO);
    SetStdHandle(STD_INPUT_HANDLE, ifh);
#else
    setvbuf(stdin, NULL, _IONBF, 0);
#endif
}

io_redirect::io_redirect(int stdout_bufsize, int stdin_bufsize)
{
    open_pipe(fd_opipe, stdout_bufsize);
    open_pipe(fd_ipipe, stdin_bufsize);

    fd_stdout = dup(STDOUT_FILENO);
    fd_stdin  = dup(STDIN_FILENO);
}

void io_redirect::start()
{
    dup_stdout(fd_opipe[PIPE_WRITE]);
    dup_stdin(fd_ipipe[PIPE_READ]);
}

void io_redirect::stop()
{
    dup2(fd_stdout, STDOUT_FILENO);
    dup2(fd_stdin, STDIN_FILENO);
    close(fd_stdout);
    close(fd_stdin);
    close(fd_opipe[PIPE_READ]);
    close(fd_ipipe[PIPE_WRITE]);
}

int io_redirect::read(char *buffer, int size)
{
    int nOutRead = ::read(fd_opipe[PIPE_READ], buffer, size);
    return nOutRead;
}


int io_redirect::write(const char *buffer, int size)
{
    int nOutW = ::write(fd_ipipe[PIPE_WRITE], buffer, size);
    return nOutW;
}
