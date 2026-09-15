#include <CrossUI.h>
#include <stdio.h>

cuiHandle ProgressBarHandle;

void OnWindowResized ( const cuiHandle WindowHandle, const unsigned Width, const unsigned Height )
	{
	printf ( "Window %p resized to %ux%u\n", WindowHandle, Width, Height );
	}

void OnWindowMoved ( const cuiHandle WindowHandle, const int X, int Y )
	{
	printf ( "Window %p moved to %dx%d\n", WindowHandle, X, Y );
	}

void OnWindowDestroyed ( const cuiHandle WindowHandle )
	{
	printf ( "Window %p closed\n", WindowHandle );
	}

void OnGreet ( const cuiHandle WindowHandle )
	{
	printf ( "Greet pressed\n" );
	}

void OnShutdown ( const cuiHandle WindowHandle )
	{
	printf ( "Forcing shutdown\n" );
	cuiShutdown();
	}

void OnCheckChanged ( const cuiHandle WindowHandle, const bool State )
	{
	printf ( "Checkbox changed to %s\n", State ? "true" : "false" );
	}

void OnSliderChanged ( const cuiHandle WidgetHandle, const float Value )
	{
	printf ( "Slider changed - %f\n", Value );
	cuiSetProgressBarValue ( ProgressBarHandle, Value );
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
	cuiHandle WindowHandle = cuiCreateWindow ( "Window title", 100, 100, 500, 500 );
	if ( WindowHandle == cuiHandle_Invalid )
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

	cuiHandle GreetButton = cuiCreateButton ( WindowHandle, "Greet", 380, 56, 100, 32 );
	Callbacks = cuiGetWidgetCallbacks ( GreetButton );
	Callbacks.Clicked = &OnGreet;
	cuiSetWidgetCallbacks ( GreetButton, Callbacks );

	cuiHandle ShutdownButton = cuiCreateButton ( WindowHandle, "Force Shutdown", 490, 56, 100, 32 );
	Callbacks = cuiGetWidgetCallbacks ( ShutdownButton );
	Callbacks.Clicked = &OnShutdown;
	cuiSetWidgetCallbacks ( ShutdownButton, Callbacks );

	cuiHandle Slider = cuiCreateSlider ( WindowHandle, 0, 50, 100, 20, 0, 100, 46 );
	Callbacks = cuiGetWidgetCallbacks ( Slider );
	Callbacks.SliderChangedValue = OnSliderChanged;
	cuiSetWidgetCallbacks ( Slider, Callbacks );

	cuiHandle CheckBox = cuiCreateCheckbox ( WindowHandle, "My check", 0, 100, 100, 20 );
	Callbacks = cuiGetWidgetCallbacks ( CheckBox );
	Callbacks.CheckChanged = OnCheckChanged;
	cuiSetWidgetCallbacks ( CheckBox, Callbacks );

	ProgressBarHandle = cuiCreateProgressBar ( WindowHandle, 0, 100, 100, 20 );

	while ( cuiUpdate ( true ) == true )
		{}
	cuiShutdown();

	return 0;
	}