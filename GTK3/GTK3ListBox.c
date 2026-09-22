#include "GTK3BackendInternal.h"

static GtkListStore *GetListStore ( const cuiWidget *Widget )
	{
	return GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( Widget->NativeHandle ) ) );
	}

static void OnListSelectionChanged ( GtkTreeSelection *Selection, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;

	int Index = -1;
	GtkTreeModel *Model = NULL;
	GtkTreeIter Iter;
	if ( gtk_tree_selection_get_selected ( Selection, &Model, &Iter ) )
		{
		GtkTreePath *Path = gtk_tree_model_get_path ( Model, &Iter );
		gint *Indices = gtk_tree_path_get_indices ( Path );
		Index = ( Indices != NULL ) ? Indices[0] : -1;
		gtk_tree_path_free ( Path );
		}
	if ( Widget->Callbacks.ListBoxSelectionChanged )
		Widget->Callbacks.ListBoxSelectionChanged ( Widget, Index );
	}

cuiWidget *cuiCreateListBox ( cuiWidget *ParentWidget, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *Widget = cuiInternal_CreateWidgetEntry ( cuiType_ListBox, ParentWidget, NULL, X, Y, Width, Height );
	if ( Widget == NULL )
		return NULL;

	GtkListStore *Store = gtk_list_store_new ( 1, G_TYPE_STRING );
	GtkWidget *View = gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( Store ) );
	GtkCellRenderer *Renderer = gtk_cell_renderer_text_new ();
	GtkTreeViewColumn *Column = gtk_tree_view_column_new_with_attributes ( "Item", Renderer, "text", 0, NULL );
	GtkTreeSelection *Selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( View ) );
	gtk_tree_view_append_column ( GTK_TREE_VIEW ( View ), Column );
	gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW ( View ), FALSE );
	gtk_tree_selection_set_mode ( Selection, GTK_SELECTION_SINGLE );
	g_object_set_data ( G_OBJECT ( View ), "cui-store", Store );
	g_object_unref ( Store );
	g_signal_connect ( Selection, "changed", G_CALLBACK ( OnListSelectionChanged ), Widget );

	GtkWidget *Scroll = gtk_scrolled_window_new ( NULL, NULL );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( Scroll ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( Scroll ), GTK_SHADOW_IN );
	gtk_container_add ( GTK_CONTAINER ( Scroll ), View );

	Widget->ScrollHandle = Scroll;
	Widget->NativeHandle = View;

	FinishedCreatingNewWidget ( Widget );
	return Widget;
	}

void cuiClearListBoxItems ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ListBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;
	GtkListStore *ListStore = GetListStore ( Widget );
	gtk_list_store_clear ( ListStore );
	}

int cuiAddItemToListBox ( const cuiWidget *Widget, const char *Text )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ListBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return -1;

	GtkListStore *ListStore = GetListStore ( Widget );
	GtkTreeIter Iter;
	gtk_list_store_append ( ListStore, &Iter );
	gtk_list_store_set ( ListStore, &Iter, 0, EMPTY_STRING_IF_NULL ( Text ), -1 );
	return gtk_tree_model_iter_n_children ( GTK_TREE_MODEL ( ListStore ), NULL ) -1;
	}

void cuiRemoveItemFromListBox ( const cuiWidget *Widget, const int Index )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ListBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;
	if ( Index < 0 )
		return;
	GtkListStore *ListStore = GetListStore ( Widget );
	GtkTreeIter Iter;
	if ( gtk_tree_model_iter_nth_child ( GTK_TREE_MODEL ( ListStore ), &Iter, NULL, Index ) )
		gtk_list_store_remove ( ListStore, &Iter );
	}

void cuiSetSelectedItemInListBox ( const cuiWidget *Widget, const int Index )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ListBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;

	GtkTreeSelection *Selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( Widget->NativeHandle ) );
	gtk_tree_selection_unselect_all ( Selection );
	if ( Index >= 0 )
		{
		GtkListStore *ListStore = GetListStore ( Widget );
		if ( Index < gtk_tree_model_iter_n_children ( GTK_TREE_MODEL ( ListStore ), NULL ) )
			{
			GtkTreePath *Path = gtk_tree_path_new_from_indices ( Index, -1 );
			gtk_tree_selection_select_path ( Selection, Path );
			gtk_tree_path_free ( Path );
			}
		}
	}

int cuiGetSelectedItemInListBox ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ListBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return -1;

	GtkTreeSelection *Selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( Widget->NativeHandle ) );
	GtkTreeModel *Model = NULL;
	GtkTreeIter Iter;
	if ( gtk_tree_selection_get_selected ( Selection, &Model, &Iter ) == FALSE )
		return -1;
	GtkTreePath *Path = gtk_tree_model_get_path ( Model, &Iter );
	gint *Indices = gtk_tree_path_get_indices ( Path );
	int Index = ( Indices != NULL ) ? Indices[0] : -1;
	gtk_tree_path_free ( Path );
	return Index;
	}

int cuiGetEntryCountInListBox ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ListBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return -1;

	GtkListStore *ListStore = GetListStore ( Widget );
	return gtk_tree_model_iter_n_children ( GTK_TREE_MODEL ( ListStore ), NULL );
	}

void cuiGetListBoxItemText ( const cuiWidget *Widget, const int Index, char *Buffer, const unsigned BufferSize )
	{
	GtkListStore *ListStore;
	GtkTreeIter Iter;
	char *Text = NULL;
	if ( ( Buffer == NULL ) || ( BufferSize == 0 ) )
		return;
	Buffer[0] = 0;
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_ListBox );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) || ( Index < 0 ) )
		return;
	ListStore = GetListStore ( Widget );
	if ( ( ListStore == NULL ) || ( gtk_tree_model_iter_nth_child ( GTK_TREE_MODEL ( ListStore ), &Iter, NULL, Index ) == FALSE ) )
		return;
	gtk_tree_model_get ( GTK_TREE_MODEL ( ListStore ), &Iter, 0, &Text, -1 );
	if ( Text != NULL )
		{
		strncpy ( Buffer, Text, BufferSize - 1 );
		Buffer[BufferSize - 1] = 0;
		g_free ( Text );
		}
	}