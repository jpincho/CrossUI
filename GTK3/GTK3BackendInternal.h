#pragma once
#include "../CrossUI.h"
#include "../CrossUI_Internal.h"
#include "../CrossUI_InternalBackendFunctions.h"
#include <Platform/Platform.h>
#include <Platform/Logger.h>
#include <stdlib.h>
#include <gtk/gtk.h>

GtkWidget *GetContainer ( const cuiWidget *Widget );
GtkWidget *GetOutmostWidgetHandle ( const cuiWidget *Widget );
void FinishedCreatingNewWidget ( cuiWidget *Widget );