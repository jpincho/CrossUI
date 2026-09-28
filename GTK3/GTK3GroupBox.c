#include "GTK3BackendInternal.h"

cuiWidget *cuiCreateGroupBox ( cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *Widget = cuiInternal_CreateWidgetEntry ( cuiType_GroupBox, ParentWidget, Text, X, Y, Width, Height );
	if ( Widget == NULL )
		return NULL;

	Widget->NativeLayoutHandle = gtk_fixed_new ();
	Widget->NativeHandle = gtk_frame_new ( Widget->Text );
	gtk_container_add ( GTK_CONTAINER ( Widget->NativeHandle ), Widget->NativeLayoutHandle );
	FinishedCreatingNewWidget ( Widget );
	return Widget;
	}