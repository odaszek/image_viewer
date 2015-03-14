#include <gtk/gtk.h>

#include "application.h"

int
main (int argc, char *argv[])
{
  return g_application_run (G_APPLICATION (application_new ()), argc, argv);
}
