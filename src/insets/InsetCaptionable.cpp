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

#include "InsetCaption.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "FloatList.h"
#include "InsetList.h"
#include "output_xhtml.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "support/docstream.h"

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
	return (it == floats.end()) ? from_utf8(type) : bp.B_(it->second.name());
}


InsetCaption const * InsetCaptionable::getCaptionInset() const
{
	ParagraphList::const_iterator pit = paragraphs().begin();
	for (; pit != paragraphs().end(); ++pit) {
		InsetList::const_iterator it = pit->insetList().begin();
		for (; it != pit->insetList().end(); ++it) {
			Inset & inset = *it->inset;
			if (inset.lyxCode() == CAPTION_CODE) {
				InsetCaption const * ins =
					static_cast<InsetCaption const *>(it->inset);
				return ins;
			}
		}
	}
	return 0;
}


docstring InsetCaptionable::getCaptionText(OutputParams const & runparams) const
{
	InsetCaption const * ins = getCaptionInset();
	if (ins == 0)
		return docstring();

	odocstringstream ods;
	ins->getCaptionAsPlaintext(ods, runparams);
	return ods.str();
}


docstring InsetCaptionable::getCaptionHTML(OutputParams const & runparams) const
{
	InsetCaption const * ins = getCaptionInset();
	if (ins == 0)
		return docstring();

	odocstringstream ods;
	XMLStream xs(ods);
	docstring def = ins->getCaptionAsHTML(xs, runparams);
	if (!def.empty())
		// should already have been escaped
		xs << XMLStream::ESCAPE_NONE << def << '\n';
	return ods.str();
}


void InsetCaptionable::addToToc(DocIterator const & cpit, bool output_active,
								UpdateType utype, TocBackend & backend) const
{
	DocIterator pit = cpit;
	pit.push_back(CursorSlice(const_cast<InsetCaptionable &>(*this)));
	docstring str;
	// Leave str empty if we generate for output (e.g. xhtml lists of figures).
	// This ensures that there is a caption if and only if the string is
	// non-empty.
	if (utype != OutputUpdate)
		text().forOutliner(str, TOC_ENTRY_LENGTH);
	TocBuilder & b = backend.builder(caption_type_);
	b.pushItem(pit, str, output_active);
	// Proceed with the rest of the inset.
	InsetCollapsible::addToToc(cpit, output_active, utype, backend);
	b.pop();
}

void InsetCaptionable::updateBuffer(ParIterator const & it, UpdateType utype, bool const deleted)
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
	InsetCollapsible::updateBuffer(it, utype, deleted);
	// Restore counters
	cnts.current_float(saveflt);
	if (utype == OutputUpdate)
		cnts.restoreLastCounter();
	cnts.isSubfloat(savesubflt);
}


bool InsetCaptionable::insetAllowed(InsetCode c) const
{
	return (c == CAPTION_CODE) || InsetCollapsible::insetAllowed(c);
}


} // namespace lyx
