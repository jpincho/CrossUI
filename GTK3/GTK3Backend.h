#pragma once
#include "../CrossUI.h"
#include "../CrossUI_Internal.h"
#include "../CrossUI_BackendFunctions.h"
#include <gtk/gtk.h>

GtkWidget *GetContainer ( cuiWidget *Widget );
GtkWidget *GetInnermostWidget ( cuiWidget *Widget );