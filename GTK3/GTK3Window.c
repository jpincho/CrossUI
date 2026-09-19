#include "GTK3BackendInternal.h"

static gboolean OnDelete ( GtkWidget *Native, GdkEvent *Event, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	UNUSED ( Native );
	UNUSED ( Event );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) )
		return TRUE;

	cuiInternal_DestroyWidgetEntry ( Widget );
	return TRUE;
	}

static gboolean OnConfigure ( GtkWidget *Native, GdkEvent *Event, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	unsigned NewWidth;
	unsigned NewHeight;
	UNUSED ( Native );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return FALSE;
	NewWidth = ( Event->configure.width < 0 ) ? 0 : ( unsigned ) Event->configure.width;
	NewHeight = ( Event->configure.height < 0 ) ? 0 : ( unsigned ) Event->configure.height;
	if ( ( NewWidth != Widget->Width ) || ( NewHeight != Widget->Height ) )
		{
		if ( Widget->Callbacks.Resized )
			Widget->Callbacks.Resized ( Widget, NewWidth, NewHeight );
		Widget->Width = NewWidth;
		Widget->Height = NewHeight;
		}
	if ( ( Event->configure.x != Widget->X ) || ( Event->configure.y != Widget->Y ) )
		{
		if ( Widget->Callbacks.Moved )
			Widget->Callbacks.Moved ( Widget, Event->configure.x, Event->configure.y );
		Widget->X = Event->configure.x;
		Widget->Y = Event->configure.y;
		}
	return FALSE;
	}

cuiWidget *cuiCreateWindow ( const char *Title, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *Widget = cuiInternal_CreateWidgetEntry ( cuiType_Window, NULL, Title, X, Y, Width, Height );
	if ( Widget == NULL )
		return cuiWidget_Invalid;

	GtkWidget *Fixed = gtk_fixed_new ();
	GtkWidget *Native = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title ( GTK_WINDOW ( Native ), EMPTY_STRING_IF_NULL ( Widget->Text ) );
	gtk_window_set_default_size ( GTK_WINDOW ( Native ), ( gint ) Widget->Width, ( gint ) Widget->Height );
	if ( ( Widget->X >= 0 ) && ( Widget->Y >= 0 ) )
		gtk_window_move ( GTK_WINDOW ( Native ), Widget->X, Widget->Y );
	else
		gtk_window_set_position ( GTK_WINDOW ( Native ), GTK_WIN_POS_CENTER );
	gtk_container_add ( GTK_CONTAINER ( Native ), Fixed );
	Widget->NativeHandle = Native;
	Widget->NativeLayoutHandle = Fixed;
	g_signal_connect ( Native, "delete-event", G_CALLBACK ( OnDelete ), Widget );
	g_signal_connect ( Native, "configure-event", G_CALLBACK ( OnConfigure ), Widget );
	gtk_widget_show_all ( Native );
	return Widget;
	}

