#include "GTK3BackendInternal.h"

static void OnTextBoxClicked ( GtkButton *Button, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	UNUSED ( Button );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;
	if ( Widget->Callbacks.Clicked )
		Widget->Callbacks.Clicked ( Widget );
	}

static void OnEntryChanged ( GtkEditable *Editable, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	UNUSED ( Editable );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;

	if ( Widget->Callbacks.TextBoxChanged )
		Widget->Callbacks.TextBoxChanged ( Widget );
	}

cuiWidget *cuiCreateTextBox ( cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *Widget = cuiInternal_CreateWidgetEntry ( cuiType_TextBox, ParentWidget, Text, X, Y, Width, Height );
	if ( Widget == NULL )
		return NULL;

	Widget->NativeHandle = gtk_entry_new ();
	gtk_entry_set_text ( GTK_ENTRY ( Widget->NativeHandle ), Widget->Text );
	g_signal_connect ( Widget->NativeHandle, "changed", G_CALLBACK ( OnEntryChanged ), Widget );
	g_signal_connect ( Widget->NativeHandle, "activate", G_CALLBACK ( OnTextBoxClicked ), Widget );
	FinishedCreatingNewWidget ( Widget );
	return Widget;
	}
