#include <CrossUI.h>
#include <stdio.h>

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

	while ( cuiUpdate ( true ) == true )
		{}
	cuiShutdown();

	return 0;
	}