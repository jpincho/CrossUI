#include "GTK3BackendInternal.h"

static void OnBufferChanged ( GtkTextBuffer *Buffer, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	UNUSED ( Buffer );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;
	if ( Widget->Callbacks.TextAreaChanged )
		Widget->Callbacks.TextAreaChanged ( Widget );
	}

cuiWidget *cuiCreateTextArea ( cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *Widget = cuiInternal_CreateWidgetEntry ( cuiType_TextArea, ParentWidget, Text, X, Y, Width, Height );
	if ( Widget == NULL )
		return NULL;

	GtkWidget *View = gtk_text_view_new ();
	GtkTextBuffer *Buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( View ) );
	gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW ( View ), GTK_WRAP_WORD_CHAR );
	gtk_text_buffer_set_text ( Buffer, Widget->Text, -1 );
	g_signal_connect ( Buffer, "changed", G_CALLBACK ( OnBufferChanged ), Widget );

	GtkWidget *Scroll = gtk_scrolled_window_new ( NULL, NULL );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( Scroll ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( Scroll ), GTK_SHADOW_IN );
	gtk_container_add ( GTK_CONTAINER ( Scroll ), View );

	Widget->TextAreaBuffer = Buffer;
	Widget->TextAreaBufferView = View;
	Widget->NativeHandle = Scroll;
	FinishedCreatingNewWidget ( Widget );
	return Widget;
	}
