/*
 * controlview.c
 *
 *  Created on: Mar 2, 2015
 *      Author: odach
 */

#include "controlview.h"

static void xontrol_view_class_init	 (XontrolViewClass 	  *class);
static void xontrol_view_init		 (XontrolView 		  *view);

struct _XontrolView
{
	GtkDrawingArea widget;
	cairo_surface_t *surface;
};

struct _XontrolViewClass
{
	GtkDrawingAreaClass parent_class;
};

G_DEFINE_TYPE(XontrolView, xontrol_view, GTK_TYPE_DRAWING_AREA);


static gboolean
xontrol_view_configure_event (GtkWidget 		*widget,
							  GdkEventConfigure *event)
{
  g_print("control_view_configure_event\n");

  XontrolView *view;
  guint        width,
               height;
  GdkRGBA      color;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (IS_XONTROL_VIEW (widget), FALSE);

  view = XONTROL_VIEW(widget);

  if (view->surface)
    cairo_surface_destroy (view->surface);

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);

  view->surface = gdk_window_create_similar_surface (
		  	  	  	  	  gtk_widget_get_window (widget),
		  	  	  	  	  CAIRO_CONTENT_COLOR,
						  gtk_widget_get_allocated_width (widget),
						  gtk_widget_get_allocated_height (widget));
  cairo_t *cr;
  cr = cairo_create (view->surface);

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);

//  cairo_arc (cr,
//             width / 2.0, height / 2.0,
//             MIN (width, height) / 2.0,
//             0, 2 * G_PI);

  gtk_style_context_get_color (gtk_widget_get_style_context (widget), 0, &color);
  gdk_cairo_set_source_rgba (cr, &color);

//  cairo_fill (cr);
  cairo_paint (cr);

  cairo_destroy (cr);

  return TRUE;
}


static gboolean
xontrol_view_draw (GtkWidget *widget,
                   cairo_t   *cr)
{
  g_print("control_view_draw\n");

  XontrolView *view;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (IS_XONTROL_VIEW (widget), FALSE);

  view = XONTROL_VIEW(widget);

  cairo_set_source_surface (cr, view->surface, 0, 0);
  cairo_paint (cr);

  return FALSE;
}


static void
xontrol_view_class_init (XontrolViewClass *class)
{
  g_print("ControlView class Init\n");

  GTK_WIDGET_CLASS(class)->draw = xontrol_view_draw;
  GTK_WIDGET_CLASS(class)->configure_event = xontrol_view_configure_event;

//  GtkWidgetClass *widget_class;
//  widget_class = (GtkWidgetClass*) class;

//  GtkObjectClass *object_class;
//  object_class = (GtkObjectClass*) class;

}

static void
xontrol_view_init (XontrolView *view)
{
  g_print("ControlView Init\n");

  view->surface = NULL;
  gtk_widget_set_size_request((GtkWidget*)(view), -1,75);
}

XontrolView *
xontrol_view_new ()
{
  g_print("ControlView New\n");
  return g_object_new (XONTROL_VIEW_TYPE, NULL);
}






