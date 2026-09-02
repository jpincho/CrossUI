#include "CrossUI.h"
#include <stdio.h>
#include <stdlib.h>

static GUIHandle NameBox;
static GUIHandle LanguageCombo;
static GUIHandle ShaderList;
static GUIHandle ShaderTree;
static GUIHandle QualitySlider;
static GUIHandle Progress;
static GUIHandle NotesArea;
static GUIHandle DebugCheck;

static void OnLanguageChanged ( GUIHandle Widget, void *UserData )
	{
	char Status[128];
	int Index = cuiComboGetSelected ( Widget );
	( void ) UserData;
	snprintf ( Status, sizeof ( Status ), "Language index: %d", Index );
	cuiProgressSetValue ( Progress, ( Index + 1 ) * 25 );
	cuiSetWindowTitle ( cuiGetParent ( Widget ), Status );
	}

static void OnQualityChanged ( GUIHandle Widget, void *UserData )
	{
	( void ) UserData;
	cuiProgressSetValue ( Progress, cuiSliderGetValue ( Widget ) );
	}

static void OnTreeChanged ( GUIHandle Widget, void *UserData )
	{
	char Text[128];
	cuiTreeItem Selected = cuiTreeGetSelected ( Widget );
	( void ) UserData;
	cuiTreeGetItemText ( Widget, Selected, Text, sizeof ( Text ) );
	if ( Text[0] != 0 )
		cuiSetText ( NameBox, Text );
	}

static void OnGreet ( GUIHandle Widget, void *UserData )
	{
	char Name[128];
	char Notes[512];
	char Message[768];
	GUIHandle Window = ( GUIHandle ) UserData;
	( void ) Widget;
	cuiGetText ( NameBox, Name, sizeof ( Name ) );
	cuiGetText ( NotesArea, Notes, sizeof ( Notes ) );
	snprintf ( Message, sizeof ( Message ), "Hello %s\nDebug: %s\n%s",
	           ( Name[0] != 0 ) ? Name : "there",
	           cuiGetChecked ( DebugCheck ) ? "on" : "off",
	           Notes );
	cuiMessageBox ( Window, "Greeting", Message, cuiMessage_Info );
	}

static void OnOpenFile ( GUIHandle Widget, void *UserData )
	{
	GUIHandle Window = ( GUIHandle ) UserData;
	char *Path = cuiOpenFileDialog ( Window, "Open shader", "Shader Files (*.glsl;*.frag;*.vert)|*.glsl;*.frag;*.vert|All Files (*.*)|*.*" );
	( void ) Widget;
	if ( Path != NULL )
		{
		cuiSetText ( NameBox, Path );
		cuiMessageBox ( Window, "Opened", Path, cuiMessage_Info );
		free ( Path );
		}
	}

static void OnQuit ( GUIHandle Widget, void *UserData )
	{
	( void ) Widget;
	cuiDestroyWindow ( ( GUIHandle ) UserData );
	}

int main ( void )
	{
	GUIHandle Window;
	GUIHandle GraphGroup;
	GUIHandle GreetButton;
	GUIHandle OpenButton;
	GUIHandle QuitButton;
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
	if ( Window == GUIHandle_Invalid )
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
