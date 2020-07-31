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
#include "xml.h"
#include "ParagraphParameters.h"
#include "Paragraph.h"
#include <output_docbook.h>

#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/TempFile.h"

#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

InsetERT::InsetERT(Buffer * buf, CollapseStatus status)
	: InsetCollapsible(buf)
{
	status_ = status;
}


InsetERT::InsetERT(InsetERT const & old)
	: InsetCollapsible(old)
{}


void InsetERT::write(ostream & os) const
{
	os << "ERT" << "\n";
	InsetCollapsible::write(os);
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


void InsetERT::docbook(XMLStream & xs, OutputParams const & runparams) const
{
	auto const begin = paragraphs().begin();
	auto par = begin;
	auto const end = paragraphs().end();

	odocstringstream os2;
	XMLStream xs2(os2);

	// Recreate the logic of makeParagraphs in output_docbook.cpp, but much simplified: never open <para>
	// in an ERT, use simple line breaks.
	while (par != end) {
		par->simpleDocBookOnePar(buffer(), xs2, runparams, text().outerFont(distance(begin, par)));

		// New line after each paragraph of the ERT, save the last one.
		++par;
		if (par != end)
			xs << "\n";
	}

	// Output the ERT as a comment with the appropriate escaping.
	xs << XMLStream::ESCAPE_NONE << "<!-- ";
	xs << XMLStream::ESCAPE_COMMENTS << os2.str();
	xs << XMLStream::ESCAPE_NONE << " -->";
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
		InsetCollapsible::doDispatch(cur, cmd);
		break;
	}

}


bool InsetERT::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action()) {
	case LFUN_INSET_INSERT:
		status.setEnabled(false);
		return true;
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "ert") {
			status.setEnabled(true);
			return true;
		}
		//fall through

	default:
		return InsetCollapsible::getStatus(cur, cmd, status);
	}
}



docstring const InsetERT::buttonLabel(BufferView const & bv) const
{
	if (decoration() == InsetLayout::CLASSIC)
		return isOpen(bv) ? _("ERT") : getNewLabel(_("ERT"));
	else
		return getNewLabel(_("ERT"));
}


InsetCollapsible::CollapseStatus InsetERT::string2params(string const & in)
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


docstring InsetERT::xhtml(XMLStream &, OutputParams const &) const
{
	return docstring();
}

} // namespace lyx
