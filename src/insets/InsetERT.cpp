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
#include "LyXAction.h"
#include "OutputParams.h"
#include "ParagraphParameters.h"
#include "Paragraph.h"
#include "TextClass.h"

#include "support/gettext.h"
#include "support/lstrings.h"

#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

InsetERT::InsetERT(Buffer * buf, CollapseStatus status)
	: InsetCollapsable(buf)
{
	status_ = status;
}


void InsetERT::write(ostream & os) const
{
	os << "ERT" << "\n";
	InsetCollapsable::write(os);
}


int InsetERT::plaintext(odocstringstream & os,
        OutputParams const & rp, size_t max_length) const
{
	if (!rp.inIndexEntry)
		// do not output TeX code
		return 0;

	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	while (par != end && os.str().size() <= max_length) {
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
	switch (cmd.action()) {
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "ert") {
			cur.recordUndoInset(this);
			setStatus(cur, string2params(to_utf8(cmd.argument())));
			break;
		}
		//fall-through
	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}

}


bool InsetERT::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action()) {
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "ert") {
			status.setEnabled(true);
			return true;
		}
		//fall through

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

} // namespace lyx
