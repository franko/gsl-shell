#ifndef FOXGUI_REDIRECT_H
#define FOXGUI_REDIRECT_H

class stdout_redirect
{
public:
  stdout_redirect(int buffer_size);
  ~stdout_redirect();

  int start();
  int stop();
  int read(char *buffer, int size);

private:
  int fd_pipe[2];
  int fd_stdout;
};

#endif
