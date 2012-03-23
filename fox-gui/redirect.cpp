
#include <windows.h>
 
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <assert.h>
#include "redirect.h"
 
#ifndef _USE_OLD_IOSTREAMS
using namespace std;
#endif
 
#define READ_FD 0
#define WRITE_FD 1
 
#define CHECK(a) if ((a)!= 0) return -1;
 
stdout_redirect::~stdout_redirect()
{
        _close(fd_stdout);
        _close(fd_pipe[WRITE_FD]);
        _close(fd_pipe[READ_FD]);
}

stdout_redirect::stdout_redirect(int bufferSize)
{
        if (_pipe(fd_pipe, bufferSize, O_TEXT)!=0)
        {
	  assert(0);
        }
	//	int val = _fileno(stderr);
        fd_stdout = _dup(_fileno(stdout));
}
 
int stdout_redirect::start()
{
        fflush( stdout );
        CHECK(_dup2(fd_pipe[WRITE_FD], _fileno(stdout)));
        ios::sync_with_stdio();
        setvbuf( stdout, NULL, _IONBF, 0 ); // absolutely needed
        return 0;
}
 
int stdout_redirect::stop()
{
        CHECK(_dup2(fd_stdout, _fileno(stdout)));
        ios::sync_with_stdio();
        return 0;
}
 
int stdout_redirect::read(char *buffer, int size)
{
        int nOutRead = _read(fd_pipe[READ_FD], buffer, size);
        buffer[nOutRead] = '\0';
        return nOutRead;
}
