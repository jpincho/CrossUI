#include <CrossUI.h>
#include <stdio.h>

cuiWidget *ProgressBarHandle;
cuiWidget *ListBoxHandle;
int NextListItem;

void OnWindowResized ( const cuiWidget *Widget, const unsigned Width, const unsigned Height )
	{
	printf ( "Window %p resized to %ux%u\n", Widget, Width, Height );
	}

void OnWindowMoved ( const cuiWidget *Widget, const int X, int Y )
	{
	printf ( "Window %p moved to %dx%d\n", Widget, X, Y );
	}

void OnWindowDestroyed ( const cuiWidget *Widget )
	{
	printf ( "Window %p closed\n", Widget );
	}

void OnGreet ( const cuiWidget *Widget )
	{
	printf ( "Greet pressed\n" );
	}

void OnShutdown ( const cuiWidget *Widget )
	{
	printf ( "Forcing shutdown\n" );
	cuiShutdown();
	}

void OnCheckChanged ( const cuiWidget *Widget, const bool State )
	{
	printf ( "Checkbox changed to %s\n", State ? "true" : "false" );
	}

void OnSliderChanged ( const cuiWidget *Widget, const float Value )
	{
	printf ( "Slider changed - %f\n", Value );
	if ( ProgressBarHandle != NULL )
		cuiSetProgressBarValue ( ProgressBarHandle, Value );
	}

void OnComboSelectionChanged ( const cuiWidget *Widget, const int NewIndex )
	{
	char Text[256];
	cuiGetComboBoxItemText ( Widget, NewIndex, Text, sizeof ( Text ) );
	printf ( "Combo changed - %d - '%s'\n", NewIndex, Text );
	}

void OnTextBoxContentChanged ( const cuiWidget *Widget )
	{
	char Text[256];
	cuiGetWidgetText ( Widget, Text, sizeof ( Text ) );
	printf ( "TextBox changed - '%s'\n", Text );
	}

void OnTextAreaContentChanged ( const cuiWidget *Widget )
	{
	char Text[256];
	cuiGetWidgetText ( Widget, Text, sizeof ( Text ) );
	printf ( "TextArea changed - '%s'\n", Text );
	}

void OnListBoxSelectionChanged ( const cuiWidget *Widget, const int NewIndex )
	{
	char Text[256];
	cuiGetListBoxItemText ( Widget, NewIndex, Text, sizeof ( Text ) );
	printf ( "ListBox changed - %d - '%s'\n", NewIndex, Text );
	}

void OnTreeSelectionChanged ( const cuiWidget *Widget, const int NewIndex )
	{
	char Text[256];
	cuiGetTreeItemText ( Widget, NewIndex, Text, sizeof ( Text ) );
	printf ( "Tree changed - %d - '%s'\n", NewIndex, Text );
	}

void OnAddListItem ( const cuiWidget *Widget )
	{
	char Text[32];
	( void ) Widget;
	snprintf ( Text, sizeof ( Text ), "Added %d", NextListItem++ );
	printf ( "Added list item at index %d\n", cuiAddItemToListBox ( ListBoxHandle, Text ) );
	}

void OnRemoveListItem ( const cuiWidget *Widget )
	{
	( void ) Widget;
	int Index = cuiGetSelectedItemInListBox ( ListBoxHandle );
	if ( Index >= 0 )
		{
		cuiRemoveItemFromListBox ( ListBoxHandle, Index );
		printf ( "Removed list item at index %d\n", Index );
		}
	}

int main ( int argc, char *argv[] )
	{
	printf ( "Initializing %s backend\n", cuiGetBackendName() );
	if ( cuiInitialize() == false )
		{
		fprintf ( stderr, "Failed to initialize CrossUI\n" );
		return 1;
		}

	printf ( "CrossUI backend: %s\n", cuiGetBackendName () );
	cuiWidget *WindowHandle = cuiCreateWindow ( "Window title", 100, 100, 500, 500 );
	if ( WindowHandle == cuiWidget_Invalid )
		{
		fprintf ( stderr, "Failed to create window\n" );
		cuiShutdown ();
		return 1;
		}
	cuiWidgetCallbacks Callbacks = cuiGetWidgetCallbacks ( WindowHandle );
	Callbacks.Moved = &OnWindowMoved;
	Callbacks.Resized = &OnWindowResized;
	Callbacks.Destroyed = &OnWindowDestroyed;
	cuiSetWidgetCallbacks ( WindowHandle, Callbacks );

	cuiWidget *Slider = cuiCreateSlider ( WindowHandle, 0, 0, 100, 20, 0, 100, 46 );
	Callbacks = cuiGetWidgetCallbacks ( Slider );
	Callbacks.SliderChangedValue = OnSliderChanged;
	cuiSetWidgetCallbacks ( Slider, Callbacks );

	cuiWidget *CheckBox = cuiCreateCheckbox ( WindowHandle, "My check", 0, 50, 100, 20, true );
	Callbacks = cuiGetWidgetCallbacks ( CheckBox );
	Callbacks.CheckChanged = OnCheckChanged;
	cuiSetWidgetCallbacks ( CheckBox, Callbacks );

	ProgressBarHandle = cuiCreateProgressBar ( WindowHandle, 0, 100, 100, 20 );
	cuiCreateLabel ( WindowHandle, "Label widget", 0, 130, 100, 20 );

	cuiWidget *ComboBox = cuiCreateComboBox ( WindowHandle, 0, 150, 100, 20 );
	cuiAddItemToComboBox ( ComboBox, "GLSL" );
	cuiAddItemToComboBox ( ComboBox, "HLSL" );
	cuiAddItemToComboBox ( ComboBox, "Metal" );
	cuiSetSelectedItemInComboBox ( ComboBox, 0 );
	Callbacks = cuiGetWidgetCallbacks ( ComboBox );
	Callbacks.ComboBoxChanged = OnComboSelectionChanged;
	cuiSetWidgetCallbacks ( ComboBox, Callbacks );

	cuiWidget *GreetButton = cuiCreateButton ( WindowHandle, "Greet", 0, 200, 100, 32 );
	Callbacks = cuiGetWidgetCallbacks ( GreetButton );
	Callbacks.Clicked = &OnGreet;
	cuiSetWidgetCallbacks ( GreetButton, Callbacks );

	cuiWidget *ShutdownButton = cuiCreateButton ( WindowHandle, "Shutdown", 0, 250, 100, 32 );
	Callbacks = cuiGetWidgetCallbacks ( ShutdownButton );
	Callbacks.Clicked = &OnShutdown;
	cuiSetWidgetCallbacks ( ShutdownButton, Callbacks );

	cuiWidget *GroupBox = cuiCreateGroupBox ( WindowHandle, "TextStuff", 280, 0, 120, 220 );

	cuiWidget *TextArea = cuiCreateTextArea ( GroupBox, "This is a text area", 10, 10, 100, 100 );
	Callbacks = cuiGetWidgetCallbacks ( TextArea );
	Callbacks.TextAreaChanged = OnTextAreaContentChanged;
	cuiSetWidgetCallbacks ( TextArea, Callbacks );

	cuiWidget *TextBox = cuiCreateTextBox ( GroupBox, "This is a text box", 10, 129, 100, 100 );
	Callbacks = cuiGetWidgetCallbacks ( TextBox );
	Callbacks.TextBoxChanged = OnTextBoxContentChanged;
	cuiSetWidgetCallbacks ( TextBox, Callbacks );

	ListBoxHandle = cuiCreateListBox ( WindowHandle, 120, 0, 100, 100 );
	Callbacks = cuiGetWidgetCallbacks ( ListBoxHandle );
	Callbacks.ListBoxSelectionChanged = OnListBoxSelectionChanged;
	cuiAddItemToListBox ( ListBoxHandle, "Item1" );
	cuiAddItemToListBox ( ListBoxHandle, "Item2" );
	cuiAddItemToListBox ( ListBoxHandle, "Item3" );
	cuiSetSelectedItemInListBox ( ListBoxHandle, 0 );
	cuiSetWidgetCallbacks ( ListBoxHandle, Callbacks );

	cuiWidget *AddItemButton = cuiCreateButton ( WindowHandle, "Add item", 120, 110, 100, 28 );
	Callbacks = cuiGetWidgetCallbacks ( AddItemButton );
	Callbacks.Clicked = OnAddListItem;
	cuiSetWidgetCallbacks ( AddItemButton, Callbacks );

	cuiWidget *RemoveItemButton = cuiCreateButton ( WindowHandle, "Remove", 120, 145, 100, 28 );
	Callbacks = cuiGetWidgetCallbacks ( RemoveItemButton );
	Callbacks.Clicked = OnRemoveListItem;
	cuiSetWidgetCallbacks ( RemoveItemButton, Callbacks );

	cuiWidget *Tree = cuiCreateTree ( WindowHandle, 120, 190, 180, 220 );
	Callbacks = cuiGetWidgetCallbacks ( Tree );
	Callbacks.TreeSelectionChanged = OnTreeSelectionChanged;
	cuiTreeItem Root = cuiAddItemToTree ( Tree, cuiTreeItem_Root, "Root" );
	cuiAddItemToTree ( Tree, Root, "Child 1" );
	cuiAddItemToTree ( Tree, Root, "Child 2" );
	cuiAddItemToTree ( Tree, cuiTreeItem_Root, "Another root" );
	cuiSetWidgetCallbacks ( Tree, Callbacks );

	while ( cuiUpdate ( true ) == true )
		{}
	cuiShutdown();

	return 0;
	}
