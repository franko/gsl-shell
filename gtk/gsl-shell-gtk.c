#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "gsl-shell-interp.h"

struct tinfo {
  lua_State *L;
  const char *line;
};

static struct tinfo eval_info;

static int stdout_save_fd;
static int pipe_fd[2];

static volatile gboolean luajit_running = FALSE;

#define IO_TIMEOUT_MS 50

#define BUF_SIZE 4096
char output_buf[BUF_SIZE];

lua_State *global_L;

struct lua_repl {
  GtkTextBuffer *buffer;
  GtkTextMark *input_start, *input_end;
  GtkTextMark *output_end;
  gboolean input_ready;
};

static struct lua_repl repl;

static void *
exec_line (void *_inf)
{
  struct tinfo *inf = _inf;
  fpos_t pos;

  fflush (stdout);
  fgetpos (stdout, &pos);

  dup2 (pipe_fd[1], fileno(stdout));

  /* feed line to Lua interpreter */
  gsl_shell_exec (inf->L, inf->line);

  fflush (stdout);

  dup2 (stdout_save_fd, fileno(stdout));
  clearerr (stdout);
  fsetpos (stdout, &pos);

  luajit_running = FALSE;

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
  if (event->keyval == GDK_KEY_Return && repl.input_ready)
    {
      GtkTextIter s[1], e[1];
      gchar *input_line;

      repl.input_ready = FALSE;

      gtk_text_buffer_get_iter_at_mark (repl.buffer, s, repl.input_start);
      gtk_text_buffer_get_iter_at_mark (repl.buffer, e, repl.input_end);
      input_line = gtk_text_buffer_get_text (repl.buffer, s, e, TRUE);

      luajit_running = TRUE;
      g_timeout_add (IO_TIMEOUT_MS, retrieve_data, NULL);
      start_eval_thread (global_L, input_line);
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

  gtk_init (&argc, &argv);

  global_L = gsl_shell_init ();

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

  gsl_shell_close (global_L);

  return 0;
}
