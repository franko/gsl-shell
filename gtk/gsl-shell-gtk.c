#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

static gboolean
on_key_pressed (GtkWidget *w, GdkEventKey *event, void *data)
{
  if (event->keyval >= GDK_KEY_space || event->keyval <= GDK_KEY_asciitilde)
    {
      printf("%c\n", event->keyval);
    }
  
  return FALSE;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *view;
  GtkTextBuffer *buffer;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_default_size (window, 640, 480);

  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  view = gtk_text_view_new ();

  g_signal_connect (view, "key-press-event", G_CALLBACK (on_key_pressed), NULL);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

  gtk_text_buffer_set_text (buffer, "Hello, this is some text", -1);

  gtk_container_add (GTK_CONTAINER (window), view);

  gtk_widget_show (view);
  gtk_widget_show (window);

  gtk_main ();

  return 0;
}
