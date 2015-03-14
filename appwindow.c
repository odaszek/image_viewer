#include <gtk/gtk.h>

#include "application.h"
#include "appwindow.h"
#include "controlview.h"
#include "imageview.h"

struct _AppWindow
{
	GtkApplicationWindow parent;

	GtkWidget *vbox;
	GtkWidget *menubar;
	GtkWidget *filemenu;
	GtkWidget *file;
	GtkWidget *quit;

	ImageView *image_view;
	XontrolView *control_view;
};

struct _AppWindowClass
{
	GtkApplicationWindowClass parent_class;
};

G_DEFINE_TYPE(AppWindow, app_window, GTK_TYPE_APPLICATION_WINDOW);

void
app_window_open_dialog(GtkWidget *widget)
{
	GtkWidget *dialog;
	AppWindow *win;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_APP_WINDOW (widget));

	win = APP_WINDOW(widget);

	dialog = gtk_file_chooser_dialog_new (	"Open File",
											(GtkWindow*)widget,
											GTK_FILE_CHOOSER_ACTION_OPEN,
											GTK_STOCK_CANCEL,
											GTK_RESPONSE_CANCEL,
											GTK_STOCK_OPEN,
											GTK_RESPONSE_ACCEPT,
											NULL);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;

		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
		filename = gtk_file_chooser_get_filename (chooser);
		app_window_open(win, g_file_new_for_path (filename));

		printf("FILE OPENED: %s\n", filename);
		g_free (filename);
	}

	gtk_widget_destroy (dialog);
}


static gboolean
app_window_on_key(GtkWidget *widget, GdkEventKey *event)
{
	AppWindow *win;

	g_return_val_if_fail (widget != NULL, TRUE);
	g_return_val_if_fail (IS_APP_WINDOW (widget), TRUE);

	win = APP_WINDOW(widget);

	switch (event->keyval)
	{
		case GDK_KEY_f:
			printf("key pressed: %s ; FIT IMAGE\n", "f");
			image_view_set_fit(win->image_view, TRUE);
			break;

		case GDK_KEY_a:
			printf("key pressed: %s ; ALL IMAGE\n", "a");
			image_view_set_fit(win->image_view, FALSE);
			break;

		case GDK_KEY_l:
			printf("key pressed: %s ; ROTATE LEFT\n", "l");
			image_view_rotate_left(win->image_view);
			break;

		case GDK_KEY_r:
			printf("key pressed: %s ; ROTATE RIGHT\n", "r");
			image_view_rotate_right(win->image_view);
			break;

		case GDK_KEY_plus:
			printf("key pressed: %s ; ZOOM IN\n", "+");
			image_view_zoom_in(win->image_view);
			break;

		case GDK_KEY_minus:
			printf("key pressed: %s ; ZOOM OUT\n", "-");
			image_view_zoom_out(win->image_view);
			break;

		case GDK_KEY_o:
			if (event->state & GDK_CONTROL_MASK)
			{
				printf("key pressed: %s ; OPEN IMAGE\n", "ctrl + o");
				app_window_open_dialog(widget);
			}
			else
			{
				printf("key pressed: %s\n", "s");
			}
			break;

		case GDK_KEY_m:
			if (event->state & GDK_SHIFT_MASK)
			{
				printf("key pressed: %s\n", "shift + m");
			}
			else if (event->state & GDK_CONTROL_MASK)
			{
				printf("key pressed: %s\n", "ctrl + m");
			}
			else
			{
				printf("key pressed: %s\n", "m");
			}
			break;

		default:
			return FALSE;
	}

	return FALSE;
}

static void
app_window_init (AppWindow *win)
{
	  g_print("Window Init\n");

	  g_return_if_fail (win != NULL);
	  g_return_if_fail (IS_APP_WINDOW (win));

	  win->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	  gtk_container_add(GTK_CONTAINER(win), win->vbox);

	  win->menubar = gtk_menu_bar_new();
	  win->filemenu = gtk_menu_new();

	  win->file = gtk_menu_item_new_with_label("File");
	  win->quit = gtk_menu_item_new_with_label("Quit");

	  gtk_menu_item_set_submenu(GTK_MENU_ITEM(win->file), win->filemenu);
	  gtk_menu_shell_append(GTK_MENU_SHELL(win->filemenu), win->quit);
	  gtk_menu_shell_append(GTK_MENU_SHELL(win->menubar), win->file);
	  gtk_box_pack_start(GTK_BOX(win->vbox), win->menubar, FALSE, FALSE, 3);

	  win->image_view = image_view_new();
	  gtk_box_pack_start(GTK_BOX(win->vbox), GTK_WIDGET(win->image_view), TRUE, TRUE, 0);

	  win->control_view = xontrol_view_new();
	  gtk_box_pack_start(GTK_BOX(win->vbox), GTK_WIDGET(win->control_view), FALSE, FALSE, 0);

	  gtk_widget_show_all(win->vbox);
}

static void
app_window_dispose (GObject *object)
{
 	G_OBJECT_CLASS (app_window_parent_class)->dispose (object);
}

static void
app_window_class_init (AppWindowClass *class)
{
	//g_signal_connect (G_OBJECT (win), "key_press_event", G_CALLBACK (on_key_press), NULL);
	//gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
	GTK_WIDGET_CLASS (class)->key_press_event = app_window_on_key;
}

AppWindow *
app_window_new (Application *app)
{
	return g_object_new (APP_WINDOW_TYPE, "application", app, NULL);
}

void
app_window_open (AppWindow *win, GFile *file)
{
	image_view_open(win->image_view, file);
}
