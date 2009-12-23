/**
 * \file InsetERT.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetERT.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Language.h"
#include "Layout.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "LyXAction.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "ParagraphParameters.h"
#include "Paragraph.h"
#include "TextClass.h"

#include "graphics/PreviewImage.h"

#include "insets/RenderPreview.h"

#include "frontends/alert.h"
#include "frontends/Application.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

InsetERT::InsetERT(Buffer * buf, CollapseStatus status)
	: InsetCollapsable(buf), preview_(new RenderPreview(this))
{
	status_ = status;
}


InsetERT::InsetERT(InsetERT const & other) : InsetCollapsable(other)
{
	operator=(other);
}

InsetERT & InsetERT::operator=(InsetERT const & other)
{
	if (this == &other)
		return *this;
	InsetCollapsable::operator=(other);
	buffer_ = other.buffer_;
	preview_.reset(new RenderPreview(*other.preview_, this));

	return *this;
}


InsetERT::~InsetERT()
{
	hideDialogs("ert", this);
}


void InsetERT::write(ostream & os) const
{
	os << "ERT" << "\n";
	InsetCollapsable::write(os);
}


int InsetERT::plaintext(odocstream & os, OutputParams const & rp) const
{
	if (!rp.inIndexEntry)
		// do not output TeX code
		return 0;

	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	while (par != end) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			char_type const c = par->getChar(i);
			// output the active characters
			switch (c) {
			case '|':
			case '!':
			case '@':
				os.put(c);
				break;
			default:
				break;
			}
		}
		++par;
	}
	return 0;
}


int InsetERT::docbook(odocstream & os, OutputParams const &) const
{
	// FIXME can we do the same thing here as for LaTeX?
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	int lines = 0;
	while (par != end) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i)
			os.put(par->getChar(i));
		++par;
		if (par != end) {
			os << "\n";
			++lines;
		}
	}

	return lines;
}


void InsetERT::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		setStatus(cur, string2params(to_utf8(cmd.argument())));
		break;
	}
	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetERT::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY:
		status.setEnabled(true);
		return true;
		
	default:
		return InsetCollapsable::getStatus(cur, cmd, status);
	}
}


docstring const InsetERT::buttonLabel(BufferView const & bv) const
{
	if (decoration() == InsetLayout::CLASSIC)
		return isOpen(bv) ? _("ERT") : getNewLabel(_("ERT"));
	else
		return getNewLabel(_("ERT"));
}


bool InsetERT::showInsetDialog(BufferView * bv) const
{
	bv->showDialog("ert", params2string(status(*bv)), 
		const_cast<InsetERT *>(this));
	return true;
}


InsetCollapsable::CollapseStatus InsetERT::string2params(string const & in)
{
	if (in.empty())
		return Collapsed;
	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetERT::string2params");
	lex >> "ert";
	int s;
	lex >> s;
	return static_cast<CollapseStatus>(s);
}


string InsetERT::params2string(CollapseStatus status)
{
	ostringstream data;
	data << "ert" << ' ' << status;
	return data.str();
}


docstring InsetERT::xhtml(XHTMLStream &, OutputParams const &) const
{
	return docstring();
}


bool InsetERT::previewState(BufferView * bv) const
{
	if (!editing(bv) && RenderPreview::status() == LyXRC::PREVIEW_ON) {
		graphics::PreviewImage const * pimage =
			preview_->getPreviewImage(bv->buffer());
		return pimage && pimage->image();
	}
	return false;
}


void InsetERT::addPreview(DocIterator const & inset_pos,
	graphics::PreviewLoader & ploader) const
{
	preparePreview(inset_pos);
}


void InsetERT::preparePreview(DocIterator const & pos) const  
{
	odocstringstream str;  
	OutputParams runparams(&pos.buffer()->params().encoding());
	latex(str, runparams);
	docstring const snippet = str.str();
	LYXERR(Debug::MACROS, "Preview snippet: " << snippet);  
	preview_->addPreview(snippet, *pos.buffer());  
}


void InsetERT::reloadPreview(DocIterator const & pos) const
{
	preparePreview(pos);
	preview_->startLoading(*pos.buffer());
}


bool InsetERT::notifyCursorLeaves(Cursor const & old, Cursor & cur)
{
	reloadPreview(old);
	cur.updateFlags(Update::Force);
	return InsetCollapsable::notifyCursorLeaves(old, cur);
}


void InsetERT::draw(PainterInfo & pi, int x, int y) const
{
	use_preview_ = previewState(pi.base.bv);

	if (use_preview_) {
		// one pixel gap in front
		preview_->draw(pi, x + 1, y);
		setPosCache(pi, x, y);
		return;
	}
	InsetCollapsable::draw(pi, x, y);
}


Inset * InsetERT::editXY(Cursor & cur, int x, int y)
{
	if (use_preview_) {
		edit(cur, true);
		return this;
	}
	return InsetCollapsable::editXY(cur, x, y);
}


void InsetERT::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (previewState(mi.base.bv)) {
		preview_->metrics(mi, dim);
		// insert a one pixel gap in front of the formula
		dim.wid += 1;
		// Cache the inset dimension.
		setDimCache(mi, dim);
		Dimension dim_dummy = dim;
		MetricsInfo mi_dummy = mi;
		InsetCollapsable::metrics(mi_dummy, dim_dummy);
		return;
	}
	InsetCollapsable::metrics(mi, dim);
}

} // namespace lyx
