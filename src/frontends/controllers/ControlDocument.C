/**
 * \file ControlDocument.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlDocument.h"
#include "ViewBase.h"

#include "BranchList.h"
#include "buffer.h"
#include "bufferparams.h"
#include "funcrequest.h"
#include "language.h"
#include "LColor.h"
#include "lyxtextclasslist.h"

#include "frontends/LyXView.h"

#include "support/std_sstream.h"

using std::ostringstream;
using std::string;


ControlDocument::ControlDocument(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d), bp_(0)
{}


ControlDocument::~ControlDocument()
{}


void ControlDocument::setParams()
{
	if (!bp_.get())
		bp_.reset(new BufferParams);

	/// Set the buffer parameters
	*bp_ = buffer()->params();
}


BufferParams & ControlDocument::params()
{
	BOOST_ASSERT(bp_.get());
	return *bp_;
}


LyXTextClass ControlDocument::textClass()
{
	return textclasslist[bp_->textclass];
}


namespace {

void dispatch_params(LyXView & lv, BufferParams const & bp, kb_action lfun)
{
 	ostringstream ss;
 	bp.writeFile(ss);
 	ss << "\\end_header\n";
 	lv.dispatch(FuncRequest(lfun, ss.str()));
}

} // namespace anon


void ControlDocument::apply()
{
	if (!bufferIsAvailable())
		return;

	view().apply();

	// This must come first so that a language change is correctly noticed
	setLanguage();

	// Set the document class.
 	lyx::textclass_type const old_class = buffer()->params().textclass;
 	lyx::textclass_type const new_class = bp_->textclass;

 	if (new_class != old_class) {
 		string const name = textclasslist[new_class].name();
		lv_.dispatch(FuncRequest(LFUN_TEXTCLASS_APPLY, name));
	}

	// Apply the BufferParams.
 	dispatch_params(lv_, params(), LFUN_BUFFERPARAMS_APPLY);

	// Generate the colours requested by each new branch.
	BranchList & branchlist = params().branchlist();
	if (branchlist.empty())
		return;

	BranchList::const_iterator it = branchlist.begin();
	BranchList::const_iterator const end = branchlist.end();
	for (; it != end; ++it) {
		string const & current_branch = it->getBranch();
		Branch const * branch = branchlist.find(current_branch);
		string x11hexname = branch->getColor();
		// check that we have a valid color!
		if (x11hexname.empty() || x11hexname[0] != '#')
			x11hexname = lcolor.getX11Name(LColor::background);
		// display the new color
		string const str = current_branch  + ' ' + x11hexname;
		lv_.dispatch(FuncRequest(LFUN_SET_COLOR, str));
	}

	// Open insets of selected branches, close deselected ones
	lv_.dispatch(FuncRequest(LFUN_ALL_INSETS_TOGGLE, "toggle branch"));
}


void ControlDocument::setLanguage()
{
	Language const * const newL = bp_->language;
	if (buffer()->params().language == newL)
 		return;

 	string const lang_name = newL->lang();
 	lv_.dispatch(FuncRequest(LFUN_LANGUAGE_BUFFER, lang_name));
}


bool ControlDocument::loadTextclass(lyx::textclass_type tc) const
{
	string const name = textclasslist[tc].name();
	lv_.dispatch(FuncRequest(LFUN_TEXTCLASS_LOAD, name));

	// Report back whether we were able to change the class.
	bool const success = textclasslist[tc].loaded();
	return success;
}


void ControlDocument::saveAsDefault()
{
 	dispatch_params(lv_, params(), LFUN_SAVE_AS_DEFAULT);
}
