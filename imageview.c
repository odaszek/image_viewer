/*
 * imageview.c
 *
 *  Created on: Mar 2, 2015
 *      Author: odach
 */

#include "application.h"
#include "imageview.h"

static void image_view_class_init	 (ImageViewClass 	  *class);
static void image_view_init			 (ImageView 		  *view);

#define IMG_VIEW_ZOOM_STEP 	 0.02
#define IMG_VIEW_ZOOM_MIN 	 1.00
#define IMG_VIEW_ZOOM_MAX 	30.00

struct _ImageView
{
	GtkDrawingArea   widget;
	GFile			*file;
	cairo_surface_t *surface;
	cairo_surface_t *img_surface;
	GdkPixbuf       *image;
	gint 			 img_width;
	gint 			 img_height;

	gboolean		 fit;
	guint			 rotation;
	gfloat			 scale;

	gint 			 view_zoom_x;
	gint 			 view_zoom_y;
	gint 			 img_zoom_x;
	gint 			 img_zoom_y;

	gint			 view_mouse_x;
	gint			 view_mouse_y;
	gint			 img_mouse_x;
	gint			 img_mouse_y;

	struct _drag_data
	{
		gboolean dragging;
		gint drag_x;
		gint drag_y;

	} drag_data;
};

struct _ImageViewClass
{
	GtkDrawingAreaClass parent_class;
};

G_DEFINE_TYPE(ImageView, image_view, GTK_TYPE_DRAWING_AREA);


gboolean
point_in_rect(gint x, gint y, GdkRectangle *rect)
{
//	if((rect->x < x < rect->width) && (rect->y < y < rect->height))
//		return TRUE;
//	else return FALSE;
	if(	(rect->x < x) &&
		( x < rect->width) &&
		(rect->y < y ) &&
		( y < rect->height))
		return TRUE;
	else return FALSE;
}

gboolean
image_view_create_image_surface(GtkWidget *widget)
{
	g_print("ENTER: image_view_create_image_surface\n");

	ImageView 			*view;

	char   				*path;
	GError 				*error = NULL;
	GdkPixbuf 			*tmp_image;
	cairo_surface_t 	*tmp_img_surface;
	cairo_t 			*tmp_cr;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (IS_IMAGEVIEW (widget), FALSE);

	view = IMAGEVIEW(widget);

	if(view->file == NULL)
	{
		g_print("ERROR: No Image File Specified!");
		return FALSE;
	}

	if(view->surface == NULL)
	{
		g_print("ERROR: No window surface exists!");
		return FALSE;
	}

	path = g_file_get_path(view->file);

	g_print("INFO: Image Path: %s\n", path);
	g_print("INFO: Image Name: %s\n", basename(path));

	tmp_image = gdk_pixbuf_new_from_file(path, &error);

	if(!GDK_IS_PIXBUF(tmp_image))
	{
		g_printerr("ERROR: Unable to create pixbuffer, %s !\n",
										error->message);
		g_error_free (error);
		g_free(path);
		return FALSE;
	}

	g_print("INFO: PixBuf created successfully.\n");

    int img_width = gdk_pixbuf_get_width (tmp_image);
    int img_height = gdk_pixbuf_get_height (tmp_image);

    tmp_img_surface = cairo_surface_create_similar_image(
    		view->surface,
    		CAIRO_FORMAT_ARGB32,
			gdk_pixbuf_get_width (tmp_image),
			gdk_pixbuf_get_height (tmp_image));

    if (tmp_img_surface == NULL)
    {
    	g_print("ERROR: Unable to create img_surface.\n");
    	g_object_unref(tmp_image);
		g_free(path);
		return FALSE;
    }

    g_print("INFO: Created image surface.\n");

	tmp_cr = cairo_create (tmp_img_surface);
	if(tmp_cr == NULL)
	{
		g_print("ERROR: Unable to create Cairo context.\n");
		g_object_unref(tmp_image);
		g_free(path);
		return FALSE;
	}

	g_print("INFO: Cairo context created.\n");

	gdk_cairo_set_source_pixbuf (tmp_cr,tmp_image,0,0);
	cairo_paint (tmp_cr);
	cairo_destroy (tmp_cr);

	// all good

	if(view->image)
		g_object_unref(view->image);

	view->image = tmp_image;

	if(view->img_surface)
	{
		g_print("INFO: Destroy old img_surface.\n");
		cairo_surface_destroy(view->img_surface);
	}

	view->img_surface = tmp_img_surface;

	view->img_width = img_width;
	view->img_height = img_height;

	view->img_zoom_x = view->img_width/2;
	view->img_zoom_y = view->img_height/2;

//    image_view_draw_surface (GTK_WIDGET(view));
//    gtk_widget_queue_draw (GTK_WIDGET(view));

	g_free(path);

	return TRUE;
}

gboolean
image_view_create_surface(GtkWidget *widget)
{
	g_print("ENTER: image_view_create_surface\n");

	ImageView *view;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (IS_IMAGEVIEW (widget), FALSE);

	view = IMAGEVIEW(widget);

	int width = gtk_widget_get_allocated_width (widget);
	int height = gtk_widget_get_allocated_height (widget);

	if(width < 1 || height < 1)
		return FALSE;

	if (view->surface)
		cairo_surface_destroy (view->surface);

	view->surface = gdk_window_create_similar_surface (
						gtk_widget_get_window (widget),
						CAIRO_CONTENT_COLOR,
						width,
						height);

	if(view->surface == NULL)
		return FALSE;

	if(view->file != NULL && view->img_surface == NULL)
			image_view_create_image_surface(widget);

	view->view_zoom_x = width/2;
	view->view_zoom_y = height/2;

	image_view_draw_surface(widget);
	return TRUE;
}


gboolean
image_view_draw_surface(GtkWidget *widget)
{
	//g_print("ENTER: image_view_draw_surface\n");

	ImageView   *view;
	GdkRGBA      color;

	float 	width;
	float 	height;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (IS_IMAGEVIEW (widget), FALSE);

	view = IMAGEVIEW(widget);

	if(view->surface == NULL)
	{
		g_print("INFO: surface not found!\n");
		return FALSE;
	}

	width = gtk_widget_get_allocated_width (widget);
	height = gtk_widget_get_allocated_height (widget);

    if(width < 1 || height < 1){
      return TRUE;
    }


	cairo_t *cr;
	cr = cairo_create (view->surface);
	cairo_save(cr);

	gtk_style_context_get_color (
			gtk_widget_get_style_context (GTK_WIDGET(view)),0, &color);
	gdk_cairo_set_source_rgba (cr, &color);
	cairo_paint (cr);

	if(view->img_surface == NULL)
	{
		g_print("INFO: No image surface exists.\n");
		cairo_destroy (cr);
		return TRUE;
	}

    // translate --------------------------------------------------------------------------------------
    //cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	//cairo_set_antialias(cr, CAIRO_ANTIALIAS_SUBPIXEL);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
    cairo_pattern_set_filter(cairo_get_source (cr),CAIRO_FILTER_NEAREST);
    cairo_translate(cr, view->view_zoom_x, view->view_zoom_y);
    cairo_rotate (cr, (view->rotation * 90) * 3.14159265358979323846/180);

    if(view->fit)
    {
		view->scale = MIN(1.00,
			MIN( 	(float)width/view->img_width,
					(float)height/view->img_height));

    }
    cairo_scale (cr,view->scale,view->scale);

	GdkRectangle 		img_rect;
	image_view_get_scaled_rect(widget, &img_rect);

	if(point_in_rect(view->view_mouse_x, view->view_mouse_y, &img_rect))
	{
		view->img_mouse_x = (view->view_mouse_x - img_rect.x) / view->scale;
		view->img_mouse_y = (view->view_mouse_y - img_rect.y) / view->scale;
	}
	else
	{
		view->img_mouse_x = 0;
		view->img_mouse_y = 0;
	}

    cairo_set_source_surface(
    		 cr,
    		 view->img_surface,
			-view->img_zoom_x,
			-view->img_zoom_y);


	cairo_paint (cr);

	cairo_restore(cr);



	// paint view_zoom
	//cairo_translate(cr, view->view_zoom_x, view->view_zoom_y);
	double xc = view->view_zoom_x;
	double yc = view->view_zoom_y;
	double radius = 20.0;
	double angle1 = 0.0  * (3.14159265358979323846/180.0);  /* angles are specified */
	double angle2 = 360.0 * (3.14159265358979323846/180.0);  /* in radians           */

	cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
	cairo_set_line_width (cr, 10.0);
	cairo_arc (cr, xc, yc, radius, angle1, angle2);
	cairo_stroke (cr);


	// Paint image_zoom



// show text ----------------------------------------------------------

	  double info_rc_x = 20.0;
	  double info_rc_y = 20.0;
	  double info_rc_width = 300.0;
	  double info_rc_height = height-40.0;
	  double info_rc_radius = 5.0;
	  double info_rc_degrees = 3.14159265358979323846 / 180.0;

	  cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.6);
	  cairo_rectangle (cr, 10, 10, 200, height-20.0);
	  cairo_fill (cr);

	  cairo_select_font_face (cr, "Sans",
		  CAIRO_FONT_SLANT_NORMAL,
		  CAIRO_FONT_WEIGHT_NORMAL);
	  cairo_set_font_size (cr, 10.0);
	  cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.6);

	  cairo_text_extents_t extents;
	  char *info_text;
	  info_text = malloc(300 * sizeof(char));
	  double x,y;

	  double top = 30;
	  sprintf(info_text,"DEBUG INFO:");
	  cairo_text_extents (cr, info_text, &extents);
	  cairo_move_to (cr, 15,top);
	  cairo_show_text (cr, info_text);

	  // file name
	  top += extents.height + 5;
	  sprintf(info_text,"File Name: %s",basename(g_file_get_path(view->file)));
	  cairo_text_extents (cr, info_text, &extents);
	  cairo_move_to (cr, 15,top);
	  cairo_show_text (cr, info_text);

	  // Image Size
	  top += extents.height + 3;
	  sprintf(info_text,"Image Size: %i x %i",
			  (int)view->img_width, (int)view->img_height);
	  cairo_text_extents (cr, info_text, &extents);
	  cairo_move_to (cr, 15,top);
	  cairo_show_text (cr, info_text);

	  // Window Size
	  top += extents.height + 3;
	  sprintf(info_text,"Window Size: %i x %i",(int)width, (int)height);
	  cairo_text_extents (cr, info_text, &extents);
	  cairo_move_to (cr, 15,top);
	  cairo_show_text (cr, info_text);

	  // Zoom
	  top += extents.height + 3;
	  sprintf(info_text,"Zoom: %u",(int)(view->scale *100.0));
	  cairo_text_extents (cr, info_text, &extents);
	  cairo_move_to (cr, 15,top);
	  cairo_show_text (cr, info_text);

	  // Image Rectangle
	  top += extents.height + 3;
	  sprintf(info_text,"Image Rect: %i, %i, %i, %i",
			  (int)img_rect.x,(int)img_rect.y,
			  (int)img_rect.width,(int)img_rect.height);
	  cairo_text_extents (cr, info_text, &extents);
	  cairo_move_to (cr, 15,top);
	  cairo_show_text (cr, info_text);

	  // Window Mouse
	  top += extents.height + 3;
	  sprintf(info_text,"Window Mouse: %i, %i",
			  (int)view->view_mouse_x,(int)view->view_mouse_y);
	  cairo_text_extents (cr, info_text, &extents);
	  cairo_move_to (cr, 15,top);
	  cairo_show_text (cr, info_text);

	  // Image Mouse
	  top += extents.height + 3;
	  sprintf(info_text,"Image Mouse: %i, %i",
			  (int)view->img_mouse_x,(int)view->img_mouse_y);
	  cairo_text_extents (cr, info_text, &extents);
	  cairo_move_to (cr, 15,top);
	  cairo_show_text (cr, info_text);

	  // Window Zoom:
	  top += extents.height + 3;
	  sprintf(info_text,"Window Zoom: %i, %i",
			  (int)view->view_zoom_x,(int)view->view_zoom_y);
	  cairo_text_extents (cr, info_text, &extents);
	  cairo_move_to (cr, 15,top);
	  cairo_show_text (cr, info_text);

	  // Image Zoom:
	  top += extents.height + 3;
	  sprintf(info_text,"Image Zoom: %i, %i",
			  (int)view->img_zoom_x,(int)view->img_zoom_y);
	  cairo_text_extents (cr, info_text, &extents);
	  cairo_move_to (cr, 15,top);
	  cairo_show_text (cr, info_text);

	  free(info_text);

//show text ----------------------------------------------------------

	  cairo_destroy (cr);

//	  g_print("INFO: Zoom: %u.\n", (guint)(view->scale * 100));

	  return TRUE;
}


static gboolean
image_view_configure_event (GtkWidget *widget,
							GdkEventConfigure  *event)
{
  g_print("ENTER: image_view_configure_event\n");

  return image_view_create_surface(widget);
}



static gboolean
image_view_draw (GtkWidget *widget, cairo_t *cr)
{
  //g_print("ENTER: image_view_draw\n");

  ImageView *view;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (IS_IMAGEVIEW (widget), FALSE);

  view = IMAGEVIEW(widget);

  if(view->surface == NULL)
  {
	  return FALSE;
  }

  cairo_set_source_surface (cr, view->surface, 0, 0);
  cairo_paint (cr);

  return FALSE;
}


static void
image_view_dispose (GObject *object)
{
	g_print("ENTER: image_view_dispose\n");

	ImageView *view;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_IMAGEVIEW (object));

	view = IMAGEVIEW(object);

	if(view->image)
	{
		g_object_unref(view->image);
		view->image = NULL;
	}

	if(view->surface)
	{
		cairo_surface_destroy(view->surface);
		view->surface = NULL;
	}

	if(view->img_surface)
	{
		cairo_surface_destroy(view->img_surface);
		view->img_surface = NULL;
	}

	G_OBJECT_CLASS (image_view_parent_class)->dispose (object);
}


static gboolean
image_view_move_image( GtkWidget *widget, int offset_x, int offset_y)
{

	return TRUE;
}

void
image_view_get_scaled_rect(GtkWidget *widget, GdkRectangle *img_rect)
{
	ImageView 			*view;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_IMAGEVIEW (widget));

	view = IMAGEVIEW(widget);

	img_rect->x = (view->view_zoom_x - (view->img_zoom_x * view->scale))- view->scale/2;
	img_rect->y = (view->view_zoom_y - (view->img_zoom_y * view->scale))- view->scale/2;
	img_rect->width = img_rect->x + (view->img_width * view->scale);
	img_rect->height = img_rect->y + (view->img_height * view->scale);
}


static gboolean
image_view_button_press( GtkWidget *widget, GdkEventButton *event )
{
	g_print("ENTER: image_view_button_press\n");

	ImageView 			*view;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (IS_IMAGEVIEW (widget), FALSE);

	view = IMAGEVIEW(widget);

	int width = gtk_widget_get_allocated_width (widget);
	int height = gtk_widget_get_allocated_height (widget);
//	int img_left = (view->view_zoom_x - (view->img_zoom_x * view->scale)) - view->scale/2;
//	int img_top = (view->view_zoom_y - (view->img_zoom_y * view->scale)) - view->scale/2;
//	int img_right = img_left + (view->img_width * view->scale);
//	int img_bottom = img_top + (view->img_height * view->scale);


	view->drag_data.dragging = TRUE;
	view->drag_data.drag_x = event->x;
	view->drag_data.drag_y = event->y;

	GdkRectangle 		img_rect;
	image_view_get_scaled_rect(widget, &img_rect);

	if(point_in_rect(event->x, event->y, &img_rect))
	{
		view->view_zoom_x = event->x;
		view->view_zoom_y = event->y;
		view->img_zoom_x = (event->x - img_rect.x)/view->scale;
		view->img_zoom_y = (event->y - img_rect.y)/view->scale;

		image_view_draw_surface (GTK_WIDGET(view));
		gtk_widget_queue_draw (GTK_WIDGET(view));
	}

//	if((view->img_width * view->scale) > width
//			&& event->y > img_rect.y
//			&& event->y < img_rect.height)
//	{
//		view->view_zoom_x = event->x;
//		view->img_zoom_x = (event->x - img_rect.x)/view->scale;
//	}


//	view->img_zoom_x = (event->x - img_left)/view->scale;
//	view->img_zoom_y = (event->y - img_top)/view->scale;

	return TRUE;
}

static gboolean
image_view_mouse_move( GtkWidget *widget, GdkEventMotion *event )
{
	//g_print("ENTER: image_view_mouse_move\n");

	ImageView 			*view;

	int offset_x;
	int offset_y;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (IS_IMAGEVIEW (widget), FALSE);

	view = IMAGEVIEW(widget);

	view->view_mouse_x = event->x;
	view->view_mouse_y = event->y;

	int width = gtk_widget_get_allocated_width (widget);
	int height = gtk_widget_get_allocated_height (widget);
//	int img_left = (view->view_zoom_x - (view->img_zoom_x * view->scale)) - view->scale/2;
//	int img_top = (view->view_zoom_y - (view->img_zoom_y * view->scale)) - view->scale/2;
//	int img_right = img_left + (view->img_width * view->scale);
//	int img_bottom = img_top + (view->img_height * view->scale);

	GdkRectangle 		img_rect;
	image_view_get_scaled_rect(widget, &img_rect);

	if(view->drag_data.dragging)
	{
//		offset_x = event->x - view->drag_data.x;
//		view->drag_data.x = event->x;
//
//		offset_y = event->y - view->drag_data.y;
//		view->drag_data.y = event->y;

//		//image_view_move_image(widget,offset_x, offset_y);

//		if( offset_x < 0)
//		{
//			offset_x = -MIN(abs(offset_x), img_right - width);
//		}
//		else if( offset_x > 0)
//		{
//			offset_x = MIN(offset_x, abs(img_left));
//		}

//		if( offset_y < 0)
//		{
//			offset_y = -MIN(abs(offset_y), img_bottom - height);
//		}
//		else if( offset_y > 0)
//		{
//			offset_y = MIN(offset_y, abs(img_top));
//		}

		int offset_x;
		int offset_y;

		offset_x = event->x - view->drag_data.drag_x;
		offset_y = event->y - view->drag_data.drag_y;

		g_print("Dragging: %i x %i\n", offset_x, offset_y);

		if(offset_x > 0)
		{
			offset_x = MIN(offset_x, MAX(0, -img_rect.x));
			view->view_zoom_x += offset_x;
		}
		else if(offset_x < 0)
		{
			offset_x = -MIN(-offset_x, MAX(0, (img_rect.width - width)));
			view->view_zoom_x += offset_x;
		}

		if(offset_y > 0)
		{
			offset_y = MIN(offset_y, MAX(0, -img_rect.y));
			view->view_zoom_y += offset_y;
		}
		else if(offset_y < 0)
		{
			offset_y = -MIN(-offset_y, MAX(0, (img_rect.height - height)));
			view->view_zoom_y += offset_y;
		}

		view->drag_data.drag_x = event->x;  // minus to right, plus to left
		view->drag_data.drag_y = event->y;  //  minus to right, plus to left


//		offset_x = min(offset_x, )
//
//
//
//		view->view_zoom_x = min(0, view->view_zoom_x + offset_x);

//		view->view_zoom_x += offset_x;
//		view->view_zoom_y += offset_y;
//		if(event->x - (view->img_zoom_x * view->scale) < 0 &&
//				(event->x + (view->img_width - view->img_zoom_x) * view->scale) > width)
//					view->view_zoom_x = event->x;
//
//		if(event->y - (view->img_zoom_y * view->scale) < 0 &&
//				(event->y + (view->img_height - view->img_zoom_y) * view->scale) > height)
//					view->view_zoom_y = event->y;

//		if(event->x > view->img_zoom_x)
//			view->view_zoom_x = view->img_zoom_x;
//		if(event->x < img_right - ((view->img_width - view->img_zoom_x)/view->scale))
//				view->view_zoom_x = img_right - ((view->img_width - view->img_zoom_x)/view->scale);



		//view->view_zoom_y = event->y;
//		view->img_zoom_x = (event->x - img_left)/view->scale;
//		view->img_zoom_y = (event->y - img_top)/view->scale;



//		view->offset_x -= offset_x;
//		view->offset_y -= offset_y;
//
//		//calculate image mouse coordinates
//		int win_width = gtk_widget_get_allocated_width (widget);
//		int win_height = gtk_widget_get_allocated_height (widget);
//	    int img_width = gdk_pixbuf_get_width (view->image);
//	    int img_height = gdk_pixbuf_get_height (view->image);
//
//	    int img_left = win_width - (img_width * view->scale) / 2;
//	    int img_top = win_height - (img_height * view->scale) / 2;
//	    int img_right = img_left + (img_width * view->scale);
//	    int img_bottom = img_top + (img_height * view->scale);
//
//		int mouse_img_x;
//		int mouse_img_y;
//
//		if((view->img_mouse_x > img_left) && (view->img_mouse_x < img_right))
//		{
//			mouse_img_x = (view->img_mouse_x - img_left) * (1/view->scale);
//		}
//		else
//		{
//			mouse_img_x = 0;
//		}
//
//		if((view->img_mouse_y > img_top) && (view->img_mouse_y < img_bottom))
//		{
//			mouse_img_y = (view->img_mouse_y - img_top)  * (1/view->scale);
//		}
//		else
//		{
//			mouse_img_y = 0;
//		}
//
//		//event->x
//		//event->y
//	    view->img_mouse_x = event->x;
//	    view->img_mouse_y = event->y;
//
//		image_view_draw_surface (GTK_WIDGET(view));
//		gtk_widget_queue_draw (GTK_WIDGET(view));
//
//
		g_print("Dragging: %i x %i\n", offset_x, offset_y);
//		g_print("Mouse: %i x %i\n", event->x, event->y);
//
	}
	else
	{
//		//calculate image mouse coordinates
//		int win_width = gtk_widget_get_allocated_width (widget);
//		int win_height = gtk_widget_get_allocated_height (widget);
//	    int img_width = gdk_pixbuf_get_width (view->image);
//	    int img_height = gdk_pixbuf_get_height (view->image);
//
//	    int img_left = win_width - (img_width * view->scale) / 2;
//	    int img_top = win_height - (img_height * view->scale) / 2;
//	    int img_right = img_left + (img_width * view->scale);
//	    int img_bottom = img_top + (img_height * view->scale);
//
//		//event->x
//		//event->y
//	    view->img_mouse_x = event->x;
//	    view->img_mouse_y = event->y;
//
//		image_view_draw_surface (GTK_WIDGET(view));
//		gtk_widget_queue_draw (GTK_WIDGET(view));
//
//
////		g_print("Dragging: %i x %i\n", offset_x, offset_y);
////		g_print("Mouse: %i x %i\n", event->x, event->y);
	}

	image_view_draw_surface (GTK_WIDGET(view));
	gtk_widget_queue_draw (GTK_WIDGET(view));

	return TRUE;
}

static gboolean
image_view_button_release( GtkWidget *widget, GdkEventButton *event )
{
	g_print("ENTER: image_view_button_release\n");

	ImageView 			*view;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (IS_IMAGEVIEW (widget), FALSE);

	view = IMAGEVIEW(widget);

	view->drag_data.dragging = FALSE;
	view->drag_data.drag_x = 0;
	view->drag_data.drag_y = 0;

	return TRUE;
}

static gboolean
image_view_mouse_scroll( GtkWidget *widget, GdkEventScroll *event )
{
	g_print("ENTER: image_view_mouse_scroll\n");

	ImageView 			*view;
	GdkRectangle 		img_rect;
	GdkRectangle 		view_rect;


	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (IS_IMAGEVIEW (widget), FALSE);

	view = IMAGEVIEW(widget);

	int width = gtk_widget_get_allocated_width (widget);
	int height = gtk_widget_get_allocated_height (widget);

	g_print("INFO: UP Mouse Delta X: %f\n", event->delta_x);
	g_print("INFO: UP Mouse Delta Y: %f\n", event->delta_y);
	g_print("INFO: UP Mouse X: %f\n", event->x);
	g_print("INFO: UP Mouse Y: %f\n", event->y);

	if(event->direction == GDK_SCROLL_UP)
	{
		if(view->scale > IMG_VIEW_ZOOM_MAX - IMG_VIEW_ZOOM_STEP)
		{
			return TRUE;
		}

//		int img_left = (view->view_zoom_x - (view->img_zoom_x * view->scale)) - view->scale/2;
//		int img_top = (view->view_zoom_y - (view->img_zoom_y * view->scale)) - view->scale/2;
//		int img_right = img_left + (view->img_width * view->scale);
//		int img_bottom = img_top + (view->img_height * view->scale);

		image_view_get_scaled_rect(widget, &img_rect);
//		img_rect.x -= view->scale/2;
//		img_rect.y -= view->scale/2;
//		img_rect.width -= view->scale/2;
//		img_rect.height -= view->scale/2;

		if((view->img_width * view->scale) > width
				&& event->y > img_rect.y
				&& event->y < img_rect.height)
		{
			view->view_zoom_x = event->x;
			view->img_zoom_x = (event->x - img_rect.x)/view->scale;
		}
		else
		{
			view->view_zoom_x = width/2;
			view->img_zoom_x = view->img_width/2;
		}

		if((view->img_height * view->scale) > height
				&& event->x > img_rect.x
				&& event->x < img_rect.width)
		{
			view->view_zoom_y = event->y;
			view->img_zoom_y = (event->y - img_rect.y)/view->scale;
		}
		else
		{
			view->view_zoom_y = height/2;
			view->img_zoom_y = view->img_height/2;
		}

		view->fit = FALSE;
		view->scale = MIN ( (view->scale + IMG_VIEW_ZOOM_STEP),
							IMG_VIEW_ZOOM_MAX);

		image_view_draw_surface (GTK_WIDGET(view));
		gtk_widget_queue_draw (GTK_WIDGET(view));
	}
	else if(event->direction == GDK_SCROLL_DOWN)
	{
		if(view->scale < (IMG_VIEW_ZOOM_MIN + IMG_VIEW_ZOOM_STEP))
		{
			return TRUE;
		}

//		int img_left = (view->view_zoom_x - (view->img_zoom_x * view->scale)) - view->scale/2;
//		int img_top = (view->view_zoom_y - (view->img_zoom_y * view->scale)) - view->scale/2;
//		int img_right = img_left + (view->img_width * view->scale);
//		int img_bottom = img_top + (view->img_height * view->scale);
		//view_rect
		//set_rectangle(&view_rect, 0,0,0,0);
		image_view_get_scaled_rect(widget, &img_rect);

		if((view->img_width * view->scale) > width
				&& event->y > img_rect.y
				&& event->y < img_rect.height)
		{
			view->view_zoom_x = event->x;
			view->img_zoom_x = (event->x - img_rect.x)/view->scale;
		}
		else
		{
			view->view_zoom_x = width/2;
			view->img_zoom_x = view->img_width/2;
		}

		if((view->img_height * view->scale) > height
				&& event->x > img_rect.x
				&& event->x < img_rect.width)
		{
			view->view_zoom_y = event->y;
			view->img_zoom_y = (event->y - img_rect.y)/view->scale;
		}
		else
		{
			view->view_zoom_y = height/2;
			view->img_zoom_y = view->img_height/2;
		}

		view->fit = FALSE;
		view->scale = MAX ( (view->scale - IMG_VIEW_ZOOM_STEP),
							IMG_VIEW_ZOOM_MIN);

		image_view_draw_surface (GTK_WIDGET(view));
		gtk_widget_queue_draw (GTK_WIDGET(view));
	}
	return TRUE;
}

//GDK_BUTTON1_MOTION_MASK


static void
image_view_class_init (ImageViewClass *class)
{
	g_print("ENTER: image_view_class_init\n");

	GTK_WIDGET_CLASS(class)->draw = image_view_draw;
	GTK_WIDGET_CLASS(class)->configure_event = image_view_configure_event;
	GTK_WIDGET_CLASS(class)->motion_notify_event = image_view_mouse_move;
	GTK_WIDGET_CLASS(class)->button_press_event = image_view_button_press;
	GTK_WIDGET_CLASS(class)->button_release_event = image_view_button_release;
	GTK_WIDGET_CLASS(class)->scroll_event = image_view_mouse_scroll;

	//image_view_dispose
}


static void
image_view_init (ImageView *view)
{
	g_print("ENTER: image_view_init\n");

	view->surface = NULL;
	view->image = NULL;
	view->img_surface = NULL;
	view->img_width = 0;
	view->img_height = 0;

	view->fit = TRUE;
	view->rotation = 0;
	view->scale = 1.00;
	view->file = NULL;

	view->view_zoom_x = 0;
	view->view_zoom_y = 0;
	view->img_zoom_x = 0;
	view->img_zoom_y = 0;

	view->view_mouse_x = 0;
	view->view_mouse_y = 0;
	view->img_mouse_x = 0;
	view->img_mouse_y = 0;

	gtk_widget_set_events (GTK_WIDGET(view),
			GDK_POINTER_MOTION_MASK|
			GDK_BUTTON1_MOTION_MASK|
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK|
			GDK_SCROLL_MASK
			);
//			GDK_EXPOSURE_MASK |
//			GDK_LEAVE_NOTIFY_MASK |
//			GDK_BUTTON_PRESS_MASK |
//			GDK_POINTER_MOTION_MASK |
//			GDK_POINTER_MOTION_HINT_MASK);
	gtk_widget_set_double_buffered (GTK_WIDGET (view), FALSE);
}


ImageView *
image_view_new ()
{
  g_print("ENTER: image_view_new\n");

  return g_object_new (IMAGEVIEW_TYPE, NULL);
}


void
image_view_open (ImageView *view, GFile *file)
{
	g_print("ENTER: image_view_open\n");

	g_return_if_fail (view != NULL);
	g_return_if_fail (IS_IMAGEVIEW (view));

	view->file = file;

	if(view->surface == NULL)
	{
		g_print("Window surface not ready!");
		return;
	}

	image_view_create_image_surface(GTK_WIDGET(view));
    image_view_draw_surface (GTK_WIDGET(view));
    gtk_widget_queue_draw (GTK_WIDGET(view));
}


void
image_view_set_fit(ImageView *view, gboolean ffit)
{
	g_print("ENTER: image_view_set_fit\n");

	g_return_if_fail (view != NULL);
	g_return_if_fail (IS_IMAGEVIEW (view));

	if(view->fit != ffit)
	{
		view->fit = ffit;
		if(view->fit == TRUE)
		{
			view->scale = 1.00;
			view->view_zoom_x = gtk_widget_get_allocated_width (GTK_WIDGET(view))/2;
			view->img_zoom_x = view->img_width/2;
			view->view_zoom_y = gtk_widget_get_allocated_height (GTK_WIDGET(view))/2;
			view->img_zoom_y = view->img_height/2;
		}

		image_view_draw_surface (GTK_WIDGET(view));
		gtk_widget_queue_draw (GTK_WIDGET(view));
	}
}


gboolean
image_view_get_fit(ImageView *view)
{
	g_print("ENTER: image_view_get_fit\n");

	g_return_val_if_fail (view != NULL, FALSE);
	g_return_val_if_fail (IS_IMAGEVIEW (view), FALSE);

	return view->fit;
}


guint image_view_rotate_left (ImageView *view)
{
	g_print("ENTER: image_view_rotate_left\n");

	g_return_val_if_fail (view != NULL, FALSE);
	g_return_val_if_fail (IS_IMAGEVIEW (view), FALSE);

	view->rotation = (view->rotation-1) % 4;
	if (view->rotation < 0)
		view->rotation = 3;
	view->fit = TRUE;

	image_view_draw_surface (GTK_WIDGET(view));
	gtk_widget_queue_draw (GTK_WIDGET(view));

	return (view->rotation * 90);
}


guint image_view_rotate_right (ImageView *view)
{
	g_print("ENTER: image_view_rotate_right\n");

	g_return_val_if_fail (view != NULL, FALSE);
	g_return_val_if_fail (IS_IMAGEVIEW (view), FALSE);

	view->rotation = (view->rotation+1) % 4;
	view->fit = TRUE;

	image_view_draw_surface (GTK_WIDGET(view));
	gtk_widget_queue_draw (GTK_WIDGET(view));

	return (view->rotation * 90);
}

gfloat
image_view_zoom_in(ImageView *view)
{
	g_print("ENTER: image_view_zoom_in\n");

	g_return_val_if_fail (view != NULL, FALSE);
	g_return_val_if_fail (IS_IMAGEVIEW (view), FALSE);

	view->fit = FALSE;
	view->scale = MIN ((view->scale + 0.10), 30.00);

	image_view_draw_surface (GTK_WIDGET(view));
	gtk_widget_queue_draw (GTK_WIDGET(view));
}

gfloat
image_view_zoom_out(ImageView *view)
{
	g_print("ENTER: image_view_zoom_out\n");

	g_return_val_if_fail (view != NULL, FALSE);
	g_return_val_if_fail (IS_IMAGEVIEW (view), FALSE);

	view->fit = FALSE;
	view->scale = MAX ((view->scale - 0.10), 0.01);

	image_view_draw_surface (GTK_WIDGET(view));
	gtk_widget_queue_draw (GTK_WIDGET(view));
}

gfloat
image_view_zoom_set(ImageView *view, gfloat zoom)
{

}

