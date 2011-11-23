#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "gsl-shell-interp.h"

static pthread_mutex_t eval_mutex;
static pthread_cond_t eval_ready;

static int stdout_save_fd;
static int pipe_fd[2];

static volatile gboolean luajit_running = FALSE;

#define IO_TIMEOUT_MS 50

#define BUF_SIZE 4096
char output_buf[BUF_SIZE];

struct lua_repl {
  GtkTextBuffer *buffer;
  GtkTextMark *input_start, *input_end;
  GtkTextMark *output_end;
  gboolean input_ready;
};

static struct lua_repl repl;

enum luajit_cmd {
  LUAJIT_EVAL,
  LUAJIT_EXIT,
};

struct luajit_request {
  enum luajit_cmd cmd;
  const char *line;
};

static volatile struct luajit_request g_luajit_request;

static void
luajit_exec_line (lua_State *L, const char *line)
{
  fpos_t pos;

  luajit_running = TRUE;

  fflush (stdout);
  fgetpos (stdout, &pos);

  dup2 (pipe_fd[1], fileno(stdout));

  /* feed line to Lua interpreter */
  gsl_shell_exec (L, line);

  fflush (stdout);

  dup2 (stdout_save_fd, fileno(stdout));
  clearerr (stdout);
  fsetpos (stdout, &pos);

  luajit_running = FALSE;
}

static void *
luajit_eval_thread (void *userdata)
{
  lua_State *L = gsl_shell_init ();

  while (L)
    {
      pthread_mutex_lock (&eval_mutex);
      pthread_cond_wait (&eval_ready, &eval_mutex);

      enum luajit_cmd cmd = g_luajit_request.cmd;
      const char *line = g_luajit_request.line;

      pthread_mutex_unlock (&eval_mutex);

      switch (cmd)
	{
	case LUAJIT_EVAL:
	  luajit_exec_line (L, line);
	  break;
	case LUAJIT_EXIT:
	  gsl_shell_close (L);
	  L = NULL;
	  break;
	}
    }

  pthread_exit (NULL);
}

static int
start_interp_thread ()
{
  pthread_t eval_thread[1];
  pthread_attr_t attr[1];

  pthread_attr_init (attr);
  pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

  if (pthread_create (eval_thread, attr, luajit_eval_thread, NULL))
    {
      fprintf(stderr, "error creating thread");
      return 1;
    }

  return 0;
}

static void
prepare_input ()
{
  GtkTextIter curr[1];
  gtk_text_buffer_get_iter_at_mark (repl.buffer, curr, repl.output_end);
  gtk_text_buffer_insert (repl.buffer, curr, "> ", 2);
  gtk_text_buffer_move_mark (repl.buffer, repl.input_start, curr);
  gtk_text_buffer_move_mark (repl.buffer, repl.input_end, curr);
  repl.input_ready = TRUE;
}

static gboolean
retrieve_data(gpointer data)
{
  int nr;
  
  do
    {
      nr = read (pipe_fd[0], output_buf, BUF_SIZE);
      
      if (nr > 0)
	{
	  GtkTextIter e[1];
	  gtk_text_buffer_get_iter_at_mark (repl.buffer, e, repl.output_end);
	  gtk_text_buffer_insert (repl.buffer, e, output_buf, nr);
	}
    }
  while (nr > 0);

  if (!luajit_running)
    {
      int n_rem;
      ioctl (pipe_fd[0], FIONREAD, &n_rem);
      if (n_rem == 0)
	{
	  prepare_input ();
	  return FALSE;
	}
    }

  return TRUE;
}

static gboolean
on_key_pressed (GtkWidget *w, GdkEventKey *event, void *data)
{
  if (event->keyval == GDK_KEY_Return && repl.input_ready && !luajit_running)
    {
      GtkTextIter s[1], e[1];
      gchar *input_line;

      repl.input_ready = FALSE;

      gtk_text_buffer_get_iter_at_mark (repl.buffer, s, repl.input_start);
      gtk_text_buffer_get_iter_at_mark (repl.buffer, e, repl.input_end);
      input_line = gtk_text_buffer_get_text (repl.buffer, s, e, TRUE);

      pthread_mutex_lock (&eval_mutex);
      g_luajit_request.cmd = LUAJIT_EVAL;
      g_luajit_request.line = input_line;
      pthread_cond_signal (&eval_ready);
      pthread_mutex_unlock (&eval_mutex);

      g_timeout_add (IO_TIMEOUT_MS, retrieve_data, NULL);
    }
  
  return FALSE;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *view;
  GtkWidget *swin;
  GtkTextBuffer *buffer;
  GtkTextIter p[1];

  stdout_save_fd = dup (fileno(stdout));

  pipe (pipe_fd);

  fcntl (pipe_fd[0], F_SETOWN, getpid());
  fcntl (pipe_fd[0], F_SETFL, O_NONBLOCK);

  if (start_interp_thread ())
    return 1;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window), swin);
  view = gtk_text_view_new ();
  gtk_container_add (GTK_CONTAINER (swin), view);
  gtk_scrolled_window_set_policy (swin, GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

  g_signal_connect (view, "key-press-event", G_CALLBACK (on_key_pressed), NULL);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

  gtk_text_buffer_set_text (buffer, "Hello, this is some text\n", -1);

  gtk_text_buffer_get_end_iter (buffer, p);
  repl.output_end  = gtk_text_buffer_create_mark (buffer, NULL, p, FALSE);
  repl.input_start = gtk_text_buffer_create_mark (buffer, NULL, p, TRUE);
  repl.input_end   = gtk_text_buffer_create_mark (buffer, NULL, p, FALSE);
  repl.buffer = buffer;
  prepare_input ();

  gtk_widget_show (swin);
  gtk_widget_show (view);
  gtk_widget_show (window);

  gtk_main ();

  pthread_mutex_lock (&eval_mutex);
  g_luajit_request.cmd = LUAJIT_EXIT;
  pthread_cond_signal (&eval_ready);
  pthread_mutex_unlock (&eval_mutex);

  return 0;
}
