#include "GTK3BackendInternal.h"

cuiWidget *cuiCreateProgressBar ( cuiWidget *ParentWidget, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *Widget = cuiInternal_CreateWidgetEntry ( cuiType_ProgressBar, ParentWidget, NULL, X, Y, Width, Height );
	if ( Widget == NULL )
		return NULL;
	Widget->NativeHandle = gtk_progress_bar_new ();
	gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR ( Widget->NativeHandle ), 0.0 );
	gtk_progress_bar_set_show_text ( GTK_PROGRESS_BAR ( Widget->NativeHandle ), TRUE );
	FinishedCreatingNewWidget ( Widget );
	return Widget;
	}

void cuiSetProgressBarValue ( cuiWidget *Widget, const float Value )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ProgressBar );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;

	char Label[16];
	float FinalValue = CLAMP ( Value, 0.0f, 100.0f );

	gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR ( Widget->NativeHandle ), FinalValue / 100.0 );
	snprintf ( Label, sizeof ( Label ), "%6.3f%%", FinalValue );
	gtk_progress_bar_set_text ( GTK_PROGRESS_BAR ( Widget->NativeHandle ), Label );
	}
