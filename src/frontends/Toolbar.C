/**
 * \file Toolbar.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "Toolbar.h"
#include "ToolbarBackend.h"
#include "debug.h"
#include "LyXAction.h"

using std::endl;

Toolbar::Toolbar()
	: last_textclass_(-1)
{
}


Toolbar::~Toolbar()
{
}


void Toolbar::init() 
{
	// extracts the toolbars from the backend
	ToolbarBackend::Toolbars::const_iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::const_iterator end = toolbarbackend.end();

	for (; cit != end; ++cit)
		add(*cit);
}


void Toolbar::update(bool in_math, bool in_table)
{
	update();

	// extracts the toolbars from the backend
	ToolbarBackend::Toolbars::const_iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::const_iterator end = toolbarbackend.end();

	for (; cit != end; ++cit) {
		if (cit->flags & ToolbarBackend::MATH)
			displayToolbar(*cit, in_math);
		else if (cit->flags & ToolbarBackend::TABLE)
			displayToolbar(*cit, in_table);
	}
}


bool Toolbar::updateLayoutList(int textclass)
{
	// update the layout display
	if (last_textclass_ != textclass) {
		updateLayoutList();
		last_textclass_ = textclass;
		return true;
	} else
		return false;
}
