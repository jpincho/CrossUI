#pragma once
#include <stdbool.h>
typedef struct cuiWidget cuiWidget;

bool cuiBackend_Initialize ( void );
void cuiBackend_Shutdown ( void );
const char *cuiBackend_Name ( void );
bool cuiBackend_CreateNativeWidget ( cuiWidget *Widget );
void cuiBackend_DestroyNativeWidget ( cuiWidget *Widget );
bool cuiBackend_Update ( bool Wait );
void cuiBackend_SetVisible ( cuiWidget *Widget, const bool Visible );
void cuiBackend_SetEnabled ( cuiWidget *Widget, const bool Enabled );