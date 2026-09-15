#include "GTK3Backend.h"
#include "GTK3Button.h"
#include <Platform/Platform.h>
#include <Platform/Logger.h>
#include <stdlib.h>
#include <gtk/gtk.h>

static void OnButtonClicked ( GtkButton *Button, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	UNUSED ( Button );
	if ( Widget->Callbacks.Clicked )
		Widget->Callbacks.Clicked ( cuiInternal_WidgetToHandle ( Widget ) );
	}

bool cuiBackend_Button_Create ( cuiWidget *Widget )
	{
	Widget->NativeHandle = gtk_button_new_with_label ( Widget->Text );
	g_signal_connect ( Widget->NativeHandle, "clicked", G_CALLBACK ( OnButtonClicked ), Widget );
	return true;
	}
