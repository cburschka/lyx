/**
 * \file insetref.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS
 */
#include <config.h>


#include "insetref.h"
#include "buffer.h"
#include "funcrequest.h"
#include "debug.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"
#include "BufferView.h"
#include "support/lstrings.h"

using namespace lyx::support;

using std::ostream;

InsetRef::InsetRef(InsetCommandParams const & p, Buffer const & buf)
	: InsetCommand(p), isLatex(buf.isLatex())
{}


InsetRef::InsetRef(InsetRef const & ir)
	: InsetCommand(ir), isLatex(ir.isLatex)
{
}


InsetRef::~InsetRef()
{
	InsetCommandMailer("ref", *this).hideDialog();
}


dispatch_result InsetRef::localDispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_EDIT:
		// Eventually trigger dialog with button 3 not 1
		if (cmd.button() == mouse_button::button3)
			cmd.view()->owner()->
				dispatch(FuncRequest(LFUN_REF_GOTO, getContents()));
		else
			InsetCommandMailer("ref", *this).showDialog(cmd.view());
		return DISPATCHED;

	default:
		return InsetCommand::localDispatch(cmd);
	}
}


string const InsetRef::getScreenLabel(Buffer const *) const
{
	string temp;
	for (int i = 0; !types[i].latex_name.empty(); ++ i)
		if (getCmdName() == types[i].latex_name) {
			temp = _(types[i].short_gui_name);
			break;
		}
	temp += getContents();

	if (!isLatex
	   && !getOptions().empty()) {
		temp += "||";
		temp += getOptions();
	}
	return temp;
}


int InsetRef::latex(Buffer const *, ostream & os,
		    LatexRunParams const &) const
{
	if (getOptions().empty())
		os << escape(getCommand());
	else {
		InsetCommandParams p(getCmdName(), getContents(), "");
		os << escape(p.getCommand());
	}
	return 0;
}


int InsetRef::ascii(Buffer const *, ostream & os, int) const
{
	os << '[' << getContents() << ']';
	return 0;
}


int InsetRef::linuxdoc(Buffer const *, ostream & os) const
{
	os << "<ref id=\"" << getContents()
	   << "\" name=\"" << getOptions() << "\" >";
	return 0;
}


int InsetRef::docbook(Buffer const *, ostream & os, bool) const
{
	if (getOptions().empty()) {
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
