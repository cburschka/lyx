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
#include "LaTeXFeatures.h"
#include "LyXFunc.h"
#include "OutputParams.h"
#include "ParIterator.h"
#include "sgml.h"
#include "TocBackend.h"

#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

using namespace lyx::support;
using namespace std;

namespace lyx {


InsetRef::InsetRef(Buffer const & buf, InsetCommandParams const & p)
	: InsetCommand(p, "ref"), isLatex(buf.isLatex())
{}


InsetRef::InsetRef(InsetRef const & ir)
	: InsetCommand(ir), isLatex(ir.isLatex)
{}


bool InsetRef::isCompatibleCommand(string const & s) {
	//FIXME This is likely not the best way to handle this.
	//But this stuff is hardcoded elsewhere already.
	return s == "ref" 
		|| s == "pageref"
		|| s == "vref" 
		|| s == "vpageref"
		|| s == "prettyref"
		|| s == "eqref";
}


ParamInfo const & InsetRef::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("name", ParamInfo::LATEX_OPTIONAL);
		param_info_.add("reference", ParamInfo::LATEX_REQUIRED);
	}
	return param_info_;
}


docstring InsetRef::screenLabel() const
{
	return screen_label_;
}


int InsetRef::latex(odocstream & os, OutputParams const &) const
{
	// We don't want to output p_["name"], since that is only used 
	// in docbook. So we construct new params, without it, and use that.
	InsetCommandParams p(REF_CODE, getCmdName());
	p["reference"] = getParam("reference");
	os << escape(p.getCommand());
	return 0;
}


int InsetRef::plaintext(odocstream & os, OutputParams const &) const
{
	docstring const str = getParam("reference");
	os << '[' << str << ']';
	return 2 + str.size();
}


int InsetRef::docbook(odocstream & os, OutputParams const & runparams) const
{
	docstring const & name = getParam("name");
	if (name.empty()) {
		if (runparams.flavor == OutputParams::XML) {
			os << "<xref linkend=\""
			   << sgml::cleanID(buffer(), runparams, getParam("reference"))
			   << "\" />";
		} else {
			os << "<xref linkend=\""
			   << sgml::cleanID(buffer(), runparams, getParam("reference"))
			   << "\">";
		}
	} else {
		os << "<link linkend=\""
		   << sgml::cleanID(buffer(), runparams, getParam("reference"))
		   << "\">"
		   << getParam("name")
		   << "</link>";
	}

	return 0;
}


void InsetRef::textString(odocstream & os) const
{
	plaintext(os, OutputParams(0));
}


void InsetRef::updateLabels(ParIterator const & it)
{
	docstring const & label = getParam("reference");
	// register this inset into the buffer reference cache.
	buffer().references(label).push_back(make_pair(this, it));

	for (int i = 0; !types[i].latex_name.empty(); ++i) {
		if (getCmdName() == types[i].latex_name) {
			screen_label_ = _(types[i].short_gui_name);
			break;
		}
	}
	screen_label_ += getParam("reference");

	if (!isLatex && !getParam("name").empty()) {
		screen_label_ += "||";
		screen_label_ += getParam("name");
	}
}


void InsetRef::addToToc(DocIterator const & cpit)
{
	docstring const & label = getParam("reference");
	if (buffer().insetLabel(label))
		// This InsetRef has already been taken care of in InsetLabel::addToToc().
		return;

	// It seems that this reference does not point to any valid label.
	screen_label_ = _("BROKEN: ") + screen_label_;
	Toc & toc = buffer().tocBackend().toc("label");
	toc.push_back(TocItem(cpit, 0, screen_label_));
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
