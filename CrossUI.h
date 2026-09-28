#pragma once
#include <stdbool.h>
#include "CrossUI_PublicBackendFunctions.h"

bool cuiInitialize ( void );
bool cuiInitializeWithArgs ( int argc, char *argv[] );
void cuiShutdown ( void );
void cuiDestroyWidget ( cuiWidget *Widget );
void cuiSetWidgetCallbacks ( cuiWidget *Widget, cuiWidgetCallbacks Callbacks );
cuiWidgetCallbacks cuiGetWidgetCallbacks ( const cuiWidget *Widget );
cuiWidget *cuiGetParent ( const cuiWidget *Widget );
