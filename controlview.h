#ifndef __CONTROL_VIEW_H
#define __CONTROL_VIEW_H

#include <gtk/gtk.h>

#define XONTROL_VIEW_TYPE (xontrol_view_get_type ())
#define XONTROL_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), XONTROL_VIEW_TYPE, XontrolView))
#define IS_XONTROL_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XONTROL_VIEW_TYPE))
//#define CONTROL_VIEW_CLASS(klass)(G_TYPE_CHECK_CLASS_CAST ((klass), CONTROL_VIEW_TYPE, XontrolViewClass))
//#define IS_CONTROL_VIEW_CLASS(klass)(G_TYPE_CHECK_CLASS_TYPE ((klass), CONTROL_VIEW_TYPE))
//#define CONTROL_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), CONTROL_VIEW_TYPE, XontrolViewClass))

typedef struct _XontrolView         XontrolView;
typedef struct _XontrolViewClass    XontrolViewClass;

GType           	   xontrol_view_get_type     (void);
XontrolView 		  *xontrol_view_new          (void);

#endif /* __CONTROL_VIEW_H */
