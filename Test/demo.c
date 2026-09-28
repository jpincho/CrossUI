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

	while ( cuiUpdate ( true ) == true )
		{}
	cuiShutdown();

	return 0;
	}
