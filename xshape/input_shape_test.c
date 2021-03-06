/*******************************************************************************
**3456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 
**      10        20        30        40        50        60        70        80
**
** program:
**    input_shape_test
**
** created:
**    19.2.2006
**
** last change:
**    20.2.2006
**
** author:
**    Mirco "MacSlow" Mueller <macslow@bangang.de>
**
** license:
**    GPL
**
** notes:
**    - this is a test I did to figure out how to use input-shapes (XShape 1.1)
**    - opens a decoration-less and transparent gtk+-window and draws a red
**      cross with a circle around it
**    - only the parts drawn will be "draggable"
**    - window can be dragged around with LMB-drag
**    - window can be resized with MMB-drag (the input-shape also resizes!)
**    - window can be exited with ESC or q
**    - needs a compositing manager to run in order to look as intended
**    - tested with xcompmgr and compiz
**    - tested with gtk+-2.8.11 and gtk+-2.9.0 (CVS-head)
**
** bugs:
**    - there are no size-checks done for the input-shape, so I don't know what
**      will happen, if you make the window super large
**
** todo:
**    - nothing
**
** compile with:
**    gcc `pkg-config --cflags --libs gtk+-2.0` input_shape_test.c -o input_shape_test
**
*******************************************************************************/

#include <math.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#if !GTK_CHECK_VERSION(2,9,0)
#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <gdk/gdkx.h>
#endif

#define WIN_WIDTH 300
#define WIN_HEIGHT 300

gint g_iCurrentWidth = WIN_WIDTH;
gint g_iCurrentHeight = WIN_HEIGHT;

void update_input_shape (GtkWidget* pWindow, int iWidth, int iHeight);
void on_alpha_screen_changed (GtkWidget* pWidget, GdkScreen* pOldScreen, GtkWidget* pLabel);
void render (cairo_t* pCairoContext, gint iWidth, gint iHeight);
gboolean on_expose (GtkWidget* pWidget, GdkEventExpose* pExpose);
gboolean on_key_press (GtkWidget* pWidget, GdkEventKey* pKey, gpointer userData);
gboolean on_button_press (GtkWidget* pWidget, GdkEventButton* pButton, GdkWindowEdge edge);
gboolean on_configure (GtkWidget* pWidget, GdkEventConfigure* pEvent, gpointer data);
#if !GTK_CHECK_VERSION(2,9,0)
void do_shape_combine_mask (GdkWindow* window, GdkBitmap* mask, gint x, gint y);
#endif
void update_input_shape (GtkWidget* pWindow, int iWidth, int iHeight);

void on_alpha_screen_changed (GtkWidget* pWidget,
							  GdkScreen* pOldScreen,
							  GtkWidget* pLabel)
{                       
	GdkScreen* pScreen = gtk_widget_get_screen (pWidget);
	GdkColormap* pColormap = gdk_screen_get_rgba_colormap (pScreen);
      
	if (!pColormap)
		pColormap = gdk_screen_get_rgb_colormap (pScreen);

	gtk_widget_set_colormap (pWidget, pColormap);
}

void render (cairo_t* pCairoContext, gint iWidth, gint iHeight)
{
	cairo_scale (pCairoContext, (double) iWidth, (double) iHeight);
	cairo_set_source_rgba (pCairoContext, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	cairo_set_source_rgba (pCairoContext, 1.0f, 0.0f, 0.0f, 0.75f);
	cairo_set_line_width (pCairoContext, 0.1f);
	cairo_move_to (pCairoContext, 0.15f, 0.15f);
	cairo_line_to (pCairoContext, 0.85f, 0.85f);
	cairo_move_to (pCairoContext, 0.85f, 0.15f);
	cairo_line_to (pCairoContext, 0.15f, 0.85f);
	cairo_stroke (pCairoContext);
	cairo_arc (pCairoContext, 0.5f, 0.5f, 0.45f, 0.0f, M_PI/180.0f * 360.0f);
	cairo_stroke (pCairoContext);
}

gboolean on_expose (GtkWidget*		pWidget,
					GdkEventExpose*	pExpose)
{
	gint iWidth;
	gint iHeight;
	cairo_t* pCairoContext = NULL;

	pCairoContext = gdk_cairo_create (pWidget->window);
	if (!pCairoContext)
		return FALSE;

	gtk_window_get_size (GTK_WINDOW (pWidget), &iWidth, &iHeight);
	render (pCairoContext, iWidth, iHeight);
	cairo_destroy (pCairoContext);

	return FALSE;
}

gboolean on_configure (GtkWidget* pWidget,
					   GdkEventConfigure* pEvent,
					   gpointer userData)
{
	gint iNewWidth = pEvent->width;
	gint iNewHeight = pEvent->height;

	if (iNewWidth != g_iCurrentWidth || iNewHeight != g_iCurrentHeight)
	{
		update_input_shape (pWidget, iNewWidth, iNewHeight);
		g_iCurrentWidth = iNewWidth;
		g_iCurrentHeight = iNewHeight;
	}

	return FALSE;
}

gboolean on_key_press (GtkWidget*	pWidget,
					   GdkEventKey*	pKey,
					   gpointer		userData)
{
	gint iWidth;
	gint iHeight;

	if (pKey->type == GDK_KEY_PRESS)
	{
		switch (pKey->keyval)
		{
			case GDK_Escape :
			case GDK_q :
				gtk_main_quit ();
			break;
		}
	}

	return FALSE;
}

gboolean on_button_press (GtkWidget* pWidget,
						  GdkEventButton* pButton,
						  GdkWindowEdge edge)
{
	if (pButton->type == GDK_BUTTON_PRESS)
	{
		if (pButton->button == 1)
			gtk_window_begin_move_drag (GTK_WINDOW (gtk_widget_get_toplevel (pWidget)),
										pButton->button,
										pButton->x_root,
										pButton->y_root,
										pButton->time);
		if (pButton->button == 2)
			gtk_window_begin_resize_drag (GTK_WINDOW (gtk_widget_get_toplevel (pWidget)),
										  edge,
										  pButton->button,
										  pButton->x_root,
										  pButton->y_root,
										  pButton->time);
    }

	return FALSE;
}

#if !GTK_CHECK_VERSION(2,9,0)
/* this is piece by piece taken from gtk+ 2.9.0 (CVS-head with a patch applied
regarding XShape's input-masks) so people without gtk+ >= 2.9.0 can compile and
run input_shape_test.c */
void do_shape_combine_mask (GdkWindow* window,
							GdkBitmap* mask,
							gint x,
							gint y)
{
	Pixmap pixmap;
	int ignore;
	int maj;
	int min;

	if (!XShapeQueryExtension (GDK_WINDOW_XDISPLAY (window), &ignore, &ignore))
		return;

	if (!XShapeQueryVersion (GDK_WINDOW_XDISPLAY (window), &maj, &min))
		return;

	/* for shaped input we need at least XShape 1.1 */
	if (maj != 1 && min < 1)
		return;

	if (mask)
		pixmap = GDK_DRAWABLE_XID (mask);
	else
	{
		x = 0;
		y = 0;
		pixmap = None;
	}

	XShapeCombineMask (GDK_WINDOW_XDISPLAY (window),
					   GDK_DRAWABLE_XID (window),
					   ShapeInput,
					   x,
					   y,
					   pixmap,
					   ShapeSet);
}
#endif

void update_input_shape (GtkWidget* pWindow, int iWidth, int iHeight)
{
	static GdkBitmap* pShapeBitmap = NULL;
	static cairo_t* pCairoContext = NULL;

	pShapeBitmap = (GdkBitmap*) gdk_pixmap_new (NULL, iWidth, iHeight, 1);
	if (pShapeBitmap)
	{
		pCairoContext = gdk_cairo_create (pShapeBitmap);
		if (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS)
		{
			render (pCairoContext, iWidth, iHeight);
			cairo_destroy (pCairoContext);
#if !GTK_CHECK_VERSION(2,9,0)
			do_shape_combine_mask (pWindow->window, NULL, 0, 0);
			do_shape_combine_mask (pWindow->window, pShapeBitmap, 0, 0);
#else
			gtk_widget_input_shape_combine_mask (pWindow, NULL, 0, 0);
			gtk_widget_input_shape_combine_mask (pWindow, pShapeBitmap, 0, 0);
#endif
		}
		g_object_unref ((gpointer) pShapeBitmap);
	}
}

int main (int argc, char** argv)
{
	GtkWidget* pWindow = NULL;
	GdkBitmap* pShapeMaskBitmap = NULL;

	gtk_init (&argc, &argv);

	pWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	on_alpha_screen_changed (pWindow, NULL, NULL);
	gtk_widget_set_app_paintable (pWindow, TRUE);
	gtk_window_set_decorated (GTK_WINDOW (pWindow), FALSE);
	gtk_window_set_resizable (GTK_WINDOW (pWindow), TRUE);
	gtk_window_set_title (GTK_WINDOW (pWindow), "gtk+/XShape 1.1 test");
	gtk_widget_set_size_request (pWindow, g_iCurrentWidth, g_iCurrentHeight);
	gtk_widget_add_events (pWindow, GDK_BUTTON_PRESS_MASK);
	gtk_widget_show (pWindow);

	g_signal_connect (G_OBJECT (pWindow),
					  "destroy",
					  G_CALLBACK (gtk_main_quit),
					  NULL);
	g_signal_connect (G_OBJECT (pWindow),
					  "expose-event",
					  G_CALLBACK (on_expose),
					  NULL);
	g_signal_connect (G_OBJECT (pWindow),
					  "configure-event",
					  G_CALLBACK (on_configure),
					  NULL);
	g_signal_connect (G_OBJECT (pWindow),
					  "key-press-event",
					  G_CALLBACK (on_key_press),
					  NULL);
	g_signal_connect (G_OBJECT (pWindow),
					  "button-press-event",
					  G_CALLBACK (on_button_press),
					  NULL);

	update_input_shape (pWindow, g_iCurrentWidth, g_iCurrentHeight);

	gtk_main ();

	return 0;
}
