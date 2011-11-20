#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "gsl-shell-interp.h"

struct tinfo {
  lua_State *L;
  const char *line;
};

static struct tinfo eval_info;

static char output_buf[8192];
static int output_pos;
static int output_eof;

static int pipe_fd[2];

pthread_mutex_t io_mutex = PTHREAD_MUTEX_INITIALIZER;

lua_State *global_L;

static void
sigio_handler (int sn)
{
  int nr;
  char *buf;

  pthread_mutex_lock (&io_mutex);

  buf = output_buf + output_pos;

  while (1)
    {
      errno = 0;
      nr = read (pipe_fd[0], buf, 128);
      if (nr == EOF)
	{
	  output_eof = 1;
	  break;
	}
      if (nr <= 0 || errno != 0)
	break;
      buf += nr;
    }

  output_pos = buf - output_buf;

  pthread_mutex_unlock (&io_mutex);
}

static void *
exec_line (void *_inf)
{
  struct tinfo *inf = _inf;
  int stdout_save_fd;
  fpos_t pos;
  struct sigaction io_act;
  sigset_t block_mask;
  pid_t current_pid;

  output_eof = 0;
  output_pos = 0;

  sigemptyset (&block_mask);
  io_act.sa_handler = sigio_handler;
  io_act.sa_mask = block_mask;
  io_act.sa_flags = SA_RESTART;
  sigaction (SIGIO, &io_act, NULL);
  
  fflush (stdout);
  fgetpos (stdout, &pos);
  stdout_save_fd = dup (fileno(stdout));

  pipe (pipe_fd);

  current_pid = getpid();

  fcntl (pipe_fd[0], F_SETOWN, current_pid);
  fcntl (pipe_fd[0], F_SETFL, O_ASYNC | O_NONBLOCK);

  dup2 (pipe_fd[1], fileno(stdout));
  close (pipe_fd[1]);

  /* feed line to Lua interpreter */
  gsl_shell_exec (inf->L, inf->line);

  fflush (stdout);
  dup2 (stdout_save_fd, fileno(stdout));
  close (stdout_save_fd);
  clearerr (stdout);
  fsetpos (stdout, &pos);

  close (pipe_fd[0]);
  signal (SIGIO, SIG_DFL);

  return NULL;
}

int
start_eval_thread (lua_State *L, const char *line)
{
  pthread_t eval_thread[1];
  pthread_attr_t attr[1];

  eval_info.L = L;
  eval_info.line = line;

  pthread_attr_init (attr);
  pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

  if (pthread_create (eval_thread, attr, exec_line, (void*) &eval_info))
    {
      fprintf(stderr, "error creating thread");
      return 1;
    }

  return 0;
}

static gboolean
on_key_pressed (GtkWidget *w, GdkEventKey *event, void *data)
{
  if (event->keyval >= GDK_KEY_space || event->keyval <= GDK_KEY_asciitilde)
    {
      printf("%c\n", event->keyval);
      start_eval_thread (global_L, "for k=1, 12 do print(k^2) end");
    }
  
  return FALSE;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *view;
  GtkTextBuffer *buffer;
  GtkTextIter p[1];
  GtkTextMark *input_start, *input_end;
  GtkTextMark *output_start, *output_end;

  gtk_init (&argc, &argv);

  global_L = gsl_shell_init ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_default_size (window, 640, 480);

  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  view = gtk_text_view_new ();

  g_signal_connect (view, "key-press-event", G_CALLBACK (on_key_pressed), NULL);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

  gtk_text_buffer_get_start_iter (buffer, p);

  gtk_text_buffer_set_text (buffer, "Hello, this is some text", -1);

  gtk_container_add (GTK_CONTAINER (window), view);

  gtk_widget_show (view);
  gtk_widget_show (window);

  gtk_main ();

  gsl_shell_close (global_L);

  output_buf[output_pos] = '\0';
  printf("output buffer:\n");
  puts(output_buf);

  printf("> buffer end: %i\n", output_eof);

  return 0;
}
