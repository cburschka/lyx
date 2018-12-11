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
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "Format.h"
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

#include "support/docstream.h"
#include "support/FileName.h"
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


// Do not copy the temp file on purpose: If a copy of an inset which is
// currently being edited is made, then we simply copy the current contents.
InsetERT::InsetERT(InsetERT const & that) : InsetCollapsible(that)
{}


InsetERT & InsetERT::operator=(InsetERT const & that)
{
	if (&that == this)
		return *this;
	tempfile_.reset();
	return *this;
}


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
	case LFUN_INSET_EDIT: {
		cur.push(*this);
		text().selectAll(cur);
		string const format =
			cur.buffer()->params().documentClass().outputFormat();
		string const ext = theFormats().extension(format);
		tempfile_.reset(new TempFile("ert_editXXXXXX." + ext));
		FileName const tempfilename = tempfile_->name();
		string const name = tempfilename.toFilesystemEncoding();
		ofdocstream os(name.c_str());
		os << cur.selectionAsString(false);
		os.close();
		// Since we lock the inset while the external file is edited,
		// we need to move the cursor outside and clear any selection inside
		cur.clearSelection();
		cur.pop();
		cur.leaveInset(*this);
		theFormats().edit(buffer(), tempfilename, format);
		break;
	}
	case LFUN_INSET_END_EDIT: {
		FileName const tempfilename = tempfile_->name();
		docstring const s = tempfilename.fileContents("UTF-8");
		cur.recordUndoInset(this);
		cur.push(*this);
		text().selectAll(cur);
		cap::replaceSelection(cur);
		cur.text()->insertStringAsLines(cur, s, cur.current_font);
		// FIXME it crashes without this
		cur.fixIfBroken();
		tempfile_.reset();
		cur.pop();
		break;
	}
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
	case LFUN_INSET_EDIT:
		status.setEnabled(tempfile_ == 0);
		return true;
	case LFUN_INSET_END_EDIT:
		status.setEnabled(tempfile_ != 0);
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


bool InsetERT::editable() const
{
	if (tempfile_)
		return false;
	return InsetCollapsible::editable();
}


bool InsetERT::descendable(BufferView const & bv) const
{
	if (tempfile_)
		return false;
	return InsetCollapsible::descendable(bv);
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


docstring InsetERT::xhtml(XHTMLStream &, OutputParams const &) const
{
	return docstring();
}

} // namespace lyx
