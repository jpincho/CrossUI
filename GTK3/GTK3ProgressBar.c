#include "GTK3Backend.h"
#include "GTK3ProgressBar.h"
#include <Platform/Platform.h>
#include <Platform/Logger.h>
#include <stdlib.h>
#include <gtk/gtk.h>

bool cuiBackend_ProgressBar_Create ( cuiWidget *Widget )
	{
	Widget->NativeHandle = gtk_progress_bar_new ();
	gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR ( Widget->NativeHandle ), Widget->ProgressBarData.Value / 100.0 );
	gtk_progress_bar_set_show_text ( GTK_PROGRESS_BAR ( Widget->NativeHandle ), TRUE );
	return true;
	}

void cuiBackend_ProgressBar_SetValue ( cuiWidget *Widget, const float Value )
	{
	GtkWidget *Native = GetInnermostWidget ( Widget );
	ASSERT_FAIL ( Widget->Type == cuiType_ProgressBar );

	char Label[16];
	gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR ( Native ), Value / 100.0 );
	snprintf ( Label, sizeof ( Label ), "%f%%", Value );
	gtk_progress_bar_set_text ( GTK_PROGRESS_BAR ( Native ), Label );
	}
