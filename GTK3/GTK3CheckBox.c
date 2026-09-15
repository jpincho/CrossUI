#include "GTK3Backend.h"
#include "GTK3CheckBox.h"
#include <Platform/Platform.h>
#include <Platform/Logger.h>
#include <stdlib.h>
#include <gtk/gtk.h>

static void OnCheckBoxToggled ( GtkToggleButton *Button, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	Widget->CheckBoxData.Checked = gtk_toggle_button_get_active ( Button ) ? true : false;
	if ( Widget->Callbacks.CheckChanged )
		Widget->Callbacks.CheckChanged ( cuiInternal_WidgetToHandle ( Widget ), Widget->CheckBoxData.Checked );
	}

bool cuiBackend_CheckBox_Create ( cuiWidget *Widget )
	{
	Widget->NativeHandle = gtk_check_button_new_with_label ( Widget->Text );
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( Widget->NativeHandle ), Widget->CheckBoxData.Checked ? TRUE : FALSE );
	g_signal_connect ( Widget->NativeHandle, "toggled", G_CALLBACK ( OnCheckBoxToggled ), Widget );
	return true;
	}

void cuiBackend_CheckBox_SetState ( cuiWidget *Widget, const bool State )
	{
	GtkWidget *Native = GetInnermostWidget ( Widget );
	ASSERT_FAIL ( Widget->Type == cuiType_CheckBox );
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( Native ), State ? TRUE : FALSE );
	}

bool cuiBackend_CheckBox_GetState ( cuiWidget *Widget )
	{
	GtkWidget *Native = GetInnermostWidget ( Widget );
	ASSERT_FAIL ( Widget->Type == cuiType_CheckBox );
	return gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( Native ) ) ? true : false;
	}
