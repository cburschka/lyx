/**
 * \file insetref.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */
#include <config.h>

#include "insetref.h"

#include "buffer.h"
#include "cursor.h"
#include "BufferView.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "outputparams.h"

#include "frontends/LyXView.h"

#include "support/lstrings.h"


using lyx::support::escape;

using std::string;
using std::ostream;


InsetRef::InsetRef(InsetCommandParams const & p, Buffer const & buf)
	: InsetCommand(p, "ref"), isLatex(buf.isLatex())
{}


InsetRef::InsetRef(InsetRef const & ir)
	: InsetCommand(ir), isLatex(ir.isLatex)
{}


void InsetRef::priv_dispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_MOUSE_PRESS:
		// Eventually trigger dialog with button 3 not 1
		if (cmd.button() == mouse_button::button3)
			cur.bv().owner()->dispatch(FuncRequest(LFUN_REF_GOTO, getContents()));
		else
			InsetCommandMailer("ref", *this).showDialog(&cur.bv());
		return;

	case LFUN_MOUSE_RELEASE:
		return;

	default:
		return InsetCommand::priv_dispatch(cur, cmd);
	}
}


string const InsetRef::getScreenLabel(Buffer const &) const
{
	string temp;
	for (int i = 0; !types[i].latex_name.empty(); ++i) {
		if (getCmdName() == types[i].latex_name) {
			temp = _(types[i].short_gui_name);
			break;
		}
	}
	temp += getContents();

	if (!isLatex && !getOptions().empty()) {
		temp += "||";
		temp += getOptions();
	}
	return temp;
}


int InsetRef::latex(Buffer const &, ostream & os,
		    OutputParams const &) const
{
	if (getOptions().empty())
		os << escape(getCommand());
	else {
		InsetCommandParams p(getCmdName(), getContents(), "");
		os << escape(p.getCommand());
	}
	return 0;
}


int InsetRef::plaintext(Buffer const &, ostream & os,
		    OutputParams const &) const
{
	os << '[' << getContents() << ']';
	return 0;
}


int InsetRef::linuxdoc(Buffer const &, ostream & os,
		       OutputParams const &) const
{
	os << "<ref id=\"" << getContents()
	   << "\" name=\"" << getOptions() << "\" >";
	return 0;
}


int InsetRef::docbook(Buffer const &, ostream & os,
		      OutputParams const & runparams) const
{
	if (getOptions().empty() && runparams.flavor == OutputParams::XML) {
		os << "<xref linkend=\"" << getContents() << "\" />";
	} else if (getOptions().empty()) {
		os << "<xref linkend=\"" << getContents() << "\">";
	} else {
		os << "<link linkend=\"" << getContents()
		   << "\">" << getOptions() << "</link>";
	}

	return 0;
}


void InsetRef::validate(LaTeXFeatures & features) const
{
	if (getCmdName() == "vref" || getCmdName() == "vpageref")
		features.require("varioref");
	else if (getCmdName() == "prettyref")
		features.require("prettyref");
	else if (getCmdName() == "eqref")
		features.require("amsmath");
}


InsetRef::type_info InsetRef::types[] = {
	{ "ref",       N_("Standard"),              N_("Ref: ")},
	{ "eqref",     N_("Equation"),              N_("EqRef: ")},
	{ "pageref",   N_("Page Number"),           N_("Page: ")},
	{ "vpageref",  N_("Textual Page Number"),   N_("TextPage: ")},
	{ "vref",      N_("Standard+Textual Page"), N_("Ref+Text: ")},
	{ "prettyref", N_("PrettyRef"),             N_("PrettyRef: ")},
	{ "", "", "" }
};


int InsetRef::getType(string const & name)
{
	for (int i = 0; !types[i].latex_name.empty(); ++i)
		if (name == types[i].latex_name)
			return i;
	return 0;
}


string const & InsetRef::getName(int type)
{
	return types[type].latex_name;
}
