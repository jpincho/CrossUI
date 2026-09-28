#include "GTK3BackendInternal.h"

cuiWidget *cuiCreateLabel ( cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *Widget = cuiInternal_CreateWidgetEntry ( cuiType_Label, ParentWidget, Text, X, Y, Width, Height );
	if ( Widget == NULL )
		return NULL;
	Widget->NativeHandle = gtk_label_new ( EMPTY_STRING_IF_NULL ( Widget->Text ) );
	gtk_label_set_xalign ( GTK_LABEL ( Widget->NativeHandle ), 0.0f );
	gtk_label_set_yalign ( GTK_LABEL ( Widget->NativeHandle ), 0.5f );
	FinishedCreatingNewWidget ( Widget );
	return Widget;
	}
