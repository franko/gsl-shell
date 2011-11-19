#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

static int pfd[2];

static char output_buf[8192];
static int output_pos = 0;

void
afunction(int n)
{
  int k;
  for (k = 1; k < 100; k++)
    {
      if (k % n == 0)
	{
	  printf ("number: %i\n", k);
	  fflush(stdout);
	}
    }
}

static void
sigio_handler (int sn)
{
  int nr;
  char *buf = output_buf + output_pos;

  while (1)
    {
      errno = 0;
      nr = read (pfd[0], buf, 128);
      if (nr <= 0 || errno != 0)
	break;
      buf += nr;
    }

  output_pos = buf - output_buf;
}

int
main()
{
  int    fd;
  fpos_t pos;
  struct sigaction io_act;
  sigset_t block_mask;

  sigemptyset (&block_mask);
  io_act.sa_handler = sigio_handler;
  io_act.sa_mask = block_mask;
  io_act.sa_flags = SA_RESTART;
  sigaction (SIGIO, &io_act, NULL);

  printf("stdout, ");
  
  fflush(stdout);
  fgetpos(stdout, &pos);
  fd = dup(fileno(stdout));

  pipe (pfd);

  pid_t current_pid = getpid();

  fcntl(pfd[0], F_SETOWN, current_pid);
  fcntl(pfd[0], F_SETFL, O_ASYNC | O_NONBLOCK);

  dup2 (pfd[1], fileno(stdout));
  close (pfd[1]);

  afunction(7);

  fflush(stdout);
  dup2(fd, fileno(stdout));
  close(fd);
  clearerr(stdout);
  fsetpos(stdout, &pos);        /* for C9X */

  close (pfd[0]);
  signal (SIGIO, SIG_DFL);

  printf("stdout again\n");

  output_buf[output_pos] = '\0';

  printf("buffer content:\n");
  puts(output_buf);

  return 0;
 }
