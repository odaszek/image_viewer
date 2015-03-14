#include <gtk/gtk.h>

#include "application.h"
#include "appwindow.h"

struct _Application
{
  GtkApplication parent;
};

struct _ApplicationClass
{
  GtkApplicationClass parent_class;
};

G_DEFINE_TYPE(Application, application, GTK_TYPE_APPLICATION);


static void
application_init (Application *app)
{
}


static void
application_activate (GApplication *app)
{
  AppWindow *win;

  win = app_window_new (APPLICATION(app));
  gtk_window_set_default_size ((GtkWindow*)win, 1000, 800);
  gtk_window_set_title (GTK_WINDOW (win), "Sunny");
  gtk_window_set_icon_name (GTK_WINDOW (win), "sunny");
  gtk_window_present (GTK_WINDOW (win));
}


static void
application_open (GApplication  *app,
                  GFile        **files,
                  gint           n_files,
                  const gchar   *hint)
{
  GList *windows;
  AppWindow *win;
  int i;

  windows = gtk_application_get_windows (GTK_APPLICATION (app));
  if (windows)
    win = APP_WINDOW (windows->data);
  else
  {
    win = app_window_new (APPLICATION(app));
    gtk_window_set_default_size ((GtkWindow*)win, 640, 480);
    gtk_window_set_title (GTK_WINDOW (win), "Sunny");
    gtk_window_set_icon_name (GTK_WINDOW (win), "sunny");
  }

  for (i = 0; i < n_files; i++)
    app_window_open (win, files[i]);

  gtk_window_present (GTK_WINDOW (win));
}

static void
show_about (GSimpleAction *action,
            GVariant      *parameter,
            gpointer       user_data)
{
//  gtk_show_about_dialog (NULL,
//                         "program-name", "Sunny",
//                         "title", "About Sunny",
//                         "logo-icon-name", "sunny",
//                         "comments", "A cheap Bloatpad clone.",
//                         NULL);
}


static void
quit_app (GSimpleAction *action,
          GVariant      *parameter,
          gpointer       user_data)
{
//  GList *list, *next;
//  GtkWindow *win;
//
//  g_print ("Going down...\n");
//
//  list = gtk_application_get_windows (GTK_APPLICATION (g_application_get_default ()));
//  while (list)
//    {
//      win = list->data;
//      next = list->next;
//
//      gtk_widget_destroy (GTK_WIDGET (win));
//
//      list = next;
//    }
}

static void
new_activated (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       user_data)
{
  //GApplication *app = user_data;
  //g_application_activate (app);
}

static GActionEntry app_entries[] = {
  { "about", show_about, NULL, NULL, NULL },
  { "quit", quit_app, NULL, NULL, NULL },
  { "new", new_activated, NULL, NULL, NULL }
};

static void
application_startup (GApplication *app)
{
	GtkBuilder *builder;

	//G_OBJECT_CLASS (application_parent_class)->startup (app);
	G_APPLICATION_CLASS (application_parent_class)->startup (app);

	//what to do?
	g_action_map_add_action_entries (G_ACTION_MAP (app), app_entries, G_N_ELEMENTS (app_entries), app);

	  if (g_getenv ("APP_MENU_FALLBACK"))
	    g_object_set (gtk_settings_get_default (), "gtk-shell-shows-app-menu", FALSE, NULL);

	  builder = gtk_builder_new ();
	  gtk_builder_add_from_string (builder,
	                               "<interface>"
	                               "  <menu id='app-menu'>"
	                               "    <section>"
	                               "      <item>"
	                               "        <attribute name='label' translatable='yes'>_New Window</attribute>"
	                               "        <attribute name='action'>app.new</attribute>"
	                               "      </item>"
	                               "      <item>"
	                               "        <attribute name='label' translatable='yes'>_About Sunny</attribute>"
	                               "        <attribute name='action'>app.about</attribute>"
	                               "      </item>"
	                               "      <item>"
	                               "        <attribute name='label' translatable='yes'>_Quit</attribute>"
	                               "        <attribute name='action'>app.quit</attribute>"
	                               "        <attribute name='accel'>&lt;Primary&gt;q</attribute>"
	                               "      </item>"
	                               "    </section>"
			  	  	  	  	  	   "    <section>"
								   "      <item>"
								   "        <attribute name='label' translatable='yes'>_New Window</attribute>"
								   "        <attribute name='action'>app.new</attribute>"
								   "      </item>"
								   "      <item>"
								   "        <attribute name='label' translatable='yes'>_About Sunny</attribute>"
								   "        <attribute name='action'>app.about</attribute>"
								   "      </item>"
								   "      <item>"
								   "        <attribute name='label' translatable='yes'>_Quit</attribute>"
								   "        <attribute name='action'>app.quit</attribute>"
								   "        <attribute name='accel'>&lt;Primary&gt;q</attribute>"
								   "      </item>"
								   "    </section>"
	                               "  </menu>"
	                               "</interface>", -1, NULL);
	  gtk_application_set_app_menu (GTK_APPLICATION (app), G_MENU_MODEL (gtk_builder_get_object (builder, "app-menu")));
	  g_object_unref (builder);
}

static void
application_class_init (ApplicationClass *class)
{
  G_APPLICATION_CLASS (class)->startup = application_startup;
  G_APPLICATION_CLASS (class)->activate = application_activate;
  G_APPLICATION_CLASS (class)->open = application_open;
}


Application *
application_new (void)
{
  return g_object_new (APPLICATION_TYPE,
                       "application-id",
					   "org.gtk.exampleapp",
                       "flags",
					   G_APPLICATION_HANDLES_OPEN,
                       NULL);
}
