#define _POSIX_C_SOURCE 200809L
#include "CrossUI_Internal.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define CUI_TREE_COL_TEXT 0
#define CUI_TREE_COL_ITEM 1

typedef struct
	{
	GtkTreeRowReference *Ref;
	GtkTreeStore *Store;
	void *UserData;
	} cuiGTKTreeItem;

static void OnClicked ( GtkButton *Button, gpointer Data )
	{
	( void ) Button;
	cuiInternal_FireClick ( ( cuiWidget * ) Data );
	}

static void OnToggled ( GtkToggleButton *Button, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	Widget->Checked = gtk_toggle_button_get_active ( Button ) ? true : false;
	cuiInternal_FireChange ( Widget );
	}

static void OnEntryChanged ( GtkEditable *Editable, gpointer Data )
	{
	( void ) Editable;
	cuiInternal_FireChange ( ( cuiWidget * ) Data );
	}

static void OnBufferChanged ( GtkTextBuffer *Buffer, gpointer Data )
	{
	( void ) Buffer;
	cuiInternal_FireChange ( ( cuiWidget * ) Data );
	}

static void OnComboChanged ( GtkComboBox *Combo, gpointer Data )
	{
	( void ) Combo;
	cuiInternal_FireChange ( ( cuiWidget * ) Data );
	}

static void OnListSelectionChanged ( GtkTreeSelection *Selection, gpointer Data )
	{
	( void ) Selection;
	cuiInternal_FireChange ( ( cuiWidget * ) Data );
	}

static void OnValueChanged ( GtkRange *Range, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	Widget->SliderValue = ( int ) gtk_range_get_value ( Range );
	cuiInternal_FireChange ( Widget );
	}

static gboolean OnDelete ( GtkWidget *Native, GdkEvent *Event, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	( void ) Native;
	( void ) Event;
	cuiInternal_FireClose ( Widget );
	cuiInternal_DestroyWidget ( Widget );
	return TRUE;
	}

static GtkWidget *GetContainer ( cuiWidget *Widget )
	{
	cuiWidget *Parent = Widget->Parent;
	while ( Parent != NULL )
		{
		if ( Parent->NativeInner != NULL )
			return GTK_WIDGET ( Parent->NativeInner );
		Parent = Parent->Parent;
		}
	return NULL;
	}

static GtkWidget *InteractiveWidget ( cuiWidget *Widget )
	{
	if ( Widget->NativeInner != NULL )
		return GTK_WIDGET ( Widget->NativeInner );
	return GTK_WIDGET ( Widget->NativeHandle );
	}

static void PlaceWidget ( cuiWidget *Widget, GtkWidget *Native )
	{
	GtkWidget *Container = GetContainer ( Widget );
	gtk_widget_set_size_request ( Native, ( gint ) Widget->Width, ( gint ) Widget->Height );
	if ( Container != NULL )
		gtk_fixed_put ( GTK_FIXED ( Container ), Native, Widget->X, Widget->Y );
	if ( Widget->Visible )
		gtk_widget_show_all ( Native );
	else
		gtk_widget_hide ( Native );
	gtk_widget_set_sensitive ( Native, Widget->Enabled ? TRUE : FALSE );
	}

static GtkWidget *MakeScrolled ( GtkWidget *Child, cuiWidget *Widget )
	{
	GtkWidget *Scroll = gtk_scrolled_window_new ( NULL, NULL );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( Scroll ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( Scroll ), GTK_SHADOW_IN );
	gtk_container_add ( GTK_CONTAINER ( Scroll ), Child );
	PlaceWidget ( Widget, Scroll );
	Widget->NativeHandle = Scroll;
	Widget->NativeInner = Child;
	return Scroll;
	}

static GtkListStore *GetListStore ( cuiWidget *Widget )
	{
	GtkWidget *View = InteractiveWidget ( Widget );
	return GTK_LIST_STORE ( g_object_get_data ( G_OBJECT ( View ), "cui-store" ) );
	}

static GtkTreeStore *GetTreeStore ( cuiWidget *Widget )
	{
	GtkWidget *View = InteractiveWidget ( Widget );
	return GTK_TREE_STORE ( g_object_get_data ( G_OBJECT ( View ), "cui-store" ) );
	}

static cuiGTKTreeItem *TreeItemFromIter ( GtkTreeStore *Store, GtkTreeIter *Iter )
	{
	cuiGTKTreeItem *Item = NULL;
	gtk_tree_model_get ( GTK_TREE_MODEL ( Store ), Iter, CUI_TREE_COL_ITEM, &Item, -1 );
	return Item;
	}

static bool IterFromTreeItem ( cuiGTKTreeItem *Item, GtkTreeIter *Iter )
	{
	GtkTreePath *Path;
	GtkTreeModel *Model;
	if ( ( Item == NULL ) || ( Item->Ref == NULL ) )
		return false;
	Path = gtk_tree_row_reference_get_path ( Item->Ref );
	if ( Path == NULL )
		return false;
	Model = gtk_tree_row_reference_get_model ( Item->Ref );
	if ( gtk_tree_model_get_iter ( Model, Iter, Path ) == FALSE )
		{
		gtk_tree_path_free ( Path );
		return false;
		}
	gtk_tree_path_free ( Path );
	return true;
	}

static void FreeTreeItemAtIter ( GtkTreeStore *Store, GtkTreeIter *Iter )
	{
	cuiGTKTreeItem *Item = TreeItemFromIter ( Store, Iter );
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

bool cuiBackend_Initialize ( void )
	{
	int Argc = 1;
	char Arg0[] = "CrossUI";
	char *Argv[] = { Arg0, NULL };
	char **ArgvPtr = Argv;
	gtk_init ( &Argc, &ArgvPtr );
	return true;
	}

void cuiBackend_Shutdown ( void )
	{
	}

const char *cuiBackend_Name ( void )
	{
	return "GTK";
	}

bool cuiBackend_Realize ( cuiWidget *Widget )
	{
	GtkWidget *Native = NULL;

	switch ( Widget->Type )
		{
		case cuiType_Window:
			{
			GtkWidget *Fixed = gtk_fixed_new ();
			Native = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
			gtk_window_set_title ( GTK_WINDOW ( Native ), Widget->Text );
			gtk_window_set_default_size ( GTK_WINDOW ( Native ), ( gint ) Widget->Width, ( gint ) Widget->Height );
			if ( ( Widget->X >= 0 ) && ( Widget->Y >= 0 ) )
				gtk_window_move ( GTK_WINDOW ( Native ), Widget->X, Widget->Y );
			else
				gtk_window_set_position ( GTK_WINDOW ( Native ), GTK_WIN_POS_CENTER );
			gtk_container_add ( GTK_CONTAINER ( Native ), Fixed );
			g_signal_connect ( Native, "delete-event", G_CALLBACK ( OnDelete ), Widget );
			gtk_widget_show_all ( Native );
			Widget->NativeHandle = Native;
			Widget->NativeInner = Fixed;
			return true;
			}
		case cuiType_Label:
			Native = gtk_label_new ( Widget->Text );
			gtk_label_set_xalign ( GTK_LABEL ( Native ), 0.0f );
			gtk_label_set_yalign ( GTK_LABEL ( Native ), 0.5f );
			break;
		case cuiType_Button:
			Native = gtk_button_new_with_label ( Widget->Text );
			g_signal_connect ( Native, "clicked", G_CALLBACK ( OnClicked ), Widget );
			break;
		case cuiType_CheckBox:
			Native = gtk_check_button_new_with_label ( Widget->Text );
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( Native ), Widget->Checked ? TRUE : FALSE );
			g_signal_connect ( Native, "toggled", G_CALLBACK ( OnToggled ), Widget );
			break;
		case cuiType_TextBox:
			Native = gtk_entry_new ();
			gtk_entry_set_text ( GTK_ENTRY ( Native ), Widget->Text );
			g_signal_connect ( Native, "changed", G_CALLBACK ( OnEntryChanged ), Widget );
			g_signal_connect ( Native, "activate", G_CALLBACK ( OnClicked ), Widget );
			break;
		case cuiType_TextArea:
			{
			GtkWidget *View = gtk_text_view_new ();
			GtkTextBuffer *Buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( View ) );
			gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW ( View ), GTK_WRAP_WORD_CHAR );
			gtk_text_buffer_set_text ( Buffer, Widget->Text, -1 );
			g_signal_connect ( Buffer, "changed", G_CALLBACK ( OnBufferChanged ), Widget );
			MakeScrolled ( View, Widget );
			return true;
			}
		case cuiType_ComboBox:
			Native = gtk_combo_box_text_new ();
			g_signal_connect ( Native, "changed", G_CALLBACK ( OnComboChanged ), Widget );
			break;
		case cuiType_ListBox:
			{
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
			MakeScrolled ( View, Widget );
			return true;
			}
		case cuiType_Tree:
			{
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
			g_signal_connect ( Selection, "changed", G_CALLBACK ( OnListSelectionChanged ), Widget );
			MakeScrolled ( View, Widget );
			return true;
			}
		case cuiType_Slider:
			Native = gtk_scale_new_with_range ( GTK_ORIENTATION_HORIZONTAL, Widget->SliderMin, Widget->SliderMax, 1 );
			gtk_range_set_value ( GTK_RANGE ( Native ), Widget->SliderValue );
			gtk_scale_set_draw_value ( GTK_SCALE ( Native ), TRUE );
			g_signal_connect ( Native, "value-changed", G_CALLBACK ( OnValueChanged ), Widget );
			break;
		case cuiType_ProgressBar:
			Native = gtk_progress_bar_new ();
			gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR ( Native ), Widget->ProgressValue / 100.0 );
			gtk_progress_bar_set_show_text ( GTK_PROGRESS_BAR ( Native ), TRUE );
			break;
		case cuiType_GroupBox:
			{
			GtkWidget *Fixed = gtk_fixed_new ();
			Native = gtk_frame_new ( Widget->Text );
			gtk_container_add ( GTK_CONTAINER ( Native ), Fixed );
			PlaceWidget ( Widget, Native );
			Widget->NativeHandle = Native;
			Widget->NativeInner = Fixed;
			return true;
			}
		case cuiType_Panel:
			Native = gtk_fixed_new ();
			PlaceWidget ( Widget, Native );
			Widget->NativeHandle = Native;
			Widget->NativeInner = Native;
			return true;
		default:
			return false;
		}

	PlaceWidget ( Widget, Native );
	Widget->NativeHandle = Native;
	Widget->NativeInner = Native;
	return true;
	}

void cuiBackend_DestroyNative ( cuiWidget *Widget )
	{
	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	if ( Native == NULL )
		return;
	if ( Widget->Type == cuiType_Tree )
		cuiBackend_TreeClear ( Widget );
	Widget->NativeHandle = NULL;
	Widget->NativeInner = NULL;
	gtk_widget_destroy ( Native );
	}

void cuiBackend_SetText ( cuiWidget *Widget, const char *Text )
	{
	GtkWidget *Native = InteractiveWidget ( Widget );
	if ( Native == NULL )
		return;
	switch ( Widget->Type )
		{
		case cuiType_Window:
			gtk_window_set_title ( GTK_WINDOW ( Widget->NativeHandle ), Text );
			break;
		case cuiType_Label:
			gtk_label_set_text ( GTK_LABEL ( Native ), Text );
			break;
		case cuiType_Button:
			gtk_button_set_label ( GTK_BUTTON ( Native ), Text );
			break;
		case cuiType_CheckBox:
			gtk_button_set_label ( GTK_BUTTON ( Native ), Text );
			break;
		case cuiType_TextBox:
			gtk_entry_set_text ( GTK_ENTRY ( Native ), Text );
			break;
		case cuiType_TextArea:
			{
			GtkTextBuffer *Buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( Native ) );
			gtk_text_buffer_set_text ( Buffer, Text, -1 );
			break;
			}
		case cuiType_GroupBox:
			gtk_frame_set_label ( GTK_FRAME ( Widget->NativeHandle ), Text );
			break;
		default:
			break;
		}
	}

void cuiBackend_GetText ( cuiWidget *Widget, char *Buffer, size_t BufferSize )
	{
	GtkWidget *Native = InteractiveWidget ( Widget );
	const char *Text = "";
	if ( Native == NULL )
		{
		cuiInternal_CopyToBuffer ( Buffer, BufferSize, Widget->Text );
		return;
		}
	switch ( Widget->Type )
		{
		case cuiType_Window:
			Text = gtk_window_get_title ( GTK_WINDOW ( Widget->NativeHandle ) );
			break;
		case cuiType_Label:
			Text = gtk_label_get_text ( GTK_LABEL ( Native ) );
			break;
		case cuiType_Button:
		case cuiType_CheckBox:
			Text = gtk_button_get_label ( GTK_BUTTON ( Native ) );
			break;
		case cuiType_TextBox:
			Text = gtk_entry_get_text ( GTK_ENTRY ( Native ) );
			break;
		case cuiType_TextArea:
			{
			GtkTextBuffer *TextBuffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( Native ) );
			GtkTextIter Start, End;
			char *Allocated;
			gtk_text_buffer_get_bounds ( TextBuffer, &Start, &End );
			Allocated = gtk_text_buffer_get_text ( TextBuffer, &Start, &End, FALSE );
			cuiInternal_CopyToBuffer ( Buffer, BufferSize, Allocated );
			g_free ( Allocated );
			return;
			}
		case cuiType_GroupBox:
			Text = gtk_frame_get_label ( GTK_FRAME ( Widget->NativeHandle ) );
			break;
		default:
			Text = Widget->Text;
			break;
		}
	cuiInternal_CopyToBuffer ( Buffer, BufferSize, Text );
	}

void cuiBackend_SetBounds ( cuiWidget *Widget )
	{
	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	GtkWidget *Container;
	if ( Native == NULL )
		return;
	gtk_widget_set_size_request ( Native, ( gint ) Widget->Width, ( gint ) Widget->Height );
	if ( Widget->Type == cuiType_Window )
		{
		gtk_window_resize ( GTK_WINDOW ( Native ), ( gint ) Widget->Width, ( gint ) Widget->Height );
		if ( ( Widget->X >= 0 ) && ( Widget->Y >= 0 ) )
			gtk_window_move ( GTK_WINDOW ( Native ), Widget->X, Widget->Y );
		return;
		}
	Container = GetContainer ( Widget );
	if ( Container != NULL )
		gtk_fixed_move ( GTK_FIXED ( Container ), Native, Widget->X, Widget->Y );
	}

void cuiBackend_SetVisible ( cuiWidget *Widget )
	{
	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	if ( Native == NULL )
		return;
	if ( Widget->Visible )
		gtk_widget_show ( Native );
	else
		gtk_widget_hide ( Native );
	}

void cuiBackend_SetEnabled ( cuiWidget *Widget )
	{
	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	if ( Native != NULL )
		gtk_widget_set_sensitive ( Native, Widget->Enabled ? TRUE : FALSE );
	}

void cuiBackend_SetFocus ( cuiWidget *Widget )
	{
	GtkWidget *Native = InteractiveWidget ( Widget );
	if ( Native != NULL )
		gtk_widget_grab_focus ( Native );
	}

void cuiBackend_SetChecked ( cuiWidget *Widget )
	{
	GtkWidget *Native = InteractiveWidget ( Widget );
	if ( ( Native != NULL ) && ( Widget->Type == cuiType_CheckBox ) )
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( Native ), Widget->Checked ? TRUE : FALSE );
	}

bool cuiBackend_GetChecked ( cuiWidget *Widget )
	{
	GtkWidget *Native = InteractiveWidget ( Widget );
	if ( ( Native != NULL ) && ( Widget->Type == cuiType_CheckBox ) )
		return gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( Native ) ) ? true : false;
	return Widget->Checked;
	}

void cuiBackend_ClearItems ( cuiWidget *Widget )
	{
	GtkWidget *Native = InteractiveWidget ( Widget );
	if ( Native == NULL )
		return;
	if ( Widget->Type == cuiType_ComboBox )
		gtk_combo_box_text_remove_all ( GTK_COMBO_BOX_TEXT ( Native ) );
	else if ( Widget->Type == cuiType_ListBox )
		gtk_list_store_clear ( GetListStore ( Widget ) );
	}

int cuiBackend_AddItem ( cuiWidget *Widget, const char *Text )
	{
	GtkWidget *Native = InteractiveWidget ( Widget );
	if ( Native == NULL )
		return -1;
	if ( Widget->Type == cuiType_ComboBox )
		{
		gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT ( Native ), Text );
		return ( int ) Widget->ItemCount - 1;
		}
	if ( Widget->Type == cuiType_ListBox )
		{
		GtkListStore *Store = GetListStore ( Widget );
		GtkTreeIter Iter;
		gtk_list_store_append ( Store, &Iter );
		gtk_list_store_set ( Store, &Iter, 0, Text, -1 );
		return ( int ) Widget->ItemCount - 1;
		}
	return -1;
	}

void cuiBackend_SetSelected ( cuiWidget *Widget, int Index )
	{
	GtkWidget *Native = InteractiveWidget ( Widget );
	if ( Native == NULL )
		return;
	if ( Widget->Type == cuiType_ComboBox )
		gtk_combo_box_set_active ( GTK_COMBO_BOX ( Native ), Index );
	else if ( Widget->Type == cuiType_ListBox )
		{
		GtkTreePath *Path = gtk_tree_path_new_from_indices ( Index, -1 );
		GtkTreeSelection *Selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( Native ) );
		gtk_tree_selection_unselect_all ( Selection );
		if ( Index >= 0 )
			gtk_tree_selection_select_path ( Selection, Path );
		gtk_tree_path_free ( Path );
		}
	}

int cuiBackend_GetSelected ( cuiWidget *Widget )
	{
	GtkWidget *Native = InteractiveWidget ( Widget );
	if ( Native == NULL )
		return -1;
	if ( Widget->Type == cuiType_ComboBox )
		return gtk_combo_box_get_active ( GTK_COMBO_BOX ( Native ) );
	if ( Widget->Type == cuiType_ListBox )
		{
		GtkTreeSelection *Selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( Native ) );
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
	return -1;
	}

cuiTreeItem cuiBackend_TreeAddItem ( cuiWidget *Widget, cuiTreeItem Parent, const char *Text )
	{
	GtkTreeStore *Store = GetTreeStore ( Widget );
	GtkTreeIter Iter;
	GtkTreeIter ParentIter;
	GtkTreeIter *ParentPtr = NULL;
	GtkTreePath *Path;
	cuiGTKTreeItem *Item;

	if ( Store == NULL )
		return cuiTreeItem_Root;
	if ( Parent != cuiTreeItem_Root )
		{
		if ( IterFromTreeItem ( ( cuiGTKTreeItem * ) Parent, &ParentIter ) == false )
			return cuiTreeItem_Root;
		ParentPtr = &ParentIter;
		}

	Item = ( cuiGTKTreeItem * ) calloc ( 1, sizeof ( cuiGTKTreeItem ) );
	if ( Item == NULL )
		return cuiTreeItem_Root;
	Item->Store = Store;
	gtk_tree_store_append ( Store, &Iter, ParentPtr );
	gtk_tree_store_set ( Store, &Iter, CUI_TREE_COL_TEXT, Text, CUI_TREE_COL_ITEM, Item, -1 );
	Path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( Store ), &Iter );
	Item->Ref = gtk_tree_row_reference_new ( GTK_TREE_MODEL ( Store ), Path );
	gtk_tree_path_free ( Path );
	return ( cuiTreeItem ) Item;
	}

void cuiBackend_TreeSetItemText ( cuiWidget *Widget, cuiTreeItem Item, const char *Text )
	{
	GtkTreeIter Iter;
	( void ) Widget;
	if ( IterFromTreeItem ( ( cuiGTKTreeItem * ) Item, &Iter ) == false )
		return;
	gtk_tree_store_set ( ( ( cuiGTKTreeItem * ) Item )->Store, &Iter, CUI_TREE_COL_TEXT, Text, -1 );
	}

void cuiBackend_TreeGetItemText ( cuiWidget *Widget, cuiTreeItem Item, char *Buffer, size_t BufferSize )
	{
	GtkTreeIter Iter;
	gchar *Text = NULL;
	( void ) Widget;
	if ( IterFromTreeItem ( ( cuiGTKTreeItem * ) Item, &Iter ) == false )
		{
		cuiInternal_CopyToBuffer ( Buffer, BufferSize, "" );
		return;
		}
	gtk_tree_model_get ( GTK_TREE_MODEL ( ( ( cuiGTKTreeItem * ) Item )->Store ), &Iter, CUI_TREE_COL_TEXT, &Text, -1 );
	cuiInternal_CopyToBuffer ( Buffer, BufferSize, Text );
	g_free ( Text );
	}

void cuiBackend_TreeRemoveItem ( cuiWidget *Widget, cuiTreeItem Item )
	{
	GtkTreeStore *Store = GetTreeStore ( Widget );
	GtkTreeIter Iter;
	if ( ( Store == NULL ) || ( IterFromTreeItem ( ( cuiGTKTreeItem * ) Item, &Iter ) == false ) )
		return;
	FreeTreeItemsRecursive ( Store, &Iter );
	FreeTreeItemAtIter ( Store, &Iter );
	gtk_tree_store_remove ( Store, &Iter );
	}

void cuiBackend_TreeClear ( cuiWidget *Widget )
	{
	GtkTreeStore *Store = GetTreeStore ( Widget );
	GtkTreeIter Iter;
	if ( Store == NULL )
		return;
	if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( Store ), &Iter ) )
		{
		do
			{
			FreeTreeItemsRecursive ( Store, &Iter );
			FreeTreeItemAtIter ( Store, &Iter );
			}
		while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( Store ), &Iter ) );
		}
	gtk_tree_store_clear ( Store );
	}

cuiTreeItem cuiBackend_TreeGetSelected ( cuiWidget *Widget )
	{
	GtkWidget *View = InteractiveWidget ( Widget );
	GtkTreeSelection *Selection;
	GtkTreeModel *Model = NULL;
	GtkTreeIter Iter;
	cuiGTKTreeItem *Item = NULL;
	if ( View == NULL )
		return cuiTreeItem_Root;
	Selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( View ) );
	if ( gtk_tree_selection_get_selected ( Selection, &Model, &Iter ) == FALSE )
		return cuiTreeItem_Root;
	gtk_tree_model_get ( Model, &Iter, CUI_TREE_COL_ITEM, &Item, -1 );
	return ( cuiTreeItem ) Item;
	}

void cuiBackend_TreeSetSelected ( cuiWidget *Widget, cuiTreeItem Item )
	{
	GtkWidget *View = InteractiveWidget ( Widget );
	GtkTreeIter Iter;
	GtkTreeSelection *Selection;
	if ( View == NULL )
		return;
	Selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( View ) );
	gtk_tree_selection_unselect_all ( Selection );
	if ( IterFromTreeItem ( ( cuiGTKTreeItem * ) Item, &Iter ) )
		gtk_tree_selection_select_iter ( Selection, &Iter );
	}

void cuiBackend_TreeExpand ( cuiWidget *Widget, cuiTreeItem Item, bool Expand )
	{
	GtkWidget *View = InteractiveWidget ( Widget );
	GtkTreeIter Iter;
	GtkTreePath *Path;
	if ( ( View == NULL ) || ( IterFromTreeItem ( ( cuiGTKTreeItem * ) Item, &Iter ) == false ) )
		return;
	Path = gtk_tree_model_get_path ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( View ) ), &Iter );
	if ( Expand )
		gtk_tree_view_expand_row ( GTK_TREE_VIEW ( View ), Path, FALSE );
	else
		gtk_tree_view_collapse_row ( GTK_TREE_VIEW ( View ), Path );
	gtk_tree_path_free ( Path );
	}

void cuiBackend_TreeSetItemData ( cuiWidget *Widget, cuiTreeItem Item, void *Data )
	{
	( void ) Widget;
	if ( Item != NULL )
		( ( cuiGTKTreeItem * ) Item )->UserData = Data;
	}

void *cuiBackend_TreeGetItemData ( cuiWidget *Widget, cuiTreeItem Item )
	{
	( void ) Widget;
	return ( Item != NULL ) ? ( ( cuiGTKTreeItem * ) Item )->UserData : NULL;
	}

void cuiBackend_SliderSetRange ( cuiWidget *Widget )
	{
	GtkWidget *Native = InteractiveWidget ( Widget );
	if ( Native != NULL )
		gtk_range_set_range ( GTK_RANGE ( Native ), Widget->SliderMin, Widget->SliderMax );
	}

void cuiBackend_SliderSetValue ( cuiWidget *Widget )
	{
	GtkWidget *Native = InteractiveWidget ( Widget );
	if ( Native != NULL )
		gtk_range_set_value ( GTK_RANGE ( Native ), Widget->SliderValue );
	}

int cuiBackend_SliderGetValue ( cuiWidget *Widget )
	{
	GtkWidget *Native = InteractiveWidget ( Widget );
	if ( Native != NULL )
		return ( int ) gtk_range_get_value ( GTK_RANGE ( Native ) );
	return Widget->SliderValue;
	}

void cuiBackend_ProgressSetValue ( cuiWidget *Widget )
	{
	GtkWidget *Native = InteractiveWidget ( Widget );
	char Label[16];
	if ( Native == NULL )
		return;
	gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR ( Native ), Widget->ProgressValue / 100.0 );
	snprintf ( Label, sizeof ( Label ), "%d%%", Widget->ProgressValue );
	gtk_progress_bar_set_text ( GTK_PROGRESS_BAR ( Native ), Label );
	}

bool cuiBackend_Update ( bool Wait )
	{
	if ( cuiInternal_CountWindows () == 0 )
		return false;
	gtk_main_iteration_do ( Wait ? TRUE : FALSE );
	if ( Wait == false )
		{
		while ( gtk_events_pending () )
			gtk_main_iteration_do ( FALSE );
		}
	return cuiInternal_CountWindows () > 0;
	}

cuiResult cuiBackend_MessageBox ( cuiWidget *Parent, const char *Title, const char *Message, cuiMessageType Type )
	{
	GtkWindow *ParentWindow = NULL;
	GtkMessageType GtkType = GTK_MESSAGE_INFO;
	GtkButtonsType Buttons = GTK_BUTTONS_OK;
	GtkWidget *Dialog;
	gint Response;

	if ( ( Parent != NULL ) && ( Parent->Type == cuiType_Window ) )
		ParentWindow = GTK_WINDOW ( Parent->NativeHandle );

	switch ( Type )
		{
		case cuiMessage_Warning:
			GtkType = GTK_MESSAGE_WARNING;
			break;
		case cuiMessage_Error:
			GtkType = GTK_MESSAGE_ERROR;
			break;
		case cuiMessage_Question:
			GtkType = GTK_MESSAGE_QUESTION;
			Buttons = GTK_BUTTONS_YES_NO;
			break;
		default:
			break;
		}

	Dialog = gtk_message_dialog_new ( ParentWindow, GTK_DIALOG_MODAL, GtkType, Buttons, "%s", ( Message != NULL ) ? Message : "" );
	if ( Title != NULL )
		gtk_window_set_title ( GTK_WINDOW ( Dialog ), Title );
	Response = gtk_dialog_run ( GTK_DIALOG ( Dialog ) );
	gtk_widget_destroy ( Dialog );

	if ( Response == GTK_RESPONSE_YES )
		return cuiResult_Yes;
	if ( Response == GTK_RESPONSE_NO )
		return cuiResult_No;
	if ( Response == GTK_RESPONSE_OK )
		return cuiResult_OK;
	return cuiResult_Cancel;
	}

static void ApplyFileFilters ( GtkFileChooser *Chooser, const char *Filter )
	{
	char *Copy;
	char *Token;
	char *Save = NULL;
	char *Name = NULL;
	if ( ( Filter == NULL ) || ( Filter[0] == 0 ) )
		return;
	Copy = cuiInternal_StrDup ( Filter );
	if ( Copy == NULL )
		return;
	for ( Token = strtok_r ( Copy, "|", &Save ); Token != NULL; Token = strtok_r ( NULL, "|", &Save ) )
		{
		if ( Name == NULL )
			Name = Token;
		else
			{
			GtkFileFilter *GtkFilter = gtk_file_filter_new ();
			char *Pattern = Token;
			char *PatternSave = NULL;
			char *OnePattern;
			gtk_file_filter_set_name ( GtkFilter, Name );
			for ( OnePattern = strtok_r ( Pattern, ";", &PatternSave ); OnePattern != NULL; OnePattern = strtok_r ( NULL, ";", &PatternSave ) )
				gtk_file_filter_add_pattern ( GtkFilter, OnePattern );
			gtk_file_chooser_add_filter ( Chooser, GtkFilter );
			Name = NULL;
			}
		}
	free ( Copy );
	}

char *cuiBackend_FileDialog ( cuiWidget *Parent, const char *Title, const char *Filter, bool Save, bool Folder )
	{
	GtkWindow *ParentWindow = NULL;
	GtkFileChooserAction Action = GTK_FILE_CHOOSER_ACTION_OPEN;
	GtkWidget *Dialog;
	char *Result = NULL;
	const char *Accept = "_Open";

	if ( ( Parent != NULL ) && ( Parent->Type == cuiType_Window ) )
		ParentWindow = GTK_WINDOW ( Parent->NativeHandle );
	if ( Folder )
		{
		Action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
		Accept = "_Select";
		}
	else if ( Save )
		{
		Action = GTK_FILE_CHOOSER_ACTION_SAVE;
		Accept = "_Save";
		}

	Dialog = gtk_file_chooser_dialog_new ( ( Title != NULL ) ? Title : "", ParentWindow, Action,
	                                       "_Cancel", GTK_RESPONSE_CANCEL,
	                                       Accept, GTK_RESPONSE_ACCEPT,
	                                       NULL );
	if ( Save )
		gtk_file_chooser_set_do_overwrite_confirmation ( GTK_FILE_CHOOSER ( Dialog ), TRUE );
	ApplyFileFilters ( GTK_FILE_CHOOSER ( Dialog ), Filter );
	if ( gtk_dialog_run ( GTK_DIALOG ( Dialog ) ) == GTK_RESPONSE_ACCEPT )
		{
		char *Filename = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( Dialog ) );
		Result = cuiInternal_StrDup ( Filename );
		g_free ( Filename );
		}
	gtk_widget_destroy ( Dialog );
	return Result;
	}
