#include "GTK3Backend.h"
#include "GTK3Label.h"
#include <Platform/Platform.h>
#include <Platform/Logger.h>
#include <stdlib.h>
#include <gtk/gtk.h>

bool cuiBackend_Label_Create ( cuiWidget *Widget )
	{
	Widget->NativeHandle = gtk_label_new ( Widget->Text );
	gtk_label_set_xalign ( GTK_LABEL ( Widget->NativeHandle ), 0.0f );
	gtk_label_set_yalign ( GTK_LABEL ( Widget->NativeHandle ), 0.5f );
	return true;
	}
