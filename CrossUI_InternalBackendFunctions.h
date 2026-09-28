#pragma once
#include "CrossUI.h"
#include "CrossUI_Internal.h"

void cuiBackend_DestroyNativeWidget ( cuiWidget *Widget );
bool cuiBackend_Initialize ( int argc, char *argv[] );
void cuiBackend_Shutdown ( void );
