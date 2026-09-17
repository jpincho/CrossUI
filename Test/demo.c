#include <CrossUI.h>
#include <stdio.h>
#if 1
cuiWidget *ProgressBarHandle;

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
	cuiSetProgressBarValue ( ProgressBarHandle, Value );
	}

void OnComboSelectionChanged ( const cuiWidget *Widget, const int NewIndex )
	{
	printf ( "Combo changed - %d\n", NewIndex );
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

	cuiWidget *GreetButton = cuiCreateButton ( WindowHandle, "Greet", 380, 56, 100, 32 );
	Callbacks = cuiGetWidgetCallbacks ( GreetButton );
	Callbacks.Clicked = &OnGreet;
	cuiSetWidgetCallbacks ( GreetButton, Callbacks );

	cuiWidget *ShutdownButton = cuiCreateButton ( WindowHandle, "Force Shutdown", 490, 56, 100, 32 );
	Callbacks = cuiGetWidgetCallbacks ( ShutdownButton );
	Callbacks.Clicked = &OnShutdown;
	cuiSetWidgetCallbacks ( ShutdownButton, Callbacks );

	cuiWidget *Slider = cuiCreateSlider ( WindowHandle, 0, 50, 100, 20, 0, 100, 46 );
	Callbacks = cuiGetWidgetCallbacks ( Slider );
	Callbacks.SliderChangedValue = OnSliderChanged;
	cuiSetWidgetCallbacks ( Slider, Callbacks );

	cuiWidget *CheckBox = cuiCreateCheckbox ( WindowHandle, "My check", 0, 100, 100, 20, true );
	Callbacks = cuiGetWidgetCallbacks ( CheckBox );
	Callbacks.CheckChanged = OnCheckChanged;
	cuiSetWidgetCallbacks ( CheckBox, Callbacks );

	ProgressBarHandle = cuiCreateProgressBar ( WindowHandle, 0, 150, 100, 20 );

	cuiWidget *ComboBox = cuiCreateCombo ( WindowHandle, 0, 200, 100, 20 );
	cuiAddItemToCombo ( ComboBox, "GLSL" );
	cuiAddItemToCombo ( ComboBox, "HLSL" );
	cuiAddItemToCombo ( ComboBox, "Metal" );
	cuiSetSelectedItemInCombo ( ComboBox, 0 );
	Callbacks = cuiGetWidgetCallbacks ( ComboBox );
	Callbacks.ComboBoxChanged = OnComboSelectionChanged;
	cuiSetWidgetCallbacks ( ComboBox, Callbacks );

	while ( cuiUpdate ( true ) == true )
		{}
	cuiShutdown();

	return 0;
	}




#else
#include "CrossUI.h"
#include <Platform/Platform.h>
#include <stdio.h>
#include <stdlib.h>

static cuiWidget *NameBox;
static cuiWidget *LanguageCombo;
static cuiWidget *ShaderList;
static cuiWidget *ShaderTree;
static cuiWidget *QualitySlider;
static cuiWidget *Progress;
static cuiWidget *NotesArea;
static cuiWidget *DebugCheck;

static void OnLanguageChanged ( cuiWidget * Widget, void *UserData )
	{
	UNUSED ( UserData );
	char Status[128];
	int Index = cuiGetSelectedItemInCombo ( Widget );
	snprintf ( Status, sizeof ( Status ), "Language index: %d", Index );
	cuiSetProgressBarValue ( Progress, ( Index + 1 ) * 25 );
	cuiSetWidgetText ( cuiGetParent ( Widget ), Status );
	}

static void OnQualityChanged ( cuiWidget * Widget, void *UserData )
	{
	UNUSED ( UserData );
	cuiSetProgressBarValue ( Progress, cuiGetSliderValue ( Widget ) );
	}

static void OnTreeChanged ( cuiWidget * Widget, void *UserData )
	{
	UNUSED ( UserData );
	char Text[128];
	cuiTreeItem Selected = cuiTreeGetSelected ( Widget );

	cuiTreeGetItemText ( Widget, Selected, Text, sizeof ( Text ) );
	if ( Text[0] != 0 )
		cuiSetWidgetText ( NameBox, Text );
	}

static void OnGreet ( cuiWidget * Widget, void *UserData )
	{
	UNUSED ( Widget );
	char Name[128];
	char Notes[512];
	char Message[768];
	cuiWidget * Window = ( cuiWidget * ) UserData;
	cuiGetWidgetText ( NameBox, Name, sizeof ( Name ) );
	cuiGetWidgetText ( NotesArea, Notes, sizeof ( Notes ) );
	snprintf ( Message, sizeof ( Message ), "Hello %s\nDebug: %s\n%s",
	           ( Name[0] != 0 ) ? Name : "there",
	           cuiGetCheckboxState ( DebugCheck ) ? "on" : "off",
	           Notes );
	cuiMessageBox ( Window, "Greeting", Message, cuiMessage_Info );
	}

static void OnOpenFile ( cuiWidget * Widget, void *UserData )
	{
	UNUSED ( Widget );
	cuiWidget * Window = ( cuiWidget * ) UserData;
	char *Path = cuiOpenFileDialog ( Window, "Open shader", "Shader Files (*.glsl;*.frag;*.vert)|*.glsl;*.frag;*.vert|All Files (*.*)|*.*" );
	if ( Path != NULL )
		{
		cuiSetWidgetText ( NameBox, Path );
		cuiMessageBox ( Window, "Opened", Path, cuiMessage_Info );
		free ( Path );
		}
	}

static void OnQuit ( cuiWidget * Widget, void *UserData )
	{
	UNUSED ( Widget );
	cuiDestroyWidget ( ( cuiWidget * ) UserData );
	}

int main ( void )
	{
	cuiWidget *Window;
	cuiWidget *GraphGroup;
	cuiWidget *GreetButton;
	cuiWidget *OpenButton;
	cuiWidget *QuitButton;
	cuiTreeItem Shaders;
	cuiTreeItem Passes;
	cuiTreeItem Vertex;
	cuiTreeItem Fragment;

	if ( cuiInitialize () == false )
		{
		fprintf ( stderr, "Failed to initialize CrossUI\n" );
		return 1;
		}

	printf ( "CrossUI backend: %s\n", cuiGetBackendName () );

	Window = cuiCreateWindow ( "CrossUI Demo", 80, 60, 820, 560 );
	if ( Window == cuiWidget_Invalid )
		{
		fprintf ( stderr, "Failed to create window\n" );
		cuiShutdown ();
		return 1;
		}

	cuiCreateLabel ( Window, "Name", 20, 20, 80, 24 );
	NameBox = cuiCreateTextBox ( Window, "New Shader", 110, 18, 250, 28 );
	DebugCheck = cuiCreateCheckBox ( Window, "Debug output", 380, 20, 160, 24 );

	cuiCreateLabel ( Window, "Language", 20, 60, 80, 24 );
	LanguageCombo = cuiCreateComboBox ( Window, 110, 58, 250, 28 );
	cuiComboAddItem ( LanguageCombo, "GLSL" );
	cuiComboAddItem ( LanguageCombo, "HLSL" );
	cuiComboAddItem ( LanguageCombo, "Metal" );
	cuiComboSetSelected ( LanguageCombo, 0 );
	cuiSetChangeCallback ( LanguageCombo, OnLanguageChanged, NULL );

	GreetButton = cuiCreateButton ( Window, "Greet", 380, 56, 100, 32 );
	cuiSetClickCallback ( GreetButton, OnGreet, Window );
	OpenButton = cuiCreateButton ( Window, "Open...", 490, 56, 100, 32 );
	cuiSetClickCallback ( OpenButton, OnOpenFile, Window );
	QuitButton = cuiCreateButton ( Window, "Quit", 600, 56, 100, 32 );
	cuiSetClickCallback ( QuitButton, OnQuit, Window );

	GraphGroup = cuiCreateGroupBox ( Window, "Graph", 20, 100, 360, 250 );
	cuiCreateLabel ( GraphGroup, "Passes", 12, 24, 80, 20 );
	ShaderList = cuiCreateListBox ( GraphGroup, 12, 48, 150, 180 );
	cuiListAddItem ( ShaderList, "Forward" );
	cuiListAddItem ( ShaderList, "Deferred" );
	cuiListAddItem ( ShaderList, "Shadow" );
	cuiListSetSelected ( ShaderList, 0 );

	ShaderTree = cuiCreateTree ( GraphGroup, 180, 48, 160, 180 );
	Shaders = cuiTreeAddItem ( ShaderTree, cuiTreeItem_Root, "Shaders" );
	Passes = cuiTreeAddItem ( ShaderTree, Shaders, "Passes" );
	Vertex = cuiTreeAddItem ( ShaderTree, Passes, "Vertex" );
	Fragment = cuiTreeAddItem ( ShaderTree, Passes, "Fragment" );
	cuiTreeAddItem ( ShaderTree, Vertex, "Transform" );
	cuiTreeAddItem ( ShaderTree, Fragment, "Lighting" );
	cuiTreeExpand ( ShaderTree, Shaders, true );
	cuiTreeExpand ( ShaderTree, Passes, true );
	cuiSetChangeCallback ( ShaderTree, OnTreeChanged, NULL );

	cuiCreateLabel ( Window, "Quality", 400, 110, 80, 24 );
	QualitySlider = cuiCreateSlider ( Window, 0, 100, 40, 400, 140, 390, 32 );
	cuiSetChangeCallback ( QualitySlider, OnQualityChanged, NULL );
	Progress = cuiCreateProgressBar ( Window, 400, 180, 390, 24 );
	cuiProgressSetValue ( Progress, 40 );

	cuiCreateLabel ( Window, "Notes", 400, 220, 80, 24 );
	NotesArea = cuiCreateTextArea ( Window, "Build a shader graph with CrossUI widgets.", 400, 248, 390, 100 );

	cuiRun ();
	cuiShutdown ();
	return 0;
	}
#endif