// -*- C++ -*-
/**
 * \file ControlDocument.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "BufferView.h"
#include "ControlDocument.h"
#include "ViewBase.h"

#include "gettext.h"
#include "lyxfind.h"

#include "buffer.h"
#include "language.h"
#include "lyx_main.h"
#include "lyxtextclass.h"
#include "lyxtextclasslist.h"
#include "CutAndPaste.h"

#include "frontends/LyXView.h"
#include "frontends/Alert.h"

#include "support/lstrings.h"
#include "support/filetools.h"


ControlDocument::ControlDocument(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d), bp_(0)
{
}

ControlDocument::~ControlDocument()
{}


void ControlDocument::showPreamble()
{
}


BufferParams & ControlDocument::params()
{
	lyx::Assert(bp_.get());
	return *bp_;
}


LyXTextClass ControlDocument::textClass()
{
	return textclasslist[bp_->textclass];
}


void ControlDocument::apply()
{
	if (!bufferIsAvailable())
		return;

	setLanguage();

	// FIXME: do we need to use return value from classApply() here? (Lgb)
	classApply();
	lv_.view()->redoCurrentBuffer();

	view().apply();
	buffer()->params = *bp_;

	buffer()->markDirty();

	lv_.message(_("Document Settings Applied"));
}


void ControlDocument::setParams()
{
	if (!bp_.get())
		bp_.reset(new BufferParams());

	/// Set the buffer parameters
	*bp_ = buffer()->params;
}


void ControlDocument::setLanguage()
{
	Language const * oldL = buffer()->params.language;
	Language const * newL = bp_->language;

	if (oldL != newL
	    && oldL->RightToLeft() == newL->RightToLeft()
	    && !lv_.buffer()->isMultiLingual())
		lv_.buffer()->changeLanguage(oldL, newL);
}


bool ControlDocument::classApply()
{
	BufferParams & params = lv_.buffer()->params;
	unsigned int const old_class = bp_->textclass;
	// exit if nothing changes
	if (params.textclass == old_class)
		return true;

	// try to load new_class
	if (!textclasslist[params.textclass].load()) {
		// problem changing class
		// -- warn user (to retain old style)
		Alert::alert(_("Conversion Errors!"),
			     _("Errors loading new document class."),
			     _("Reverting to original document class."));
		return false;
	}

	// successfully loaded
	lv_.message(_("Converting document to new document class..."));
	int ret = CutAndPaste::SwitchLayoutsBetweenClasses(
		old_class, params.textclass,
		&*(lv_.buffer()->paragraphs.begin()),
		lv_.buffer()->params);
	if (ret) {
		string s;
		if (ret == 1) {
			s = _("One paragraph couldn't be converted");
		} else {
			s += tostr(ret);
			s += _(" paragraphs couldn't be converted");
		}
		Alert::alert(_("Conversion Errors!"),s,
			     _("into chosen document class"));
	}
	//lv_.view()->redoCurrentBuffer();
	return true;
}


void ControlDocument::saveAsDefault()
{
	lv_.buffer()->params.preamble = bp_->preamble;

	string const fname = AddName(AddPath(user_lyxdir, "templates/"),
				     "defaults.lyx");
	Buffer defaults(fname);
	defaults.params = params();

	// add an empty paragraph. Is this enough?
	Paragraph * par = new Paragraph;
	par->layout(params().getLyXTextClass().defaultLayout());
	defaults.paragraphs.set(par);

	defaults.writeFile(defaults.fileName());

}
