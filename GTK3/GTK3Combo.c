#include "GTK3BackendInternal.h"

static void OnComboChanged ( GtkComboBox *Combo, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;

	int NewIndex = gtk_combo_box_get_active ( Combo );
	if ( Widget->Callbacks.ComboBoxChanged )
		Widget->Callbacks.ComboBoxChanged ( Widget, NewIndex );
	}

cuiWidget *cuiCreateCombo ( cuiWidget *ParentWidget, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *Widget = cuiInternal_CreateWidgetEntry ( cuiType_ComboBox, ParentWidget, NULL, X, Y, Width, Height );
	if ( Widget == NULL )
		return NULL;
	Widget->NativeHandle = gtk_combo_box_text_new ();
	g_signal_connect ( Widget->NativeHandle, "changed", G_CALLBACK ( OnComboChanged ), Widget );

	FinishedCreatingNewWidget ( Widget );
	return Widget;
	}

void cuiClearCombo ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ComboBox );
	GtkWidget *Native = GetInnermostWidget ( Widget );
	gtk_combo_box_text_remove_all ( GTK_COMBO_BOX_TEXT ( Native ) );
	}

int cuiAddItemToCombo ( cuiWidget *Widget, const char *Text )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ComboBox );
	GtkWidget *Native = GetInnermostWidget ( Widget );
	gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT ( Native ), Text );
	GtkTreeModel *Model = gtk_combo_box_get_model ( GTK_COMBO_BOX ( Native ) );
	return gtk_tree_model_iter_n_children ( Model, NULL ) -1;
	}

void cuiRemoveItemFromCombo ( cuiWidget *Widget, const int Index )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ComboBox );
	ASSERT_FAIL ( Index >= 0 );
	GtkWidget *Native = GetInnermostWidget ( Widget );

	gtk_combo_box_text_remove ( GTK_COMBO_BOX_TEXT ( Native ), Index );
	}

void cuiSetSelectedItemInCombo ( const cuiWidget *Widget, const int Index )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ComboBox );
	ASSERT_FAIL ( Index >= 0 );
	GtkWidget *Native = GetInnermostWidget ( Widget );
	gtk_combo_box_set_active ( GTK_COMBO_BOX ( Native ), Index );
	}

int cuiGetSelectedItemInCombo ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ComboBox );
	GtkWidget *Native = GetInnermostWidget ( Widget );
	if ( Native == NULL )
		return -1;
	return gtk_combo_box_get_active ( GTK_COMBO_BOX ( Native ) );
	}

int cuiGetEntryCountInCombo ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ComboBox );
	GtkWidget *Native = GetInnermostWidget ( Widget );
	if ( Native == NULL )
		return -1;
	GtkTreeModel *Model = gtk_combo_box_get_model ( GTK_COMBO_BOX ( Native ) );

	return gtk_tree_model_iter_n_children ( Model, NULL );
	}
