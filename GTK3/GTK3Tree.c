#include "GTK3BackendInternal.h"

#define CUI_TREE_COL_TEXT 0
#define CUI_TREE_COL_ITEM 1

typedef struct
	{
	GtkTreeRowReference *Ref;
	GtkTreeStore *Store;
	void *UserData;
	} cuiGTKTreeItem;

static GtkTreeStore *GetTreeStore ( const cuiWidget *Widget )
	{
	return GTK_TREE_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( Widget->NativeHandle ) ) );
	}

static bool FindTreeItemIndex ( GtkTreeModel *Model, GtkTreeIter *Parent, GtkTreePath *TargetPath, int *Index )
	{
	GtkTreeIter Iterator;
	if ( gtk_tree_model_iter_children ( Model, &Iterator, Parent ) == FALSE )
		return false;
	do
		{
		GtkTreePath *Path = gtk_tree_model_get_path ( Model, &Iterator );
		bool IsTarget = ( gtk_tree_path_compare ( Path, TargetPath ) == 0 );
		gtk_tree_path_free ( Path );
		if ( IsTarget )
			return true;
		++*Index;
		if ( FindTreeItemIndex ( Model, &Iterator, TargetPath, Index ) )
			return true;
		}
	while ( gtk_tree_model_iter_next ( Model, &Iterator ) );
	return false;
	}

static bool GetTreeItemAtIndex ( GtkTreeModel *Model, GtkTreeIter *Parent, const int TargetIndex, int *Index, GtkTreeIter *Result )
	{
	GtkTreeIter Iterator;
	if ( gtk_tree_model_iter_children ( Model, &Iterator, Parent ) == FALSE )
		return false;
	do
		{
		if ( *Index == TargetIndex )
			{
			*Result = Iterator;
			return true;
			}
		++*Index;
		if ( GetTreeItemAtIndex ( Model, &Iterator, TargetIndex, Index, Result ) )
			return true;
		}
	while ( gtk_tree_model_iter_next ( Model, &Iterator ) );
	return false;
	}

static void OnTreeSelectionChanged ( GtkTreeSelection *Selection, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;

	int Index = -1;
	GtkTreeModel *Model = NULL;
	GtkTreeIter Iterator;
	if ( gtk_tree_selection_get_selected ( Selection, &Model, &Iterator ) )
		{
		GtkTreePath *Path = gtk_tree_model_get_path ( Model, &Iterator );
		Index = 0;
		FindTreeItemIndex ( Model, NULL, Path, &Index );
		gtk_tree_path_free ( Path );
		}
	if ( Widget->Callbacks.TreeSelectionChanged )
		Widget->Callbacks.TreeSelectionChanged ( Widget, Index );
	}

static cuiGTKTreeItem *GetcuiGTKTreeItemFromIter ( GtkTreeStore *Store, GtkTreeIter *Iterator )
	{
	cuiGTKTreeItem *Item = NULL;
	gtk_tree_model_get ( GTK_TREE_MODEL ( Store ), Iterator, CUI_TREE_COL_ITEM, &Item, -1 );
	return Item;
	}

static bool GetGtkTreeIterFromTreeItem ( GtkTreeStore *Store, cuiGTKTreeItem *Item, GtkTreeIter *Iterator )
	{
	GtkTreePath *Path;
	GtkTreeModel *Model;
	if ( ( Store == NULL ) || ( Item == NULL ) || ( Item->Store != Store ) || ( Item->Ref == NULL ) )
		return false;
	Path = gtk_tree_row_reference_get_path ( Item->Ref );
	if ( Path == NULL )
		return false;
	Model = gtk_tree_row_reference_get_model ( Item->Ref );
	if ( gtk_tree_model_get_iter ( Model, Iterator, Path ) == FALSE )
		{
		gtk_tree_path_free ( Path );
		return false;
		}
	gtk_tree_path_free ( Path );
	return true;
	}

static void FreeTreeItemAtIter ( GtkTreeStore *Store, GtkTreeIter *Iterator )
	{
	cuiGTKTreeItem *Item = GetcuiGTKTreeItemFromIter ( Store, Iterator );
	if ( Item == NULL )
		return;
	if ( Item->Ref != NULL )
		gtk_tree_row_reference_free ( Item->Ref );
	free ( Item );
	}

static void FreeTreeItemsRecursive ( GtkTreeStore *Store, GtkTreeIter *Parent )
	{
	GtkTreeIter Child;
	if ( gtk_tree_model_iter_children ( GTK_TREE_MODEL ( Store ), &Child, Parent ) == FALSE )
		return;
	do
		{
		FreeTreeItemsRecursive ( Store, &Child );
		FreeTreeItemAtIter ( Store, &Child );
		}
	while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( Store ), &Child ) );
	}

cuiWidget *cuiCreateTree ( cuiWidget *ParentWidget, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *Widget = cuiInternal_CreateWidgetEntry ( cuiType_Tree, ParentWidget, NULL, X, Y, Width, Height );
	if ( Widget == NULL )
		return NULL;

	GtkTreeStore *Store = gtk_tree_store_new ( 2, G_TYPE_STRING, G_TYPE_POINTER );
	GtkWidget *View = gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( Store ) );
	GtkCellRenderer *Renderer = gtk_cell_renderer_text_new ();
	GtkTreeViewColumn *Column = gtk_tree_view_column_new_with_attributes ( "Name", Renderer, "text", CUI_TREE_COL_TEXT, NULL );
	GtkTreeSelection *Selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( View ) );
	gtk_tree_view_append_column ( GTK_TREE_VIEW ( View ), Column );
	gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW ( View ), FALSE );
	gtk_tree_selection_set_mode ( Selection, GTK_SELECTION_SINGLE );
	g_object_set_data ( G_OBJECT ( View ), "cui-store", Store );
	g_object_unref ( Store );
	g_signal_connect ( Selection, "changed", G_CALLBACK ( OnTreeSelectionChanged ), Widget );

	GtkWidget *Scroll = gtk_scrolled_window_new ( NULL, NULL );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( Scroll ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( Scroll ), GTK_SHADOW_IN );
	gtk_container_add ( GTK_CONTAINER ( Scroll ), View );

	Widget->ScrollHandle = Scroll;
	Widget->NativeHandle = View;

	FinishedCreatingNewWidget ( Widget );
	return Widget;
	}

void cuiClearTreeItems ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Tree );
	if ( ( Widget == NULL ) || ( Widget->NativeHandle == NULL ) )
		return;

	GtkTreeStore *Store = GetTreeStore ( Widget );
	GtkTreeIter Iterator;
	if ( Store == NULL )
		return;
	if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( Store ), &Iterator ) )
		{
		do
			{
			FreeTreeItemsRecursive ( Store, &Iterator );
			FreeTreeItemAtIter ( Store, &Iterator );
			}
		while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( Store ), &Iterator ) );
		}
	gtk_tree_store_clear ( Store );
	}

cuiTreeItem cuiAddItemToTree ( const cuiWidget *Widget, cuiTreeItem Parent, const char *Text )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Tree );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return cuiTreeItem_Root;

	GtkTreeStore *Store = GetTreeStore ( Widget );
	if ( Store == NULL )
		return cuiTreeItem_Root;

	GtkTreeIter Iterator;
	GtkTreeIter ParentIterator;
	GtkTreePath *Path;
	cuiGTKTreeItem *Item;
	
	GtkTreeIter *PointerToParent = NULL;
	if ( Parent != cuiTreeItem_Root )
		{
		if ( GetGtkTreeIterFromTreeItem ( Store, ( cuiGTKTreeItem * ) Parent, &ParentIterator ) == false )
			return cuiTreeItem_Root;
		PointerToParent = &ParentIterator;
		}

	Item = ( cuiGTKTreeItem * ) calloc ( 1, sizeof ( cuiGTKTreeItem ) );
	if ( Item == NULL )
		return cuiTreeItem_Root;
	Item->Store = Store;
	gtk_tree_store_append ( Store, &Iterator, PointerToParent );
	gtk_tree_store_set ( Store, &Iterator, CUI_TREE_COL_TEXT, Text, CUI_TREE_COL_ITEM, Item, -1 );
	Path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( Store ), &Iterator );
	Item->Ref = gtk_tree_row_reference_new ( GTK_TREE_MODEL ( Store ), Path );
	gtk_tree_path_free ( Path );
	return ( cuiTreeItem ) Item;
	}

void cuiRemoveItemFromTree ( cuiWidget *Widget, cuiTreeItem Item )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Tree );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;

	GtkTreeStore *Store = GetTreeStore ( Widget );
	GtkTreeIter Iterator;
	if ( ( Store == NULL ) || ( GetGtkTreeIterFromTreeItem ( Store, ( cuiGTKTreeItem * ) Item, &Iterator ) == false ) )
		return;
	FreeTreeItemsRecursive ( Store, &Iterator );
	FreeTreeItemAtIter ( Store, &Iterator );
	gtk_tree_store_remove ( Store, &Iterator );
	}

void cuiSetTreeItemText ( cuiWidget *Widget, cuiTreeItem Item, const char *Text )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Tree );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;

	GtkTreeStore *Store = GetTreeStore ( Widget );
	GtkTreeIter Iterator;
	if ( ( Store == NULL ) || ( GetGtkTreeIterFromTreeItem ( Store, ( cuiGTKTreeItem * ) Item, &Iterator ) == false ) )
		return;
	gtk_tree_store_set ( Store, &Iterator, CUI_TREE_COL_TEXT, EMPTY_STRING_IF_NULL ( Text ), -1 );
	}

void cuiGetTreeItemText ( const cuiWidget *Widget, const int Index, char *Buffer, const unsigned BufferSize )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Tree );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;

	GtkTreeStore *Store;
	GtkTreeIter Iterator;
	int CurrentIndex = 0;
	char *Text = NULL;
	if ( ( Buffer == NULL ) || ( BufferSize == 0 ) )
		return;
	Buffer[0] = 0;
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Tree );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) || ( Index < 0 ) )
		return;
	Store = GetTreeStore ( Widget );
	if ( ( Store == NULL ) || ( GetTreeItemAtIndex ( GTK_TREE_MODEL ( Store ), NULL, Index, &CurrentIndex, &Iterator ) == false ) )
		return;
	gtk_tree_model_get ( GTK_TREE_MODEL ( Store ), &Iterator, CUI_TREE_COL_TEXT, &Text, -1 );
	if ( Text != NULL )
		{
		strncpy ( Buffer, Text, BufferSize - 1 );
		Buffer[BufferSize - 1] = 0;
		g_free ( Text );
		}
	}

cuiTreeItem cuiGetSelectedTreeItem ( cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Tree );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return cuiTreeItem_Root;

	GtkWidget *View = GTK_WIDGET ( Widget->NativeHandle );
	GtkTreeSelection *Selection;
	GtkTreeModel *Model = NULL;
	GtkTreeIter Iterator;
	cuiGTKTreeItem *Item = NULL;
	if ( View == NULL )
		return cuiTreeItem_Root;
	Selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( View ) );
	if ( gtk_tree_selection_get_selected ( Selection, &Model, &Iterator ) == FALSE )
		return cuiTreeItem_Root;
	gtk_tree_model_get ( Model, &Iterator, CUI_TREE_COL_ITEM, &Item, -1 );
	return ( cuiTreeItem ) Item;
	}

void cuiSetSelectedTreeItem ( cuiWidget *Widget, cuiTreeItem Item )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Tree );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;

	GtkWidget *View = GTK_WIDGET ( Widget->NativeHandle );
	if ( View == NULL )
		return;

	// Deselect everything
	GtkTreeSelection *Selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( View ) );
	gtk_tree_selection_unselect_all ( Selection );

	// Find this item, and select it
	GtkTreeIter Iterator;
	if ( GetGtkTreeIterFromTreeItem ( GetTreeStore ( Widget ), ( cuiGTKTreeItem * ) Item, &Iterator ) )
		gtk_tree_selection_select_iter ( Selection, &Iterator );
	}

void cuiExpandTree ( cuiWidget *Widget, cuiTreeItem Item, bool Expand )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Tree );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;

	GtkWidget *View = GTK_WIDGET ( Widget->NativeHandle );
	GtkTreeIter Iterator;
	GtkTreePath *Path;
	if ( ( View == NULL ) || ( GetGtkTreeIterFromTreeItem ( GetTreeStore ( Widget ), ( cuiGTKTreeItem * ) Item, &Iterator ) == false ) )
		return;
	Path = gtk_tree_model_get_path ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( View ) ), &Iterator );
	if ( Expand )
		gtk_tree_view_expand_row ( GTK_TREE_VIEW ( View ), Path, FALSE );
	else
		gtk_tree_view_collapse_row ( GTK_TREE_VIEW ( View ), Path );
	gtk_tree_path_free ( Path );
	}

void cuiSetTreeItemData ( cuiWidget *Widget, cuiTreeItem Item, void *Data )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Tree );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;

	if ( ( Item != NULL ) && ( ( ( cuiGTKTreeItem * ) Item )->Store == GetTreeStore ( Widget ) ) )
		( ( cuiGTKTreeItem * ) Item )->UserData = Data;
	}

void *cuiGetTreeItemData ( cuiWidget *Widget, cuiTreeItem Item )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Tree );
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return NULL;
	if ( ( Item == NULL ) || ( ( ( cuiGTKTreeItem * ) Item )->Store != GetTreeStore ( Widget ) ) )
		return NULL;
	return ( ( cuiGTKTreeItem * ) Item )->UserData;
	}
