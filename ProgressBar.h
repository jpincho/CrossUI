#pragma once
#include "CrossUI.h"

cuiHandle cuiCreateProgressBar ( const cuiHandle ParentHandle, const int X, const int Y, const unsigned Width, const unsigned Height );
void cuiSetProgressBarValue ( const cuiHandle WidgetHandle, const float Value );
float cuiGetProgressBarValue ( const cuiHandle WidgetHandle );