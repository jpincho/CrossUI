#include "GTK3BackendInternal.h"

static void OnComboChanged ( GtkComboBox *Combo, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;
	int NewIndex = gtk_combo_box_get_active ( Combo );
	if ( Widget->Callbacks.ComboBoxChanged )
		Widget->Callbacks.ComboBoxChanged ( Widget, NewIndex );
	}

cuiWidget *cuiCreateComboBox ( cuiWidget *ParentWidget, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *Widget = cuiInternal_CreateWidgetEntry ( cuiType_ComboBox, ParentWidget, NULL, X, Y, Width, Height );
	if ( Widget == NULL )
		return NULL;
	Widget->NativeHandle = gtk_combo_box_text_new ();
	g_signal_connect ( Widget->NativeHandle, "changed", G_CALLBACK ( OnComboChanged ), Widget );

	FinishedCreatingNewWidget ( Widget );
	return Widget;
	}

void cuiClearComboBox ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ComboBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;
	gtk_combo_box_text_remove_all ( GTK_COMBO_BOX_TEXT ( Widget->NativeHandle ) );
	}

int cuiAddItemToComboBox ( cuiWidget *Widget, const char *Text )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ComboBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return -1;

	GtkTreeModel *Model;
	int Count;
	gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT ( Widget->NativeHandle ), EMPTY_STRING_IF_NULL ( Text ) );
	Model = gtk_combo_box_get_model ( GTK_COMBO_BOX ( Widget->NativeHandle ) );
	Count = gtk_tree_model_iter_n_children ( Model, NULL );
	return ( Count > 0 ) ? ( Count - 1 ) : -1;
	}

void cuiRemoveItemFromComboBox ( cuiWidget *Widget, const int Index )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ComboBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) || ( Index < 0 ) )
		return;

	int Count = gtk_tree_model_iter_n_children ( gtk_combo_box_get_model ( GTK_COMBO_BOX ( Widget->NativeHandle ) ), NULL );
	if ( Index >= Count )
		return;
	gtk_combo_box_text_remove ( GTK_COMBO_BOX_TEXT ( Widget->NativeHandle ), Index );
	}

void cuiSetSelectedItemInComboBox ( const cuiWidget *Widget, const int Index )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ComboBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) || ( Index < 0 ) )
		return;

	gtk_combo_box_set_active ( GTK_COMBO_BOX ( Widget->NativeHandle ), Index );
	}

int cuiGetSelectedItemInComboBox ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ComboBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return -1;

	return gtk_combo_box_get_active ( GTK_COMBO_BOX ( Widget->NativeHandle ) );
	}

int cuiGetEntryCountInComboBox ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ComboBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return -1;

	GtkTreeModel *Model = gtk_combo_box_get_model ( GTK_COMBO_BOX ( Widget->NativeHandle ) );
	return gtk_tree_model_iter_n_children ( Model, NULL );
	}

void cuiGetComboBoxItemText ( const cuiWidget *Widget, const int Index, char *Buffer, const unsigned BufferSize )
	{
	GtkTreeModel *Model;
	GtkTreeIter Iter;
	char *Text = NULL;
	if ( ( Buffer == NULL ) || ( BufferSize == 0 ) )
		return;
	Buffer[0] = 0;
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ComboBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) || ( Index < 0 ) )
		return;
	Model = gtk_combo_box_get_model ( GTK_COMBO_BOX ( Widget->NativeHandle ) );
	if ( ( Model == NULL ) || ( gtk_tree_model_iter_nth_child ( Model, &Iter, NULL, Index ) == FALSE ) )
		return;
	gtk_tree_model_get ( Model, &Iter, 0, &Text, -1 );
	if ( Text != NULL )
		{
		strncpy ( Buffer, Text, BufferSize - 1 );
		Buffer[BufferSize - 1] = 0;
		g_free ( Text );
		}
	}
