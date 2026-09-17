#include "GTK3BackendInternal.h"

static void OnCheckBoxToggled ( GtkToggleButton *Button, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	bool Checked = gtk_toggle_button_get_active ( Button ) ? true : false;
	if ( Widget->Callbacks.CheckChanged )
		Widget->Callbacks.CheckChanged ( Widget, Checked );
	}

cuiWidget *cuiCreateCheckbox ( cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height, const bool State )
	{
	cuiWidget *Widget = cuiInternal_CreateWidgetEntry ( cuiType_CheckBox, ParentWidget, Text, X, Y, Width, Height );
	if ( Widget == NULL )
		return NULL;
	Widget->NativeHandle = gtk_check_button_new_with_label ( EMPTY_STRING_IF_NULL ( Widget->Text ) );
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( Widget->NativeHandle ), State ? TRUE : FALSE );
	g_signal_connect ( Widget->NativeHandle, "toggled", G_CALLBACK ( OnCheckBoxToggled ), Widget );
	FinishedCreatingNewWidget ( Widget );
	return Widget;
	}

void cuiSetCheckboxState ( cuiWidget *Widget, const bool State )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_CheckBox );
	GtkWidget *Native = GetInnermostWidget ( Widget );
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( Native ), State ? TRUE : FALSE );
	}

bool cuiGetCheckboxState ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_CheckBox );
	GtkWidget *Native = GetInnermostWidget ( Widget );
	return gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( Native ) ) ? true : false;
	}
