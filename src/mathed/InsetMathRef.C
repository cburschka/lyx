/**
 * \file InsetMathRef.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathRef.h"

#include "BufferView.h"
#include "LaTeXFeatures.h"
#include "buffer.h"
#include "cursor.h"
#include "debug.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "MathData.h"
#include "MathFactory.h"
#include "MathSupport.h"
#include "outputparams.h"
#include "sgml.h"


namespace lyx {

using std::string;
using std::auto_ptr;
using std::endl;


RefInset::RefInset()
	: CommandInset(from_ascii("ref"))
{}


RefInset::RefInset(docstring const & data)
	: CommandInset(data)
{}


auto_ptr<InsetBase> RefInset::doClone() const
{
	return auto_ptr<InsetBase>(new RefInset(*this));
}


void RefInset::infoize(odocstream & os) const
{
	os << "Ref: " << cell(0);
}


void RefInset::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "ref") {
			MathArray ar;
			if (createInsetMath_fromDialogStr(cmd.argument(), ar)) {
				*this = *ar[0].nucleus()->asRefInset();
				break;
			}
		}
		cur.undispatched();
		break;

	case LFUN_INSET_DIALOG_UPDATE: {
		string const data = createDialogStr("ref");
		cur.bv().updateDialog("ref", data);
		break;
	}

	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3) {
			lyxerr << "trying to goto ref '" << to_utf8(asString(cell(0))) << "'" << endl;
			cur.bv().dispatch(FuncRequest(LFUN_LABEL_GOTO, asString(cell(0))));
			break;
		}
		if (cmd.button() == mouse_button::button1) {
			// Eventually trigger dialog with button 3, not 1
			string const data = createDialogStr("ref");
			cur.bv().showInsetDialog("ref", data, this);
			break;
		}
		cur.undispatched();
		break;

	case LFUN_MOUSE_PRESS:
	case LFUN_MOUSE_MOTION:
		// eat other mouse commands
		break;

	default:
		CommandInset::doDispatch(cur, cmd);
		break;
	}
}


bool RefInset::getStatus(LCursor & cur, FuncRequest const & cmd,
			 FuncStatus & status) const
{
	switch (cmd.action) {
	// we handle these
	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
	case LFUN_MOUSE_RELEASE:
	case LFUN_MOUSE_PRESS:
	case LFUN_MOUSE_MOTION:
		status.enabled(true);
		return true;
	default:
		return CommandInset::getStatus(cur, cmd, status);
	}
}


docstring const RefInset::screenLabel() const
{
	docstring str;
	for (int i = 0; !types[i].latex_name.empty(); ++i) {
		if (commandname() == types[i].latex_name) {
			str = _(to_utf8(types[i].short_gui_name));
			break;
		}
	}
	str += asString(cell(0));

	//if (/* !isLatex && */ !cell(0).empty()) {
	//	str += "||";
	//	str += asString(cell(1));
	//}
	return str;
}


void RefInset::validate(LaTeXFeatures & features) const
{
	if (commandname() == "vref" || commandname() == "vpageref")
		features.require("varioref");
	else if (commandname() == "prettyref")
		features.require("prettyref");
}


int RefInset::plaintext(odocstream & os, OutputParams const &) const
{
	// FIXME UNICODE
	os << '[' << asString(cell(0)) << ']';
	return 0;
}


int RefInset::docbook(Buffer const & buf, odocstream & os,
		OutputParams const & runparams) const
{
	if (cell(1).empty()) {
		os << "<xref linkend=\""
		   << sgml::cleanID(buf, runparams, asString(cell(0)));
		if (runparams.flavor == OutputParams::XML)
			os << "\"/>";
		else
			os << "\">";
	} else {
		os << "<link linkend=\""
		   << sgml::cleanID(buf, runparams, asString(cell(0)))
		   << "\">"
		   << asString(cell(1))
		   << "</link>";
	}

	return 0;
}


RefInset::ref_type_info RefInset::types[] = {
	{ from_ascii("ref"),       from_ascii(N_("Standard")),              from_ascii(N_("Ref: "))},
	{ from_ascii("eqref"),     from_ascii(N_("Equation")),              from_ascii(N_("EqRef: "))},
	{ from_ascii("pageref"),   from_ascii(N_("Page Number")),           from_ascii(N_("Page: "))},
	{ from_ascii("vpageref"),  from_ascii(N_("Textual Page Number")),   from_ascii(N_("TextPage: "))},
	{ from_ascii("vref"),      from_ascii(N_("Standard+Textual Page")), from_ascii(N_("Ref+Text: "))},
	{ from_ascii("prettyref"), from_ascii(N_("PrettyRef")),             from_ascii(N_("FormatRef: "))},
	{ from_ascii(""), from_ascii(""), from_ascii("") }
};


} // namespace lyx
