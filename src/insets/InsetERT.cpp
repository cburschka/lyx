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

#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetLayout.h"
#include "Language.h"
#include "Lexer.h"
#include "xml.h"
#include "ParagraphParameters.h"
#include "Paragraph.h"
#include "output_docbook.h"

#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/TempFile.h"
#include "Encoding.h"

#include <sstream>
#include <regex>
#include <iostream>

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

	odocstringstream os; // No need for XML handling here.

	// Recreate the logic of makeParagraph in output_docbook.cpp, but much simplified: never open <para>
	// in an ERT, use simple line breaks.
	// New line after each paragraph of the ERT, save the last one.
	while (true) { // For each paragraph in the ERT...
        std::vector<docstring> pars_prepend;
        std::vector<docstring> pars;
        std::vector<docstring> pars_append;
        tie(pars_prepend, pars, pars_append) = par->simpleDocBookOnePar(buffer(), runparams, text().outerFont(distance(begin, par)), 0, false, true);

        for (docstring const & parXML : pars_prepend)
            xs << XMLStream::ESCAPE_NONE << parXML;
		auto p = pars.begin();
		while (true) { // For each line of this ERT paragraph...
			os << *p;
			++p;
			if (p != pars.end())
				os << "\n";
			else
				break;
		}
        for (docstring const & parXML : pars_append)
            xs << XMLStream::ESCAPE_NONE << parXML;

		++par;
		if (par != end)
			os << "\n";
		else
			break;
	}

//	// Implement the special case of \and: split the current item.
//	if (os.str() == "\\and" || os.str() == "\\and ") {
//		auto lay = getLayout();
//	}

	// Try to recognise some commands to have a nicer DocBook output.
	bool output_as_comment = true;

	// First step: some commands have a direct mapping to DocBook, mostly because the mapping is simply text or
	// an XML entity.
	// Logic is similar to that of convertLaTeXCommands in BiblioInfo.cpp.
	// TODO: make the code even more similar by looping over the string and applying all conversions. (What is not
	//  recognised should simply be put in comments: have a list of elements that are either already recognised or are
	//  not yet recognised? Global transformations like \string should then come first.)
	{
		docstring os_trimmed = trim(os.str());

		// Rewrite \"u to \"{u}.
		static regex const regNoBraces(R"(^\\\W\w)");
		if (regex_search(to_utf8(os_trimmed), regNoBraces)) {
			os_trimmed.insert(3, from_ascii("}"));
			os_trimmed.insert(2, from_ascii("{"));
		}

		// Rewrite \" u to \"{u}.
		static regex const regSpace(R"(^\\\W \w)");
		if (regex_search(to_utf8(os_trimmed), regSpace)) {
			os_trimmed[2] = '{';
			os_trimmed.insert(4, from_ascii("}"));
		}

		// Look into the global table of Unicode characters if there is a match.
		bool termination;
		docstring rem;
		docstring const converted = Encodings::fromLaTeXCommand(os_trimmed,
		                                                        Encodings::TEXT_CMD, termination, rem);
		if (!converted.empty()) {
			// Don't output the characters directly, even if the document should be encoded in UTF-8, for editors that
			// do not support all these funky characters.
			for (const char_type& character : converted) {
				xs << XMLStream::ESCAPE_NONE << from_ascii("&#" + std::to_string(character) + ';');
			}
			output_as_comment = false;
		}
	}

	// Second step: the command \string can be ignored. If that's the only command in the ERT, then done.
	// There may be several occurrences. (\string is 7 characters long.)
	if (os.str().length() >= 7) {
		docstring os_str = os.str();

		while (os_str.length() >= 7) {
			auto os_text = os_str.find(from_ascii("\\string"));

			if (os_text != lyx::docstring::npos && !std::isalpha(static_cast<int>(os_str[os_text + 7]))) {
				os_str = os_str.substr(0, os_text) + os_str.substr(os_text + 7, os_str.length());

				if (os_str.find('\\') == std::string::npos) {
					xs << os_str;
					output_as_comment = false;
					break;
				}
			} else {
				break;
			}
		}
	}

	// Otherwise, output the ERT as a comment with the appropriate escaping if the command is not recognised.
	if (output_as_comment) {
		xs << XMLStream::ESCAPE_NONE << "<!-- ";
		xs << XMLStream::ESCAPE_COMMENTS << os.str();
		xs << XMLStream::ESCAPE_NONE << " -->";
	}
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
	// U+1F512 LOCK
	docstring const locked = tempfile_ ? docstring(1, 0x1F512) : docstring();
	if (decoration() == InsetDecoration::CLASSIC)
		return locked + (isOpen(bv) ? _("ERT") : getNewLabel(_("ERT")));
	return locked + getNewLabel(_("ERT"));
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
