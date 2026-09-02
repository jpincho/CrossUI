#define _POSIX_C_SOURCE 200809L
#include "CrossUI_Internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct cuiZenityTreeItem cuiZenityTreeItem;
typedef struct cuiZenityTree cuiZenityTree;

struct cuiZenityTreeItem
	{
	cuiZenityTreeItem *Parent;
	cuiZenityTreeItem **Children;
	unsigned ChildCount;
	unsigned ChildCapacity;
	char *Text;
	void *UserData;
	};

struct cuiZenityTree
	{
	cuiZenityTreeItem **Roots;
	unsigned RootCount;
	unsigned RootCapacity;
	cuiZenityTreeItem *Selected;
	};

typedef struct
	{
	char *Data;
	size_t Length;
	size_t Capacity;
	} cuiBuf;

static char ZenityPath[256] = "/usr/bin/zenity";

static bool BufReserve ( cuiBuf *Buffer, const size_t Needed )
	{
	size_t Capacity;
	char *Grown;
	if ( Needed <= Buffer->Capacity )
		return true;
	Capacity = ( Buffer->Capacity == 0 ) ? 256 : Buffer->Capacity;
	while ( Capacity < Needed )
		Capacity *= 2;
	Grown = ( char * ) realloc ( Buffer->Data, Capacity );
	if ( Grown == NULL )
		return false;
	Buffer->Data = Grown;
	Buffer->Capacity = Capacity;
	return true;
	}

static bool BufAppend ( cuiBuf *Buffer, const char *Text )
	{
	size_t Length;
	if ( Text == NULL )
		Text = "";
	Length = strlen ( Text );
	if ( BufReserve ( Buffer, Buffer->Length + Length + 1 ) == false )
		return false;
	memcpy ( Buffer->Data + Buffer->Length, Text, Length + 1 );
	Buffer->Length += Length;
	return true;
	}

static bool BufAppendQuoted ( cuiBuf *Buffer, const char *Text )
	{
	size_t Index;
	if ( Text == NULL )
		Text = "";
	if ( BufAppend ( Buffer, "'" ) == false )
		return false;
	for ( Index = 0; Text[Index] != 0; ++Index )
		{
		if ( Text[Index] == '\'' )
			{
			if ( BufAppend ( Buffer, "'\\''" ) == false )
				return false;
			}
		else
			{
			char Char[2] = { Text[Index], 0 };
			if ( BufAppend ( Buffer, Char ) == false )
				return false;
			}
		}
	return BufAppend ( Buffer, "'" );
	}

static void BufFree ( cuiBuf *Buffer )
	{
	free ( Buffer->Data );
	Buffer->Data = NULL;
	Buffer->Length = 0;
	Buffer->Capacity = 0;
	}

static char *RunZenity ( const char *Arguments, int *ExitCode )
	{
	cuiBuf Command = { 0 };
	cuiBuf Output = { 0 };
	char Chunk[512];
	FILE *Pipe;
	int Status;

	BufAppend ( &Command, ZenityPath );
	BufAppend ( &Command, " " );
	BufAppend ( &Command, Arguments );
	Pipe = popen ( Command.Data, "r" );
	BufFree ( &Command );
	if ( Pipe == NULL )
		{
		if ( ExitCode != NULL )
			*ExitCode = -1;
		return NULL;
		}

	BufAppend ( &Output, "" );
	while ( fgets ( Chunk, sizeof ( Chunk ), Pipe ) != NULL )
		BufAppend ( &Output, Chunk );
	Status = pclose ( Pipe );
	if ( ExitCode != NULL )
		{
		if ( WIFEXITED ( Status ) )
			*ExitCode = WEXITSTATUS ( Status );
		else
			*ExitCode = -1;
		}
	if ( ( Output.Length > 0 ) && ( Output.Data[Output.Length - 1] == '\n' ) )
		{
		Output.Data[Output.Length - 1] = 0;
		--Output.Length;
		}
	return Output.Data;
	}

static bool PathIsExecutable ( const char *Path )
	{
	return access ( Path, X_OK ) == 0;
	}

bool cuiBackend_Initialize ( void )
	{
	if ( PathIsExecutable ( "/usr/bin/zenity" ) )
		{
		strncpy ( ZenityPath, "/usr/bin/zenity", sizeof ( ZenityPath ) - 1 );
		return true;
		}
	if ( PathIsExecutable ( "/usr/local/bin/zenity" ) )
		{
		strncpy ( ZenityPath, "/usr/local/bin/zenity", sizeof ( ZenityPath ) - 1 );
		return true;
		}
	return false;
	}

void cuiBackend_Shutdown ( void )
	{
	}

const char *cuiBackend_Name ( void )
	{
	return "Zenity";
	}

static bool GrowPointers ( void ***Array, unsigned *Capacity, const unsigned Needed )
	{
	unsigned NewCapacity;
	void **Grown;
	if ( Needed <= *Capacity )
		return true;
	NewCapacity = ( *Capacity == 0 ) ? 4 : ( *Capacity * 2 );
	while ( NewCapacity < Needed )
		NewCapacity *= 2;
	Grown = ( void ** ) realloc ( *Array, NewCapacity * sizeof ( void * ) );
	if ( Grown == NULL )
		return false;
	*Array = Grown;
	*Capacity = NewCapacity;
	return true;
	}

static void FreeTreeItem ( cuiZenityTreeItem *Item )
	{
	unsigned Index;
	if ( Item == NULL )
		return;
	for ( Index = 0; Index < Item->ChildCount; ++Index )
		FreeTreeItem ( Item->Children[Index] );
	free ( Item->Children );
	free ( Item->Text );
	free ( Item );
	}

static void FreeTree ( cuiZenityTree *Tree )
	{
	unsigned Index;
	if ( Tree == NULL )
		return;
	for ( Index = 0; Index < Tree->RootCount; ++Index )
		FreeTreeItem ( Tree->Roots[Index] );
	free ( Tree->Roots );
	free ( Tree );
	}

bool cuiBackend_Realize ( cuiWidget *Widget )
	{
	if ( Widget->Type == cuiType_Tree )
		{
		cuiZenityTree *Tree = ( cuiZenityTree * ) calloc ( 1, sizeof ( cuiZenityTree ) );
		if ( Tree == NULL )
			return false;
		Widget->NativeHandle = Tree;
		Widget->NativeInner = Tree;
		}
	return true;
	}

void cuiBackend_DestroyNative ( cuiWidget *Widget )
	{
	if ( Widget->Type == cuiType_Tree )
		{
		FreeTree ( ( cuiZenityTree * ) Widget->NativeHandle );
		Widget->NativeHandle = NULL;
		Widget->NativeInner = NULL;
		}
	}

void cuiBackend_SetText ( cuiWidget *Widget, const char *Text )
	{
	( void ) Widget;
	( void ) Text;
	}

void cuiBackend_GetText ( cuiWidget *Widget, char *Buffer, size_t BufferSize )
	{
	cuiInternal_CopyToBuffer ( Buffer, BufferSize, Widget->Text );
	}

void cuiBackend_SetBounds ( cuiWidget *Widget )
	{
	( void ) Widget;
	}

void cuiBackend_SetVisible ( cuiWidget *Widget )
	{
	( void ) Widget;
	}

void cuiBackend_SetEnabled ( cuiWidget *Widget )
	{
	( void ) Widget;
	}

void cuiBackend_SetFocus ( cuiWidget *Widget )
	{
	( void ) Widget;
	}

void cuiBackend_SetChecked ( cuiWidget *Widget )
	{
	( void ) Widget;
	}

bool cuiBackend_GetChecked ( cuiWidget *Widget )
	{
	return Widget->Checked;
	}

void cuiBackend_ClearItems ( cuiWidget *Widget )
	{
	( void ) Widget;
	}

int cuiBackend_AddItem ( cuiWidget *Widget, const char *Text )
	{
	( void ) Text;
	return ( int ) Widget->ItemCount - 1;
	}

void cuiBackend_SetSelected ( cuiWidget *Widget, int Index )
	{
	Widget->SelectedIndex = Index;
	}

int cuiBackend_GetSelected ( cuiWidget *Widget )
	{
	return Widget->SelectedIndex;
	}

cuiTreeItem cuiBackend_TreeAddItem ( cuiWidget *Widget, cuiTreeItem Parent, const char *Text )
	{
	cuiZenityTree *Tree = ( cuiZenityTree * ) Widget->NativeHandle;
	cuiZenityTreeItem *Item;
	if ( Tree == NULL )
		return cuiTreeItem_Root;
	Item = ( cuiZenityTreeItem * ) calloc ( 1, sizeof ( cuiZenityTreeItem ) );
	if ( Item == NULL )
		return cuiTreeItem_Root;
	Item->Text = cuiInternal_StrDup ( Text );
	Item->Parent = ( cuiZenityTreeItem * ) Parent;
	if ( Parent == cuiTreeItem_Root )
		{
		if ( GrowPointers ( ( void *** ) &Tree->Roots, &Tree->RootCapacity, Tree->RootCount + 1 ) == false )
			{
			free ( Item->Text );
			free ( Item );
			return cuiTreeItem_Root;
			}
		Tree->Roots[Tree->RootCount++] = Item;
		}
	else
		{
		cuiZenityTreeItem *ParentItem = ( cuiZenityTreeItem * ) Parent;
		if ( GrowPointers ( ( void *** ) &ParentItem->Children, &ParentItem->ChildCapacity, ParentItem->ChildCount + 1 ) == false )
			{
			free ( Item->Text );
			free ( Item );
			return cuiTreeItem_Root;
			}
		ParentItem->Children[ParentItem->ChildCount++] = Item;
		}
	return ( cuiTreeItem ) Item;
	}

void cuiBackend_TreeSetItemText ( cuiWidget *Widget, cuiTreeItem Item, const char *Text )
	{
	cuiZenityTreeItem *Node = ( cuiZenityTreeItem * ) Item;
	char *Copy;
	( void ) Widget;
	if ( Node == NULL )
		return;
	Copy = cuiInternal_StrDup ( Text );
	if ( Copy == NULL )
		return;
	free ( Node->Text );
	Node->Text = Copy;
	}

void cuiBackend_TreeGetItemText ( cuiWidget *Widget, cuiTreeItem Item, char *Buffer, size_t BufferSize )
	{
	cuiZenityTreeItem *Node = ( cuiZenityTreeItem * ) Item;
	( void ) Widget;
	cuiInternal_CopyToBuffer ( Buffer, BufferSize, ( Node != NULL ) ? Node->Text : "" );
	}

static void RemoveChildPointer ( cuiZenityTreeItem **List, unsigned *Count, cuiZenityTreeItem *Item )
	{
	unsigned Index;
	for ( Index = 0; Index < *Count; ++Index )
		{
		if ( List[Index] == Item )
			{
			memmove ( &List[Index], &List[Index + 1], ( *Count - Index - 1 ) * sizeof ( cuiZenityTreeItem * ) );
			-- ( *Count );
			return;
			}
		}
	}

void cuiBackend_TreeRemoveItem ( cuiWidget *Widget, cuiTreeItem Item )
	{
	cuiZenityTree *Tree = ( cuiZenityTree * ) Widget->NativeHandle;
	cuiZenityTreeItem *Node = ( cuiZenityTreeItem * ) Item;
	if ( ( Tree == NULL ) || ( Node == NULL ) )
		return;
	if ( Tree->Selected == Node )
		Tree->Selected = NULL;
	if ( Node->Parent == NULL )
		RemoveChildPointer ( Tree->Roots, &Tree->RootCount, Node );
	else
		RemoveChildPointer ( Node->Parent->Children, &Node->Parent->ChildCount, Node );
	FreeTreeItem ( Node );
	}

void cuiBackend_TreeClear ( cuiWidget *Widget )
	{
	cuiZenityTree *Tree = ( cuiZenityTree * ) Widget->NativeHandle;
	unsigned Index;
	if ( Tree == NULL )
		return;
	for ( Index = 0; Index < Tree->RootCount; ++Index )
		FreeTreeItem ( Tree->Roots[Index] );
	free ( Tree->Roots );
	Tree->Roots = NULL;
	Tree->RootCount = 0;
	Tree->RootCapacity = 0;
	Tree->Selected = NULL;
	}

cuiTreeItem cuiBackend_TreeGetSelected ( cuiWidget *Widget )
	{
	cuiZenityTree *Tree = ( cuiZenityTree * ) Widget->NativeHandle;
	return ( Tree != NULL ) ? ( cuiTreeItem ) Tree->Selected : cuiTreeItem_Root;
	}

void cuiBackend_TreeSetSelected ( cuiWidget *Widget, cuiTreeItem Item )
	{
	cuiZenityTree *Tree = ( cuiZenityTree * ) Widget->NativeHandle;
	if ( Tree != NULL )
		Tree->Selected = ( cuiZenityTreeItem * ) Item;
	}

void cuiBackend_TreeExpand ( cuiWidget *Widget, cuiTreeItem Item, bool Expand )
	{
	( void ) Widget;
	( void ) Item;
	( void ) Expand;
	}

void cuiBackend_TreeSetItemData ( cuiWidget *Widget, cuiTreeItem Item, void *Data )
	{
	( void ) Widget;
	if ( Item != NULL )
		( ( cuiZenityTreeItem * ) Item )->UserData = Data;
	}

void *cuiBackend_TreeGetItemData ( cuiWidget *Widget, cuiTreeItem Item )
	{
	( void ) Widget;
	return ( Item != NULL ) ? ( ( cuiZenityTreeItem * ) Item )->UserData : NULL;
	}

void cuiBackend_SliderSetRange ( cuiWidget *Widget )
	{
	( void ) Widget;
	}

void cuiBackend_SliderSetValue ( cuiWidget *Widget )
	{
	( void ) Widget;
	}

int cuiBackend_SliderGetValue ( cuiWidget *Widget )
	{
	return Widget->SliderValue;
	}

void cuiBackend_ProgressSetValue ( cuiWidget *Widget )
	{
	( void ) Widget;
	}

static void CollectFields ( cuiWidget *Widget, cuiWidget **Fields, unsigned *Count, const unsigned Capacity )
	{
	unsigned Index;
	if ( Widget == NULL )
		return;
	switch ( Widget->Type )
		{
		case cuiType_TextBox:
		case cuiType_TextArea:
		case cuiType_CheckBox:
		case cuiType_ComboBox:
		case cuiType_Slider:
			if ( *Count < Capacity )
				Fields[ ( *Count )++] = Widget;
			break;
		default:
			break;
		}
	for ( Index = 0; Index < Widget->ChildCount; ++Index )
		CollectFields ( Widget->Children[Index], Fields, Count, Capacity );
	}

static cuiWidget *FindFirstButton ( cuiWidget *Widget )
	{
	unsigned Index;
	cuiWidget *Found;
	if ( Widget == NULL )
		return NULL;
	if ( Widget->Type == cuiType_Button )
		return Widget;
	for ( Index = 0; Index < Widget->ChildCount; ++Index )
		{
		Found = FindFirstButton ( Widget->Children[Index] );
		if ( Found != NULL )
			return Found;
		}
	return NULL;
	}

static cuiWidget *FindListOrTree ( cuiWidget *Widget )
	{
	unsigned Index;
	cuiWidget *Found;
	if ( Widget == NULL )
		return NULL;
	if ( ( Widget->Type == cuiType_ListBox ) || ( Widget->Type == cuiType_Tree ) )
		return Widget;
	for ( Index = 0; Index < Widget->ChildCount; ++Index )
		{
		Found = FindListOrTree ( Widget->Children[Index] );
		if ( Found != NULL )
			return Found;
		}
	return NULL;
	}

static void FlattenTree ( cuiZenityTreeItem *Item, const char *Prefix, cuiBuf *Values, cuiZenityTreeItem **Map, unsigned *MapCount, const unsigned MapCapacity )
	{
	cuiBuf Label = { 0 };
	unsigned Index;
	BufAppend ( &Label, Prefix );
	if ( Prefix[0] != 0 )
		BufAppend ( &Label, " / " );
	BufAppend ( &Label, Item->Text );
	if ( *MapCount < MapCapacity )
		{
		if ( *MapCount > 0 )
			BufAppend ( Values, "|" );
		BufAppend ( Values, Label.Data );
		Map[ ( *MapCount )++] = Item;
		}
	for ( Index = 0; Index < Item->ChildCount; ++Index )
		FlattenTree ( Item->Children[Index], Label.Data, Values, Map, MapCount, MapCapacity );
	BufFree ( &Label );
	}

static void ApplyFormValues ( cuiWidget **Fields, const unsigned Count, char *Output )
	{
	unsigned Index = 0;
	char *Save = NULL;
	char *Token = strtok_r ( Output, "\x1e", &Save );
	while ( ( Token != NULL ) && ( Index < Count ) )
		{
		cuiWidget *Field = Fields[Index++];
		if ( ( Field->Type == cuiType_TextBox ) || ( Field->Type == cuiType_TextArea ) )
			{
			free ( Field->Text );
			Field->Text = cuiInternal_StrDup ( Token );
			cuiInternal_FireChange ( Field );
			}
		else if ( Field->Type == cuiType_CheckBox )
			{
			Field->Checked = ( strcmp ( Token, "Yes" ) == 0 );
			cuiInternal_FireChange ( Field );
			}
		else if ( Field->Type == cuiType_ComboBox )
			{
			unsigned ItemIndex;
			Field->SelectedIndex = -1;
			for ( ItemIndex = 0; ItemIndex < Field->ItemCount; ++ItemIndex )
				{
				if ( strcmp ( Field->Items[ItemIndex], Token ) == 0 )
					{
					Field->SelectedIndex = ( int ) ItemIndex;
					break;
					}
				}
			cuiInternal_FireChange ( Field );
			}
		else if ( Field->Type == cuiType_Slider )
			{
			Field->SliderValue = atoi ( Token );
			cuiInternal_FireChange ( Field );
			}
		Token = strtok_r ( NULL, "\x1e", &Save );
		}
	}

static bool ShowListDialog ( cuiWidget *Window, cuiWidget *List )
	{
	cuiBuf Args = { 0 };
	cuiZenityTreeItem *Map[256];
	unsigned MapCount = 0;
	unsigned Index;
	int ExitCode = 1;
	char *Output;
	cuiWidget *Button;

	BufAppend ( &Args, "--list --hide-header --column=Item --title=" );
	BufAppendQuoted ( &Args, Window->Text );
	if ( List->Type == cuiType_ListBox )
		{
		for ( Index = 0; Index < List->ItemCount; ++Index )
			{
			BufAppend ( &Args, " " );
			BufAppendQuoted ( &Args, List->Items[Index] );
			}
		}
	else
		{
		cuiZenityTree *Tree = ( cuiZenityTree * ) List->NativeHandle;
		cuiBuf Values = { 0 };
		if ( Tree != NULL )
			{
			for ( Index = 0; Index < Tree->RootCount; ++Index )
				FlattenTree ( Tree->Roots[Index], "", &Values, Map, &MapCount, 256 );
			}
		BufFree ( &Values );
		for ( Index = 0; Index < MapCount; ++Index )
			{
			BufAppend ( &Args, " " );
			BufAppendQuoted ( &Args, Map[Index]->Text );
			}
		}

	Output = RunZenity ( Args.Data, &ExitCode );
	BufFree ( &Args );
	if ( ExitCode != 0 )
		{
		free ( Output );
		cuiInternal_FireClose ( Window );
		cuiInternal_DestroyWidget ( Window );
		return false;
		}
	if ( List->Type == cuiType_ListBox )
		{
		List->SelectedIndex = -1;
		for ( Index = 0; Index < List->ItemCount; ++Index )
			{
			if ( ( Output != NULL ) && ( strcmp ( List->Items[Index], Output ) == 0 ) )
				{
				List->SelectedIndex = ( int ) Index;
				break;
				}
			}
		cuiInternal_FireChange ( List );
		}
	else
		{
		cuiZenityTree *Tree = ( cuiZenityTree * ) List->NativeHandle;
		if ( Tree != NULL )
			{
			Tree->Selected = NULL;
			for ( Index = 0; Index < MapCount; ++Index )
				{
				if ( ( Output != NULL ) && ( strcmp ( Map[Index]->Text, Output ) == 0 ) )
					{
					Tree->Selected = Map[Index];
					break;
					}
				}
			}
		cuiInternal_FireChange ( List );
		}
	free ( Output );
	Button = FindFirstButton ( Window );
	if ( Button != NULL )
		cuiInternal_FireClick ( Button );
	return cuiInternal_CountWindows () > 0;
	}

static bool ShowFormDialog ( cuiWidget *Window )
	{
	cuiWidget *Fields[64];
	unsigned FieldCount = 0;
	unsigned Index;
	cuiBuf Args = { 0 };
	char *Output;
	int ExitCode = 1;
	cuiWidget *Button;
	cuiWidget *List;

	List = FindListOrTree ( Window );
	if ( List != NULL )
		return ShowListDialog ( Window, List );

	CollectFields ( Window, Fields, &FieldCount, 64 );
	BufAppend ( &Args, "--forms --separator=" );
	{
	char Separator[] = { 0x1e, 0 };
	BufAppendQuoted ( &Args, Separator );
	}
	BufAppend ( &Args, " --title=" );
	BufAppendQuoted ( &Args, Window->Text );
	if ( ( Window->Text != NULL ) && ( Window->Text[0] != 0 ) )
		{
		BufAppend ( &Args, " --text=" );
		BufAppendQuoted ( &Args, Window->Text );
		}
	for ( Index = 0; Index < FieldCount; ++Index )
		{
		cuiWidget *Field = Fields[Index];
		if ( ( Field->Type == cuiType_TextBox ) || ( Field->Type == cuiType_TextArea ) )
			{
			BufAppend ( &Args, " --add-entry=" );
			BufAppendQuoted ( &Args, ( Field->Text[0] != 0 ) ? Field->Text : "Text" );
			}
		else if ( Field->Type == cuiType_CheckBox )
			{
			BufAppend ( &Args, " --add-combo=" );
			BufAppendQuoted ( &Args, Field->Text );
			BufAppend ( &Args, " --combo-values='No|Yes'" );
			}
		else if ( Field->Type == cuiType_ComboBox )
			{
			unsigned ItemIndex;
			BufAppend ( &Args, " --add-combo=" );
			BufAppendQuoted ( &Args, "Choice" );
			BufAppend ( &Args, " --combo-values=" );
			{
			cuiBuf Values = { 0 };
			for ( ItemIndex = 0; ItemIndex < Field->ItemCount; ++ItemIndex )
				{
				if ( ItemIndex > 0 )
					BufAppend ( &Values, "|" );
				BufAppend ( &Values, Field->Items[ItemIndex] );
				}
			BufAppendQuoted ( &Args, ( Values.Data != NULL ) ? Values.Data : "" );
			BufFree ( &Values );
			}
			}
		else if ( Field->Type == cuiType_Slider )
			{
			char Number[32];
			snprintf ( Number, sizeof ( Number ), "%d", Field->SliderValue );
			BufAppend ( &Args, " --add-entry=" );
			BufAppendQuoted ( &Args, Number );
			}
		}

	Output = RunZenity ( Args.Data, &ExitCode );
	BufFree ( &Args );
	if ( ExitCode != 0 )
		{
		free ( Output );
		cuiInternal_FireClose ( Window );
		cuiInternal_DestroyWidget ( Window );
		return false;
		}
	if ( Output != NULL )
		ApplyFormValues ( Fields, FieldCount, Output );
	free ( Output );
	Button = FindFirstButton ( Window );
	if ( Button != NULL )
		cuiInternal_FireClick ( Button );
	return cuiInternal_CountWindows () > 0;
	}

bool cuiBackend_Update ( bool Wait )
	{
	cuiWidget *Window;
	( void ) Wait;
	Window = cuiInternal_GetFirstWindow ();
	if ( Window == NULL )
		return false;
	return ShowFormDialog ( Window );
	}

cuiResult cuiBackend_MessageBox ( cuiWidget *Parent, const char *Title, const char *Message, cuiMessageType Type )
	{
	cuiBuf Args = { 0 };
	int ExitCode = 1;
	char *Output;
	( void ) Parent;

	switch ( Type )
		{
		case cuiMessage_Warning:
			BufAppend ( &Args, "--warning" );
			break;
		case cuiMessage_Error:
			BufAppend ( &Args, "--error" );
			break;
		case cuiMessage_Question:
			BufAppend ( &Args, "--question" );
			break;
		default:
			BufAppend ( &Args, "--info" );
			break;
		}
	BufAppend ( &Args, " --title=" );
	BufAppendQuoted ( &Args, ( Title != NULL ) ? Title : "" );
	BufAppend ( &Args, " --text=" );
	BufAppendQuoted ( &Args, ( Message != NULL ) ? Message : "" );
	Output = RunZenity ( Args.Data, &ExitCode );
	BufFree ( &Args );
	free ( Output );
	if ( Type == cuiMessage_Question )
		return ( ExitCode == 0 ) ? cuiResult_Yes : cuiResult_No;
	return ( ExitCode == 0 ) ? cuiResult_OK : cuiResult_Cancel;
	}

char *cuiBackend_FileDialog ( cuiWidget *Parent, const char *Title, const char *Filter, bool Save, bool Folder )
	{
	cuiBuf Args = { 0 };
	int ExitCode = 1;
	char *Output;
	( void ) Parent;
	BufAppend ( &Args, "--file-selection --title=" );
	BufAppendQuoted ( &Args, ( Title != NULL ) ? Title : "" );
	if ( Save )
		BufAppend ( &Args, " --save --confirm-overwrite" );
	if ( Folder )
		BufAppend ( &Args, " --directory" );
	if ( ( Filter != NULL ) && ( Filter[0] != 0 ) )
		{
		char *Copy = cuiInternal_StrDup ( Filter );
		char *SavePtr = NULL;
		char *Name = NULL;
		char *Token;
		for ( Token = strtok_r ( Copy, "|", &SavePtr ); Token != NULL; Token = strtok_r ( NULL, "|", &SavePtr ) )
			{
			if ( Name == NULL )
				Name = Token;
			else
				{
				cuiBuf FilterArg = { 0 };
				BufAppend ( &FilterArg, Name );
				BufAppend ( &FilterArg, " | " );
				BufAppend ( &FilterArg, Token );
				BufAppend ( &Args, " --file-filter=" );
				BufAppendQuoted ( &Args, FilterArg.Data );
				BufFree ( &FilterArg );
				Name = NULL;
				}
			}
		free ( Copy );
		}
	Output = RunZenity ( Args.Data, &ExitCode );
	BufFree ( &Args );
	if ( ExitCode != 0 )
		{
		free ( Output );
		return NULL;
		}
	return Output;
	}
