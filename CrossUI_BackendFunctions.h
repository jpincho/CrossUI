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
void cuiBackend_SetText ( cuiWidget *Widget, const char *Text );
void cuiBackend_GetText ( cuiWidget *Widget, char *Buffer, const unsigned BufferSize );
void cuiBackend_SetFocus ( cuiWidget *Widget );

void cuiBackend_CheckBox_SetState ( cuiWidget *Widget, const bool State );
bool cuiBackend_CheckBox_GetState ( cuiWidget *Widget );

void cuiBackend_Slider_SetRange ( cuiWidget *Widget, const float Min, const float Max );
void cuiBackend_Slider_SetValue ( cuiWidget *Widget, const float Value );
float cuiBackend_Slider_GetValue ( cuiWidget *Widget );

void cuiBackend_ProgressBar_SetValue ( cuiWidget *Widget, const float Value );