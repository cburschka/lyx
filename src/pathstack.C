// lyx-stack.C : implementation of PathStack class
//   this file is part of LyX, the High Level Word Processor
//   copyright (C) 1995-1996, Matthias Ettrich and the LyX Team

#include <config.h>
#include <unistd.h>

#ifdef __GNUG__
#pragma implementation "pathstack.h"
#endif

#include "pathstack.h"
#include "support/filetools.h"
#include "debug.h"
#include "LString.h"
#include "gettext.h"

// temporary hack
#include "lyx_gui_misc.h"

// global path stack
PathStack lyxPathStack;

// Standard constructor
PathStack::PathStack(string const & string)
	: Path(string)
{
	Next = 0;
}

// Destructor
PathStack::~PathStack()
{
	if (Next)
		delete Next;
}

// Changes to directory
int PathStack::PathPush(string const & Path)
{
	// checks path string validity
	if (Path.empty()) return 1;

	PathStack *NewNode;

	// gets current directory and switch to new one
	string CurrentPath = GetCWD();
	if ((CurrentPath.empty()) || chdir(Path.c_str())) {
		WriteFSAlert(_("Error: Could not change to directory: "), 
			     Path);
		return 2;
	}

	lyxerr.debug() << "PathPush: " << Path << endl;
	// adds new node
	NewNode = new PathStack(CurrentPath);
	NewNode->Next = Next;
	Next = NewNode;
	return 0;
}

// Goes back to previous directory
int PathStack::PathPop()
{
	// checks stack validity and extracts old node
	PathStack *OldNode = Next;
	if (!OldNode) {
		WriteAlert (_("LyX Internal Error:"), _("Path Stack underflow."));
		return 1;
	}
	Next = OldNode->Next;
	OldNode->Next = 0;

	// switches to old directory
	int Result = 0;
	if (chdir(OldNode->Path.c_str())) {
		WriteFSAlert(_("Error: Could not change to directory: "), 
			     Path);
		Result = 2;
	}
	lyxerr.debug() << "PathPop: " << OldNode->Path << endl;
	delete OldNode;

	return Result;
}

