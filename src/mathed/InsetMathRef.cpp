/**
 * \file InsetMathRef.cpp
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
#include "Buffer.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "LyX.h"
#include "MathData.h"
#include "MathFactory.h"
#include "MathSupport.h"
#include "ParIterator.h"
#include "xml.h"

#include "insets/InsetCommand.h"

#include "support/debug.h"
#include "support/gettext.h"

#include <ostream>

using namespace std;

namespace lyx {

InsetMathRef::InsetMathRef(Buffer * buf)
	: InsetMathCommand(buf, from_ascii("ref"), false)
{}


InsetMathRef::InsetMathRef(Buffer * buf, docstring const & data)
	: InsetMathCommand(buf, data, false)
{}


Inset * InsetMathRef::clone() const
{
	return new InsetMathRef(*this);
}


void InsetMathRef::infoize(odocstream & os) const
{
	os << "Ref: " << cell(0);
}


void InsetMathRef::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "ref") {
			if (cmd.getArg(1) == "changetarget") {
				string const oldtarget = cmd.getArg(2);
				string const newtarget = cmd.getArg(3);
				if (!oldtarget.empty() && !newtarget.empty()
				    && asString(cell(0)) == from_utf8(oldtarget))
					changeTarget(from_utf8(newtarget));
				cur.forceBufferUpdate();
				break;
			}
			MathData ar;
			if (createInsetMath_fromDialogStr(cmd.argument(), ar)) {
				cur.recordUndo();
				*this = *ar[0].nucleus()->asRefInset();
				break;
			}
		}
		cur.undispatched();
		break;

	case LFUN_INSET_DIALOG_UPDATE: {
		string const data = createDialogStr();
		cur.bv().updateDialog("ref", data);
		break;
	}

	case LFUN_MOUSE_RELEASE:
		if (cur.selection()) {
			cur.undispatched();
			break;
		}
		if (cmd.button() == mouse_button::button3) {
			LYXERR0("trying to goto ref '" << to_utf8(asString(cell(0))) << "'");
			//FIXME: use DispatchResult argument
			lyx::dispatch(FuncRequest(LFUN_LABEL_GOTO, asString(cell(0))));
			break;
		}
		if (cmd.button() == mouse_button::button1) {
			// Eventually trigger dialog with button 3, not 1
			string const data = createDialogStr();
			cur.bv().showDialog("ref", data, this);
			break;
		}
		cur.undispatched();
		break;

	case LFUN_MOUSE_PRESS: {
		bool do_selection = cmd.button() == mouse_button::button1
			&& cmd.modifier() == ShiftModifier;
		// For some reason the cursor points inside the first cell, which is not
		// active.
		cur.leaveInset(*this);
		cur.bv().mouseSetCursor(cur, do_selection);
		break;
	}

	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE:
		// eat other mouse commands
		break;

	default:
		InsetMathCommand::doDispatch(cur, cmd);
		break;
	}
}


bool InsetMathRef::getStatus(Cursor & cur, FuncRequest const & cmd,
			 FuncStatus & status) const
{
	switch (cmd.action()) {
	// we handle these
	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
	case LFUN_MOUSE_RELEASE:
	case LFUN_MOUSE_PRESS:
	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE:
		status.setEnabled(true);
		return true;
	default:
		return InsetMathCommand::getStatus(cur, cmd, status);
	}
}


docstring const InsetMathRef::screenLabel() const
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


void InsetMathRef::validate(LaTeXFeatures & features) const
{
	if (commandname() == "vref" || commandname() == "vpageref")
		features.require("varioref");
	else if (commandname() == "prettyref")
		features.require("prettyref");
	else if (commandname() == "eqref")
		features.require("amsmath");
	else if (commandname() == "nameref")
		features.require("nameref");
}


void InsetMathRef::docbook(XMLStream & xs, OutputParams const &) const
{
	if (cell(1).empty()) {
		docstring attr = from_utf8("linkend=\"") + xml::cleanID(asString(cell(0))) + from_utf8("\"");
		xs << xml::CompTag("xref", to_utf8(attr));
	} else {
		// Link with linkend, as is it within the document (not outside, in which case xlink:href is better suited).
		docstring attr = from_utf8("linkend=\"") + xml::cleanID(asString(cell(0))) + from_utf8("\"");
		xs << xml::StartTag("link", to_utf8(attr))
		   << asString(cell(1))
		   << xml::EndTag("link");
	}
}


void InsetMathRef::updateBuffer(ParIterator const & it, UpdateType /*utype*/, bool const /*deleted*/)
{
	if (!buffer_) {
		LYXERR0("InsetMathRef::updateBuffer: no buffer_!");
		return;
	}
	// register this inset into the buffer reference cache.
	buffer().addReference(getTarget(), this, it);
}


string const InsetMathRef::createDialogStr() const
{
	InsetCommandParams icp(REF_CODE, to_ascii(commandname()));
	icp["reference"] = asString(cell(0));
	if (!cell(1).empty())
		icp["name"] = asString(cell(1));
	return InsetCommand::params2string(icp);
}


docstring const InsetMathRef::getTarget() const
{
	return asString(cell(0));
}


void InsetMathRef::changeTarget(docstring const & target)
{
	InsetCommandParams icp(REF_CODE, to_ascii(commandname()));
	icp["reference"] = target;
	if (!cell(1).empty())
		icp["name"] = asString(cell(1));
	MathData ar;
	Buffer & buf = buffer();
	if (createInsetMath_fromDialogStr(
	    from_utf8(InsetCommand::params2string(icp)), ar)) {
		*this = *ar[0].nucleus()->asRefInset();
		// FIXME audit setBuffer calls
		setBuffer(buf);
	}
}


InsetMathRef::ref_type_info InsetMathRef::types[] = {
	{ from_ascii("ref"),       from_ascii(N_("Standard[[mathref]]")),   from_ascii(N_("Ref: "))},
	{ from_ascii("eqref"),     from_ascii(N_("Equation")),              from_ascii(N_("EqRef: "))},
	{ from_ascii("pageref"),   from_ascii(N_("Page Number")),           from_ascii(N_("Page: "))},
	{ from_ascii("vpageref"),  from_ascii(N_("Textual Page Number")),   from_ascii(N_("TextPage: "))},
	{ from_ascii("vref"),      from_ascii(N_("Standard+Textual Page")), from_ascii(N_("Ref+Text: "))},
	{ from_ascii("prettyref"), from_ascii(N_("PrettyRef")),             from_ascii(N_("FormatRef: "))},
	{ from_ascii("nameref"),   from_ascii(N_("Reference to Name")),     from_ascii(N_("NameRef: "))},
	{ from_ascii(""), from_ascii(""), from_ascii("") }
};


} // namespace lyx
