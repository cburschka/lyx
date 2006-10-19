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

using lyx::docstring;
using lyx::odocstream;

using std::string;
using std::auto_ptr;
using std::endl;


RefInset::RefInset()
	: CommandInset("ref")
{}


RefInset::RefInset(string const & data)
	: CommandInset(data)
{}


auto_ptr<InsetBase> RefInset::doClone() const
{
	return auto_ptr<InsetBase>(new RefInset(*this));
}


void RefInset::infoize(std::ostream & os) const
{
	os << "Ref: " << cell(0);
}


void RefInset::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "ref") {
			MathArray ar;
			if (createInsetMath_fromDialogStr(lyx::to_utf8(cmd.argument()), ar)) {
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
			lyxerr << "trying to goto ref '" << asString(cell(0)) << "'" << endl;
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
	for (int i = 0; !types[i].latex_name.empty(); ++i)
		if (commandname() == types[i].latex_name) {
			str = _(types[i].short_gui_name);
			break;
		}
	// FIXME UNICODE
	str += lyx::from_utf8(asString(cell(0)));

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


int RefInset::plaintext(lyx::odocstream & os, OutputParams const &) const
{
	// FIXME UNICODE
	os << '[' << lyx::from_utf8(asString(cell(0))) << ']';
	return 0;
}


int RefInset::docbook(Buffer const & buf, odocstream & os, OutputParams const & runparams) const
{
	if (cell(1).empty()) {
                // FIXME UNICODE
		os << "<xref linkend=\""
                   << lyx::from_ascii(sgml::cleanID(buf, runparams, asString(cell(0))));
		if (runparams.flavor == OutputParams::XML)
			os << "\"/>";
		else
			os << "\">";
	} else {
                // FIXME UNICODE
		os << "<link linkend=\""
                   << lyx::from_ascii(sgml::cleanID(buf, runparams, asString(cell(0))))
		   << "\">"
                   << lyx::from_ascii(asString(cell(1)))
                   << "</link>";
	}

	return 0;
}


RefInset::ref_type_info RefInset::types[] = {
	{ "ref",       N_("Standard"),              N_("Ref: ")},
	{ "eqref",     N_("Equation"),              N_("EqRef: ")},
	{ "pageref",   N_("Page Number"),           N_("Page: ")},
	{ "vpageref",  N_("Textual Page Number"),   N_("TextPage: ")},
	{ "vref",      N_("Standard+Textual Page"), N_("Ref+Text: ")},
	{ "prettyref", N_("PrettyRef"),             N_("PrettyRef: ")},
	{ "", "", "" }
};
