/**
 * \file ControlDocument.cpp
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
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "buffer_funcs.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Color.h"
#include "OutputParams.h"
#include "TextClassList.h"

// FIXME: those two headers are needed because of the
// WorkArea::redraw() call below.
#include "frontends/LyXView.h"
#include "frontends/WorkArea.h"

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
	: Controller(parent)
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


BufferId ControlDocument::id() const
{
	return &kernel().buffer();
}


TextClass const & ControlDocument::textClass() const
{
	return textclasslist[bp_->getBaseClass()];
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

	// Apply the BufferParams. Note that this will set the base class
	// and then update the buffer's layout.
	//FIXME Could this be done last? Then, I think, we'd get the automatic
	//update mentioned in the next FIXME...
	dispatch_bufferparams(kernel(), params(), LFUN_BUFFER_PARAMS_APPLY);

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
			docstring const str = current_branch + ' ' + from_ascii(x11hexname);
			kernel().dispatch(FuncRequest(LFUN_SET_COLOR, str));
		}

		// Open insets of selected branches, close deselected ones
		kernel().dispatch(FuncRequest(LFUN_ALL_INSETS_TOGGLE,
			"assign branch"));
	}
	// FIXME: If we used an LFUN, we would not need those two lines:
	kernel().bufferview()->update();
	kernel().lyxview().currentWorkArea()->redraw();
}


void ControlDocument::setLanguage() const
{
	Language const * const newL = bp_->language;
	if (kernel().buffer().params().language == newL)
		return;

	string const lang_name = newL->lang();
	kernel().dispatch(FuncRequest(LFUN_BUFFER_LANGUAGE, lang_name));
}


void ControlDocument::saveAsDefault() const
{
	dispatch_bufferparams(kernel(), params(), LFUN_BUFFER_SAVE_AS_DEFAULT);
}


bool const ControlDocument::isFontAvailable(std::string const & font) const
{
	if (font == "default" || font == "cmr"
	    || font == "cmss" || font == "cmtt")
		// these are standard
		return true;
	else if (font == "lmodern" || font == "lmss" || font == "lmtt")
		return LaTeXFeatures::isAvailable("lmodern");
	else if (font == "times" || font == "palatino"
		 || font == "helvet" || font == "courier")
		return LaTeXFeatures::isAvailable("psnfss");
	else if (font == "cmbr" || font == "cmtl")
		return LaTeXFeatures::isAvailable("cmbright");
	else if (font == "utopia")
		return LaTeXFeatures::isAvailable("utopia")
			|| LaTeXFeatures::isAvailable("fourier");
	else if (font == "beraserif" || font == "berasans"
		|| font == "beramono")
		return LaTeXFeatures::isAvailable("bera");
	else
		return LaTeXFeatures::isAvailable(font);
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
