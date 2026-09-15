#include "ProgressBar.h"
#include "CrossUI_Internal.h"
#include "CrossUI_BackendFunctions.h"
#include <stdlib.h>

cuiHandle cuiCreateProgressBar ( const cuiHandle ParentHandle, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return cuiCreateWidget ( cuiType_ProgressBar, ParentHandle, NULL, X, Y, Width, Height );
	}

void cuiSetProgressBarValue ( const cuiHandle WidgetHandle, const float Value )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WidgetHandle );
	if ( Widget == NULL )
		return;
	Widget->ProgressBarData.Value = Value;
	cuiBackend_ProgressBar_SetValue ( Widget, Value );
	}

float cuiGetProgressBarValue ( const cuiHandle WidgetHandle )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WidgetHandle );
	if ( Widget == NULL )
		return 0.0f;
	return Widget->ProgressBarData.Value;
	}