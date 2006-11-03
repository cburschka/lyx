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
#include "Kernel.h"

#include "BranchList.h"
#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "buffer_funcs.h"
#include "funcrequest.h"
#include "gettext.h"
#include "language.h"
#include "LaTeXFeatures.h"
#include "LColor.h"
#include "outputparams.h"
#include "lyxtextclasslist.h"
#include "tex-strings.h"

#include <sstream>

using std::ostringstream;
using std::string;

namespace lyx {
namespace frontend {

char const * const ControlDocument::fontfamilies[5] = {
	"default", "rmdefault", "sfdefault", "ttdefault", ""
};


char const * ControlDocument::fontfamilies_gui[5] = {
	N_("Default"), N_("Roman"), N_("Sans Serif"), N_("Typewriter"), ""
};


ControlDocument::ControlDocument(Dialog & parent)
	: Dialog::Controller(parent)
{}


ControlDocument::~ControlDocument()
{}


bool ControlDocument::initialiseParams(std::string const &)
{
	bp_.reset(new BufferParams);
	*bp_ = kernel().buffer().params();
	return true;
}


void ControlDocument::clearParams()
{
	bp_.reset();
}


BufferParams & ControlDocument::params() const
{
	BOOST_ASSERT(bp_.get());
	return *bp_;
}


LyXTextClass const & ControlDocument::textClass() const
{
	return textclasslist[bp_->textclass];
}


namespace {

void dispatch_bufferparams(Kernel const & kernel, BufferParams const & bp,
			   kb_action lfun)
{
	ostringstream ss;
	ss << "\\begin_header\n";
	bp.writeFile(ss);
	ss << "\\end_header\n";
	kernel.dispatch(FuncRequest(lfun, ss.str()));
}

} // namespace anon


void ControlDocument::dispatchParams()
{
	// This must come first so that a language change is correctly noticed
	setLanguage();

	// Set the document class.
	textclass_type const old_class =
		kernel().buffer().params().textclass;
	textclass_type const new_class = bp_->textclass;
	if (new_class != old_class) {
		string const name = textclasslist[new_class].name();
		kernel().dispatch(FuncRequest(LFUN_TEXTCLASS_APPLY, name));
	}

	int const old_secnumdepth = kernel().buffer().params().secnumdepth;
	int const new_secnumdepth = bp_->secnumdepth;

	// Apply the BufferParams.
	dispatch_bufferparams(kernel(), params(), LFUN_BUFFER_PARAMS_APPLY);
	
	// redo the numbering if necessary
	if (new_secnumdepth != old_secnumdepth)
		updateLabels(kernel().buffer());

	// Generate the colours requested by each new branch.
	BranchList & branchlist = params().branchlist();
	if (!branchlist.empty()) {
		BranchList::const_iterator it = branchlist.begin();
		BranchList::const_iterator const end = branchlist.end();
		for (; it != end; ++it) {
			docstring const & current_branch = it->getBranch();
			Branch const * branch = branchlist.find(current_branch);
			string const x11hexname =
					lyx::X11hexname(branch->getColor());
			// display the new color
			// FIXME UNICODE
			string const str = to_utf8(current_branch) + ' ' + x11hexname;
			kernel().dispatch(FuncRequest(LFUN_SET_COLOR, str));
		}

		// Open insets of selected branches, close deselected ones
		kernel().dispatch(FuncRequest(LFUN_ALL_INSETS_TOGGLE,
			"assign branch"));
	}
	// update the bufferview
	// If we used an LFUN, we would not need that
	kernel().bufferview()->update();
}


void ControlDocument::setLanguage() const
{
	Language const * const newL = bp_->language;
	if (kernel().buffer().params().language == newL)
		return;

	string const lang_name = newL->lang();
	kernel().dispatch(FuncRequest(LFUN_BUFFER_LANGUAGE, lang_name));
}


bool ControlDocument::loadTextclass(textclass_type tc) const
{
	string const name = textclasslist[tc].name();
	kernel().dispatch(FuncRequest(LFUN_TEXTCLASS_LOAD, name));

	// Report back whether we were able to change the class.
	bool const success = textclasslist[tc].loaded();
	return success;
}


void ControlDocument::saveAsDefault() const
{
	dispatch_bufferparams(kernel(), params(), LFUN_BUFFER_SAVE_AS_DEFAULT);
}


bool const ControlDocument::isFontAvailable(std::string const & font) const
{
	OutputParams runparams;
	LaTeXFeatures features(kernel().buffer(), kernel().buffer().params(), runparams);
	if (font == "default" || font == "cmr" 
	    || font == "cmss" || font == "cmtt")
		// these are standard
		return true;
	else if (font == "lmodern" || font == "lmss" || font == "lmtt")
		return features.isAvailable("lmodern");
	else if (font == "times" || font == "palatino" 
		 || font == "helvet" || font == "courier")
		return (features.isAvailable("psnfss"));
	else if (font == "cmbr" || font == "cmtl")
		return features.isAvailable("cmbright");
	else if (font == "utopia")
		return (features.isAvailable("utopia")
			|| features.isAvailable("fourier"));
	else if (font == "beraserif" || font == "berasans" 
		|| font == "beramono")
		return features.isAvailable("bera");
	else
		return features.isAvailable(font);
}


bool const ControlDocument::providesOSF(std::string const & font) const
{
	if (font == "cmr")
		return isFontAvailable("eco");
	else if (font == "palatino")
		return isFontAvailable("mathpazo");
	else
		return false;
}


bool const ControlDocument::providesSC(std::string const & font) const
{
	if (font == "palatino")
		return isFontAvailable("mathpazo");
	else if (font == "utopia")
		return isFontAvailable("fourier");
	else
		return false;
}


bool const ControlDocument::providesScale(std::string const & font) const
{
	return (font == "helvet" || font == "luximono"
		|| font == "berasans"  || font == "beramono");
}


} // namespace frontend
} // namespace lyx
