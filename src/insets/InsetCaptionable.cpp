/**
 * \file InsetCaptionable.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetCaptionable.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "FloatList.h"
#include "TextClass.h"
#include "TocBackend.h"

using namespace std;


namespace lyx {


void InsetCaptionable::setCaptionType(std::string const & type)
{
	caption_type_ = type.empty() ? "senseless" : type;
}


/// common to InsetFloat and InsetWrap
docstring InsetCaptionable::floatName(string const & type) const
{
	BufferParams const & bp = buffer().params();
	FloatList const & floats = bp.documentClass().floats();
	FloatList::const_iterator it = floats[type];
	// FIXME UNICODE
	return (it == floats.end()) ? from_ascii(type) : bp.B_(it->second.name());
}


void InsetCaptionable::addToToc(DocIterator const & cpit, bool output_active) const
{
	DocIterator pit = cpit;
	pit.push_back(CursorSlice(const_cast<InsetCaptionable &>(*this)));
	docstring str;
	int length = output_active ? INT_MAX : TOC_ENTRY_LENGTH;
	text().forOutliner(str, length);
	shared_ptr<TocBuilder> b = buffer().tocBackend().builder(caption_type_);
	b->pushItem(pit, str, output_active);
	// Proceed with the rest of the inset.
	InsetCollapsable::addToToc(cpit, output_active);
	b->pop();
}

void InsetCaptionable::updateBuffer(ParIterator const & it, UpdateType utype)
{
	Counters & cnts =
		buffer().masterBuffer()->params().documentClass().counters();
	string const saveflt = cnts.current_float();
	bool const savesubflt = cnts.isSubfloat();
	if (utype == OutputUpdate) {
		// counters are local to the float
		cnts.saveLastCounter();
	}
	bool const subflt = hasSubCaptions(it);
	// floats can only embed subfloats of their own kind
	if (subflt && !saveflt.empty() && saveflt != "senseless")
		setCaptionType(saveflt);
	// Tell captions what the current float is
	cnts.current_float(caption_type_);
	cnts.isSubfloat(subflt);
	InsetCollapsable::updateBuffer(it, utype);
	// Restore counters
	cnts.current_float(saveflt);
	if (utype == OutputUpdate)
		cnts.restoreLastCounter();
	cnts.isSubfloat(savesubflt);
}


bool InsetCaptionable::insetAllowed(InsetCode c) const
{
	return (c == CAPTION_CODE) || InsetCollapsable::insetAllowed(c);
}


} // namespace lyx
