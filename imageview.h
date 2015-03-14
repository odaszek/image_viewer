#ifndef __IMAGEVIEW_H
#define __IMAGEVIEW_H

#include <gtk/gtk.h>

#define IMAGEVIEW_TYPE (image_view_get_type ())
#define IMAGEVIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), IMAGEVIEW_TYPE, ImageView))
#define IS_IMAGEVIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IMAGEVIEW_TYPE))
#define IMAGEVIEW_CLASS(klass)(G_TYPE_CHECK_CLASS_CAST ((klass), IMAGEVIEW_TYPE, ImageViewClass))
#define IS_IMAGEVIEW_CLASS(klass)(G_TYPE_CHECK_CLASS_TYPE ((klass), IMAGEVIEW_TYPE))
#define IMAGEVIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), IMAGEVIEW_TYPE, ImageViewClass))

typedef struct _ImageView         ImageView;
typedef struct _ImageViewClass    ImageViewClass;

GType           	   image_view_get_type     (void);
ImageView 		      *image_view_new          (void);

void                   image_view_open         (ImageView *view, GFile *file);
void				   image_view_set_fit	   (ImageView *view, gboolean fit);
gboolean			   image_view_get_fit	   (ImageView *view);
guint				   image_view_rotate_left  (ImageView *view);
guint				   image_view_rotate_right (ImageView *view);
gfloat				   image_view_zoom_in      (ImageView *view);
gfloat				   image_view_zoom_out     (ImageView *view);
gfloat				   image_view_zoom_set     (ImageView *view, gfloat zoom);

#endif /* __IMAGEVIEW_H */
