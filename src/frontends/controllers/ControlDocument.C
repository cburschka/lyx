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

#include "BufferView.h"
#include "ControlDocument.h"
#include "ViewBase.h"

#include "gettext.h"
#include "lyxfind.h"

#include "buffer.h"
#include "buffer_funcs.h"
#include "errorlist.h"
#include "language.h"
#include "lyx_main.h"
#include "lyxtextclass.h"
#include "lyxtextclasslist.h"
#include "CutAndPaste.h"

#include "frontends/LyXView.h"
#include "frontends/Alert.h"

#include "support/LAssert.h"
#include "support/lstrings.h"
#include "support/filetools.h"
#include "support/path_defines.h"

using namespace lyx::support;

using std::endl;


ControlDocument::ControlDocument(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d), bp_(0)
{}


ControlDocument::~ControlDocument()
{}


BufferParams & ControlDocument::params()
{
	Assert(bp_.get());
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

	view().apply();

	// this must come first so that a language change
	// is correctly noticed
	setLanguage();

	classApply();

	buffer()->params = *bp_;

	lv_.view()->redoCurrentBuffer();

	buffer()->markDirty();

	lv_.message(_("Document settings applied"));
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

	if (oldL != newL) {

		if (oldL->RightToLeft() == newL->RightToLeft()
		    && !lv_.buffer()->isMultiLingual())
			lv_.buffer()->changeLanguage(oldL, newL);
		else
		    lv_.buffer()->updateDocLang(newL);
	}
}


void ControlDocument::classApply()
{
	BufferParams & params = buffer()->params;
	lyx::textclass_type const old_class = params.textclass;
	lyx::textclass_type const new_class = bp_->textclass;

	// exit if nothing changes or if unable to load the new class
	if (new_class == old_class || !loadTextclass(new_class))
		return;

	// successfully loaded
	buffer()->params = *bp_;

	lv_.message(_("Converting document to new document class..."));

	ErrorList el;
	CutAndPaste::SwitchLayoutsBetweenClasses(old_class, new_class,
						 lv_.buffer()->paragraphs,
						 el);
	bufferErrors(*buffer(), el);
	bufferview()->showErrorList(_("Class switch"));
}


bool ControlDocument::loadTextclass(lyx::textclass_type tc) const
{
	bool const success = textclasslist[tc].load();
	if (success)
		return success;

	string s = bformat(_("The document could not be converted\n"
			"into the document class %1$s."),
			textclasslist[tc].name());
	Alert::error(_("Could not change class"), s);

	return success;
}


void ControlDocument::saveAsDefault()
{
// Can somebody justify this ? I think it should be removed - jbl
#if 0
	if (!Alert::askQuestion(_("Do you want to save the current settings"),
				_("for the document layout as default?"),
				_("(they will be valid for any new document)")))
		return;
#endif

	lv_.buffer()->params.preamble = bp_->preamble;

	string const fname = AddName(AddPath(user_lyxdir(), "templates/"),
				     "defaults.lyx");
	Buffer defaults(fname);
	defaults.params = params();

	// add an empty paragraph. Is this enough?
	Paragraph par;
	par.layout(params().getLyXTextClass().defaultLayout());
	defaults.paragraphs.push_back(par);

	defaults.writeFile(defaults.fileName());

}
