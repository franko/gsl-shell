#ifndef FOXGUI_REDIRECT_H
#define FOXGUI_REDIRECT_H

class stdout_redirect
{
public:
    stdout_redirect(int buffer_size);

    void start();
    void stop();
    int read(char *buffer, int size);
    int write(const char *buffer, int size);

private:
    int fd_opipe[2];
    int fd_ipipe[2];

    int fd_stdout;
    int fd_stdin;
};

#endif
