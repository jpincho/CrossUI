#include "GTK3BackendInternal.h"

static void OnButtonClicked ( GtkButton *Button, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	UNUSED ( Button );
	if ( Widget->Callbacks.Clicked )
		Widget->Callbacks.Clicked ( Widget );
	}

cuiWidget *cuiCreateButton ( cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *Widget = cuiInternal_CreateWidgetEntry ( cuiType_Button, ParentWidget, Text, X, Y, Width, Height );
	if ( Widget == NULL )
		return NULL;
	Widget->NativeHandle = gtk_button_new_with_label ( EMPTY_STRING_IF_NULL ( Widget->Text ) );
	g_signal_connect ( Widget->NativeHandle, "clicked", G_CALLBACK ( OnButtonClicked ), Widget );
	FinishedCreatingNewWidget ( Widget );
	return Widget;
	}
