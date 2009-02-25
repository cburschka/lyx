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
#include "MetricsInfo.h"
#include "ParagraphParameters.h"
#include "Paragraph.h"
#include "TextClass.h"

#include "frontends/alert.h"
#include "frontends/Application.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

InsetERT::InsetERT(Buffer const & buf, CollapseStatus status)
	: InsetCollapsable(buf)
{
	status_ = status;
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


docstring InsetERT::editMessage() const
{
	return _("Opened ERT Inset");
}


int InsetERT::latex(odocstream & os, OutputParams const & op) const
{
	return InsetCollapsable::latex(os, op);
}


int InsetERT::plaintext(odocstream &, OutputParams const &) const
{
	return 0; // do not output TeX code
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
	BufferParams const & bp = cur.buffer()->params();
	Layout const layout = bp.documentClass().plainLayout();
	//lyxerr << "\nInsetERT::doDispatch (begin): cmd: " << cmd << endl;
	switch (cmd.action) {
	case LFUN_QUOTE_INSERT: {
		// We need to bypass the fancy quotes in Text
		FuncRequest f(LFUN_SELF_INSERT, "\"");
		dispatch(cur, f);
		break;
	}
	case LFUN_INSET_MODIFY: {
		setStatus(cur, string2params(to_utf8(cmd.argument())));
		break;
	}
	default:
		// Force any new text to latex_language
		// FIXME: This should not be necessary but
		// new paragraphs that are created by pressing enter at the
		// start of an existing paragraph get the buffer language
		// and not latex_language, so we take this brute force
		// approach.
		cur.current_font.fontInfo() = layout.font;
		cur.real_current_font.fontInfo() = layout.font;
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetERT::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
		case LFUN_CLIPBOARD_PASTE:
		case LFUN_INSET_MODIFY:
		case LFUN_PASTE:
		case LFUN_PRIMARY_SELECTION_PASTE:
		case LFUN_QUOTE_INSERT:
			status.setEnabled(true);
			return true;

		// this one is difficult to get right. As a half-baked
		// solution, we consider only the first action of the sequence
		case LFUN_COMMAND_SEQUENCE: {
			// argument contains ';'-terminated commands
			string const firstcmd = token(to_utf8(cmd.argument()), ';', 0);
			FuncRequest func(lyxaction.lookupFunc(firstcmd));
			func.origin = cmd.origin;
			return getStatus(cur, func, status);
		}

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


bool InsetERT::insetAllowed(InsetCode /* code */) const
{
	return false;
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


} // namespace lyx
