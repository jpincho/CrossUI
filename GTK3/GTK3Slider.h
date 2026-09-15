#pragma once
#include "../CrossUI_Internal.h"

bool cuiBackend_Slider_Create ( cuiWidget *Widget );
void cuiBackend_Slider_SetRange ( cuiWidget *Widget, const float Min, const float Max );
void cuiBackend_Slider_SetValue ( cuiWidget *Widget, const float Value );
float cuiBackend_Slider_GetValue ( cuiWidget *Widget );