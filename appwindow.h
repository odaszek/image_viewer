#ifndef __APPWINDOW_H
#define __APPWINDOW_H

#include <gtk/gtk.h>
#include "application.h"


#define APP_WINDOW_TYPE (app_window_get_type ())
#define APP_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), APP_WINDOW_TYPE, AppWindow))
#define IS_APP_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), APP_WINDOW_TYPE))


typedef struct _AppWindow         AppWindow;
typedef struct _AppWindowClass    AppWindowClass;
typedef struct _AppWindowPrivate  AppWindowPrivate;


GType          app_window_get_type     (void);
AppWindow 	  *app_window_new          (Application *app);
void           app_window_open         (AppWindow *win,
                                        GFile     *file);

#endif /* __APPWINDOW_H */
