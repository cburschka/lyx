/**
 * \file InsetRef.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */
#include <config.h>

#include "InsetRef.h"

#include "Buffer.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "support/gettext.h"
#include "LaTeXFeatures.h"
#include "LyXFunc.h"
#include "OutputParams.h"
#include "sgml.h"

#include "support/docstream.h"
#include "support/lstrings.h"


namespace lyx {

using support::escape;

using std::string;
using std::ostream;


InsetRef::InsetRef(InsetCommandParams const & p, Buffer const & buf)
	: InsetCommand(p, "ref"), isLatex(buf.isLatex())
{}


InsetRef::InsetRef(InsetRef const & ir)
	: InsetCommand(ir), isLatex(ir.isLatex)
{}


bool InsetRef::isCompatibleCommand(std::string const & s) {
	//FIXME This is likely not the best way to handle this.
	//But this stuff is hardcoded elsewhere already.
	return s == "ref" 
		|| s == "pageref"
		|| s == "vref" 
		|| s == "vpageref"
		|| s == "prettyref"
		|| s == "eqref";
}


CommandInfo const * InsetRef::findInfo(std::string const & /* cmdName */)
{
	static const char * const paramnames[] = {"name", "reference", ""};
	static const bool isoptional[] = {true, false};
	static const CommandInfo info = {2, paramnames, isoptional};
	return &info;
}


void InsetRef::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_MOUSE_RELEASE:
		// Eventually trigger dialog with button 3 not 1
		if (cmd.button() == mouse_button::button3)
			lyx::dispatch(FuncRequest(LFUN_LABEL_GOTO,
						  getParam("reference")));
		else
			InsetCommand::doDispatch(cur, cmd);
		break;

	default:
		InsetCommand::doDispatch(cur, cmd);
	}
}


docstring const InsetRef::getScreenLabel(Buffer const &) const
{
	docstring temp;
	for (int i = 0; !types[i].latex_name.empty(); ++i) {
		if (getCmdName() == types[i].latex_name) {
			temp = _(types[i].short_gui_name);
			break;
		}
	}
	temp += getParam("reference");

	if (!isLatex && !getParam("name").empty()) {
		temp += "||";
		temp += getParam("name");
	}
	return temp;
}


int InsetRef::latex(Buffer const &, odocstream & os,
		    OutputParams const &) const
{
	// We don't want to output p_["name"], since that is only used 
	// in docbook. So we construct new params, without it, and use that.
	InsetCommandParams p(REF_CODE, getCmdName());
	p["reference"] = getParam("reference");
	os << escape(p.getCommand());
	return 0;
}


int InsetRef::plaintext(Buffer const &, odocstream & os,
			OutputParams const &) const
{
	docstring const str = getParam("reference");
	os << '[' << str << ']';
	return 2 + str.size();
}


int InsetRef::docbook(Buffer const & buf, odocstream & os,
		      OutputParams const & runparams) const
{
	docstring const & name = getParam("name");
	if (name.empty()) {
		if (runparams.flavor == OutputParams::XML) {
			os << "<xref linkend=\""
			   << sgml::cleanID(buf, runparams, getParam("reference"))
			   << "\" />";
		} else {
			os << "<xref linkend=\""
			   << sgml::cleanID(buf, runparams, getParam("reference"))
			   << "\">";
		}
	} else {
		os << "<link linkend=\""
		   << sgml::cleanID(buf, runparams, getParam("reference"))
		   << "\">"
		   << getParam("name")
		   << "</link>";
	}

	return 0;
}


int InsetRef::textString(Buffer const & buf, odocstream & os,
		       OutputParams const & op) const
{
	return plaintext(buf, os, op);
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
	{ "prettyref", N_("PrettyRef"),             N_("FormatRef: ")},
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


} // namespace lyx
