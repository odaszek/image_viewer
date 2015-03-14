#ifndef __APPLICATION_H
#define __APPLICATION_H

#include <gtk/gtk.h>

#define APPLICATION_TYPE (application_get_type ())
#define APPLICATION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), APPLICATION_TYPE, Application))


typedef struct _Application       Application;
typedef struct _ApplicationClass  ApplicationClass;


GType           application_get_type    (void);
Application     *application_new         (void);


#endif /* __APPLICATION_H */
