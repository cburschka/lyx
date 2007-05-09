/**
 * \file InsetListings.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetListings.h"

#include "Language.h"
#include "gettext.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Cursor.h"
#include "support/lstrings.h"

#include <sstream>

namespace lyx {

using support::token;

using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::string;


void InsetListings::init()
{
	// FIXME: define Color::listing?
	Font font(Font::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(Color::foreground);
	setLabelFont(font);
	// FIXME: english_language?
	text_.current_font.setLanguage(english_language);
	text_.real_current_font.setLanguage(english_language);
	// FIXME: why I can not make text of source code black with the following two lines?
	text_.current_font.setColor(Color::foreground);
	text_.real_current_font.setColor(Color::foreground);
}


InsetListings::InsetListings(BufferParams const & bp, InsetListingsParams const & par)
	: InsetERT(bp, par.status())
{
	init();
}


InsetListings::InsetListings(InsetListings const & in)
	: InsetERT(in)
{
	init();
}


auto_ptr<Inset> InsetListings::doClone() const
{
	return auto_ptr<Inset>(new InsetListings(*this));
}


InsetListings::~InsetListings()
{
	InsetListingsMailer(*this).hideDialog();
}


bool InsetListings::display() const
{
	return !params().isInline();
}


void InsetListings::write(Buffer const & buf, ostream & os) const
{
	os << "listings" << "\n";
	InsetListingsParams const & par = params();
	// parameter string is encoded to be a valid lyx token.
	string opt = par.encodedString();
	if (!opt.empty())
		os << "lstparams \"" << opt << "\"\n";
	if (par.isInline())
		os << "inline true\n";
	else
		os << "inline false\n";
	InsetCollapsable::write(buf, os);
}


void InsetListings::read(Buffer const & buf, Lexer & lex)
{
	while (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token == "lstparams") {
			lex.next();
			string const value = lex.getString();
			params().fromEncodedString(value);
		} else if (token == "inline") {
			lex.next();
			params().setInline(lex.getBool());
		} else {
			// no special option, push back 'status' etc
			lex.pushToken(token);
			break;
		}
	}
	InsetCollapsable::read(buf, lex);
}


docstring const InsetListings::editMessage() const
{
	return _("Opened Listings Inset");
}


int InsetListings::latex(Buffer const &, odocstream & os,
		    OutputParams const &) const
{
	string param_string = params().encodedString();
	// NOTE: I use {} to quote text, which is an experimental feature
	// of the listings package (see page 25 of the manual)
	int lines = 0;
	bool lstinline = params().isInline();
	if (lstinline) {
		if (param_string.empty())
			os << "\\lstinline{";
		else
			os << "\\lstinline[" << from_ascii(param_string) << "]{";
	} else {
		if (param_string.empty())
			os << "\n\\begingroup\n\\inputencoding{latin1}\n\\begin{lstlisting}\n";
		else
			os << "\n\\begingroup\n\\inputencoding{latin1}\n\\begin{lstlisting}[" << from_ascii(param_string) << "]\n";
		lines += 4;
	}
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	while (par != end) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			// ignore all struck out text
			if (par->isDeleted(i))
				continue;
			os.put(par->getChar(i));
		}
		++par;
		// for the inline case, if there are multiple paragraphs
		// they are simply joined. Otherwise, expect latex errors. 
		if (par != end && !lstinline) {
			os << "\n";
			++lines;
		}
	}
	if (lstinline)
		os << "}";		
	else {
		os << "\n\\end{lstlisting}\n\\endgroup\n";
		lines += 3;
	}

	return lines;
}


void InsetListings::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetListingsMailer::string2params(to_utf8(cmd.argument()), params());
		break;
	}
	case LFUN_INSET_DIALOG_UPDATE:
		InsetListingsMailer(*this).updateDialog(&cur.bv());
		break;	
	case LFUN_MOUSE_RELEASE: {
		if (cmd.button() == mouse_button::button3 && hitButton(cmd)) {
			InsetListingsMailer(*this).showDialog(&cur.bv());
			break;
		}
		InsetERT::doDispatch(cur, cmd);
		break;
	}
	default:
		InsetERT::doDispatch(cur, cmd);
		break;
	}
}


bool InsetListings::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
		case LFUN_INSET_DIALOG_UPDATE:
			status.enabled(true);
			return true;
		default:
			return InsetERT::getStatus(cur, cmd, status);
	}
}


void InsetListings::setButtonLabel()
{
	// FIXME UNICODE
	setLabel(isOpen() ?  _("Listings") : getNewLabel(_("Listings")));
}


void InsetListings::validate(LaTeXFeatures & features) const
{
	features.require("listings");
	InsetERT::validate(features);
}


bool InsetListings::showInsetDialog(BufferView * bv) const
{
	InsetListingsMailer(const_cast<InsetListings &>(*this)).showDialog(bv);
	return true;
}


void InsetListings::getDrawFont(Font & font) const
{
	font = Font(Font::ALL_INHERIT, english_language);
	font.setFamily(Font::TYPEWRITER_FAMILY);
	font.setColor(Color::foreground);
}


string const InsetListingsMailer::name_("listings");

InsetListingsMailer::InsetListingsMailer(InsetListings & inset)
	: inset_(inset)
{}


string const InsetListingsMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.params());
}


void InsetListingsMailer::string2params(string const & in,
				   InsetListingsParams & params)
{
	params = InsetListingsParams();
	if (in.empty())
		return;
	istringstream data(in);
	Lexer lex(0, 0);
	lex.setStream(data);
	// discard "listings", which is only used to determine inset
	lex.next();
	params.read(lex);
}


string const
InsetListingsMailer::params2string(InsetListingsParams const & params)
{
	ostringstream data;
	data << name_ << " ";
	params.write(data);
	return data.str();
}


} // namespace lyx
