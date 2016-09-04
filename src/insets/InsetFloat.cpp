/**
 * \file InsetFloat.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetFloat.h"
#include "InsetCaption.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Counters.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "Floating.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "output_xhtml.h"
#include "ParIterator.h"
#include "TexRow.h"
#include "texstream.h"
#include "TextClass.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include "frontends/Application.h"

using namespace std;
using namespace lyx::support;


namespace lyx {

// With this inset it will be possible to support the latex package
// float.sty, and I am sure that with this and some additional support
// classes we can support similar functionality in other formats
// (read DocBook).
// By using float.sty we will have the same handling for all floats, both
// for those already in existance (table and figure) and all user created
// ones¹. So suddenly we give the users the possibility of creating new
// kinds of floats on the fly. (and with a uniform look)
//
// API to float.sty:
//   \newfloat{type}{placement}{ext}[within]
//     type      - The "type" of the new class of floats, like program or
//                 algorithm. After the appropriate \newfloat, commands
//                 such as \begin{program} or \end{algorithm*} will be
//                 available.
//     placement - The default placement for the given class of floats.
//                 They are like in standard LaTeX: t, b, p and h for top,
//                 bottom, page, and here, respectively. On top of that
//                 there is a new type, H, which does not really correspond
//                 to a float, since it means: put it "here" and nowhere else.
//                 Note, however that the H specifier is special and, because
//                 of implementation details cannot be used in the second
//                 argument of \newfloat.
//     ext       - The file name extension of an auxiliary file for the list
//                 of figures (or whatever). LaTeX writes the captions to
//                 this file.
//     within    - This (optional) argument determines whether floats of this
//                 class will be numbered within some sectional unit of the
//                 document. For example, if within is equal to chapter, the
//                 floats will be numbered within chapters.
//   \floatstyle{style}
//     style -  plain, boxed, ruled
//   \floatname{float}{floatname}
//     float     -
//     floatname -
//   \floatplacement{float}{placement}
//     float     -
//     placement -
//   \restylefloat{float}
//     float -
//   \listof{type}{title}
//     title -

// ¹ the algorithm float is defined using the float.sty package. Like this
//   \floatstyle{ruled}
//   \newfloat{algorithm}{htbp}{loa}[<sect>]
//   \floatname{algorithm}{Algorithm}
//
// The intention is that floats should be definable from two places:
//          - layout files
//          - the "gui" (i.e. by the user)
//
// From layout files.
// This should only be done for floats defined in a documentclass and that
// does not need any additional packages. The two most known floats in this
// category is "table" and "figure". Floats defined in layout files are only
// stored in lyx files if the user modifies them.
//
// By the user.
// There should be a gui dialog (and also a collection of lyxfuncs) where
// the user can modify existing floats and/or create new ones.
//
// The individual floats will also have some settable
// variables: wide and placement.
//
// Lgb

//FIXME: why do we set in stone the type here?
InsetFloat::InsetFloat(Buffer * buf, string params_str)
	: InsetCaptionable(buf)
{
	string2params(params_str, params_);
	setCaptionType(params_.type);
}


// Enforce equality of float type and caption type.
void InsetFloat::setCaptionType(std::string const & type)
{
	InsetCaptionable::setCaptionType(type);	
	params_.type = captionType();
	// check if the float type exists
	if (buffer().params().documentClass().floats().typeExist(params_.type))
		setLabel(_("float: ") + floatName(params_.type));
	else
		setLabel(bformat(_("ERROR: Unknown float type: %1$s"), from_utf8(params_.type)));
}


docstring InsetFloat::layoutName() const
{ 
	return "Float:" + from_utf8(params_.type);
}


docstring InsetFloat::toolTip(BufferView const & bv, int x, int y) const
{
	if (isOpen(bv))
		return InsetCaptionable::toolTip(bv, x, y);

	OutputParams rp(&buffer().params().encoding());
	return getCaptionText(rp);
}


void InsetFloat::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		InsetFloatParams params;
		string2params(to_utf8(cmd.argument()), params);
		cur.recordUndoInset(this);

		// placement, wide and sideways are not used for subfloats
		if (!params_.subfloat) {
			params_.placement = params.placement;
			params_.wide      = params.wide;
			params_.sideways  = params.sideways;
		}
		setNewLabel();
		if (params_.type != params.type)
			setCaptionType(params.type);
		// what we really want here is a TOC update, but that means
		// a full buffer update
		cur.forceBufferUpdate();
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE: {
		cur.bv().updateDialog("float", params2string(params()));
		break;
	}

	default:
		InsetCaptionable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetFloat::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
		flag.setEnabled(true);
		return true;

	case LFUN_INSET_SETTINGS:
		if (InsetCaptionable::getStatus(cur, cmd, flag)) {
			flag.setEnabled(flag.enabled() && !params_.subfloat);
			return true;
		} else
			return false;
	
	case LFUN_NEWLINE_INSERT:
		if (params_.subfloat) {
			flag.setEnabled(false);
			return true;
		}

	default:
		return InsetCaptionable::getStatus(cur, cmd, flag);
	}
}


bool InsetFloat::hasSubCaptions(ParIterator const & it) const
{
	return (it.innerInsetOfType(FLOAT_CODE) || it.innerInsetOfType(WRAP_CODE));
}


void InsetFloatParams::write(ostream & os) const
{
	if (type.empty()) {
		// Better this than creating a parse error. This in fact happens in the
		// parameters dialog via InsetFloatParams::params2string.
		os << "senseless" << '\n';
	} else
		os << type << '\n';

	if (!placement.empty())
		os << "placement " << placement << "\n";

	if (wide)
		os << "wide true\n";
	else
		os << "wide false\n";

	if (sideways)
		os << "sideways true\n";
	else
		os << "sideways false\n";
}


void InsetFloatParams::read(Lexer & lex)
{
	lex.setContext("InsetFloatParams::read");
	lex >> type;
	if (lex.checkFor("placement"))
		lex >> placement;
	lex >> "wide" >> wide;
	lex >> "sideways" >> sideways;
}


void InsetFloat::write(ostream & os) const
{
	os << "Float ";
	params_.write(os);
	InsetCaptionable::write(os);
}


void InsetFloat::read(Lexer & lex)
{
	params_.read(lex);
	InsetCaptionable::read(lex);
	setCaptionType(params_.type);
}


void InsetFloat::validate(LaTeXFeatures & features) const
{
	if (support::contains(params_.placement, 'H'))
		features.require("float");

	if (params_.sideways)
		features.require("rotfloat");

	if (features.inFloat())
		features.require("subfig");

	features.useFloat(params_.type, features.inFloat());
	features.inFloat(true);
	InsetCaptionable::validate(features);
	features.inFloat(false);
}


docstring InsetFloat::xhtml(XHTMLStream & xs, OutputParams const & rp) const
{
	FloatList const & floats = buffer().params().documentClass().floats();
	Floating const & ftype = floats.getType(params_.type);
	string const & htmltype = ftype.htmlTag();
	string const & attr = ftype.htmlAttrib();

	odocstringstream ods;
	XHTMLStream newxs(ods);
	newxs << html::StartTag(htmltype, attr);
	InsetText::XHTMLOptions const opts = 
		InsetText::WriteLabel | InsetText::WriteInnerTag;
	docstring deferred = InsetText::insetAsXHTML(newxs, rp, opts);
	newxs << html::EndTag(htmltype);

	if (rp.inFloat == OutputParams::NONFLOAT) {
		// In this case, this float needs to be deferred, but we'll put it
		// before anything the text itself deferred.
		deferred = ods.str() + '\n' + deferred;
	} else {
		// In this case, the whole thing is already being deferred, so
		// we can write to the stream.
		// Note that things will already have been escaped, so we do not 
		// want to escape them again.
		xs << XHTMLStream::ESCAPE_NONE << ods.str();
	}
	return deferred;
}


void InsetFloat::latex(otexstream & os, OutputParams const & runparams_in) const
{
	if (runparams_in.inFloat != OutputParams::NONFLOAT) {
		if (runparams_in.moving_arg)
			os << "\\protect";
		os << "\\subfloat";

		OutputParams rp = runparams_in;
		rp.moving_arg = true;
		getCaption(os, rp);
		os << '{';
		// The main argument is the contents of the float. This is not a moving argument.
		if (!paragraphs().empty())
			os.texrow().forceStart(paragraphs()[0].id(), 0);
		rp.moving_arg = false;
		rp.inFloat = OutputParams::SUBFLOAT;
		InsetText::latex(os, rp);
		os << "}";

		return;
	}
	OutputParams runparams(runparams_in);
	runparams.inFloat = OutputParams::MAINFLOAT;

	FloatList const & floats = buffer().params().documentClass().floats();
	string tmptype = params_.type;
	if (params_.sideways && floats.allowsSideways(params_.type))
		tmptype = "sideways" + params_.type;
	if (params_.wide && floats.allowsWide(params_.type)
		&& (!params_.sideways ||
		     params_.type == "figure" ||
		     params_.type == "table"))
		tmptype += "*";
	// Figure out the float placement to use.
	// From lowest to highest:
	// - float default placement
	// - document wide default placement
	// - specific float placement
	string tmpplacement;
	string const buf_placement = buffer().params().float_placement;
	string const def_placement = floats.defaultPlacement(params_.type);
	if (!params_.placement.empty()
	    && params_.placement != def_placement) {
		tmpplacement = params_.placement;
	} else if (params_.placement.empty()
		   && !buf_placement.empty()
		   && buf_placement != def_placement) {
		tmpplacement = buf_placement;
	}

	// Check if placement is allowed by this float
	string const allowed_placement =
		floats.allowedPlacement(params_.type);
	string placement;
	string::const_iterator lit = tmpplacement.begin();
	string::const_iterator end = tmpplacement.end();
	for (; lit != end; ++lit) {
		if (contains(allowed_placement, *lit))
			placement += *lit;
	}

	// Force \begin{<floatname>} to appear in a new line.
	os << breakln << "\\begin{" << from_ascii(tmptype) << '}';
	if (runparams.lastid != -1)
		os.texrow().start(runparams.lastid, runparams.lastpos);
	// We only output placement if different from the def_placement.
	// sidewaysfloats always use their own page
	if (!placement.empty() && !params_.sideways)
		os << '[' << from_ascii(placement) << ']';
	os << '\n';

	InsetText::latex(os, runparams);

	// Force \end{<floatname>} to appear in a new line.
	os << breakln << "\\end{" << from_ascii(tmptype) << "}\n";
}


int InsetFloat::plaintext(odocstringstream & os, OutputParams const & runparams, size_t max_length) const
{
	os << '[' << buffer().B_("float") << ' '
		<< floatName(params_.type) << ":\n";
	InsetText::plaintext(os, runparams, max_length);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetFloat::docbook(odocstream & os, OutputParams const & runparams) const
{
	// FIXME Implement subfloat!
	// FIXME UNICODE
	os << '<' << from_ascii(params_.type) << '>';
	int const i = InsetText::docbook(os, runparams);
	os << "</" << from_ascii(params_.type) << '>';

	return i;
}


bool InsetFloat::insetAllowed(InsetCode code) const
{
	// The case that code == FLOAT_CODE is handled in Text3.cpp, 
	// because we need to know what type of float is meant.
	switch(code) {
	case WRAP_CODE:
	case FOOT_CODE:
	case MARGIN_CODE:
		return false;
	default:
		return InsetCaptionable::insetAllowed(code);
	}
}


void InsetFloat::setWide(bool w, bool update_label)
{
	if (!buffer().params().documentClass().floats().allowsWide(params_.type))
		params_.wide = false;
	else
	    params_.wide = w;
	if (update_label)
		setNewLabel();
}


void InsetFloat::setSideways(bool s, bool update_label)
{
	if (!buffer().params().documentClass().floats().allowsSideways(params_.type))
		params_.sideways = false;
	else
		params_.sideways = s;
	if (update_label)
		setNewLabel();
}


void InsetFloat::setSubfloat(bool s, bool update_label)
{
	params_.subfloat = s;
	if (update_label)
		setNewLabel();
}


void InsetFloat::setNewLabel()
{
	docstring lab = _("float: ");

	if (params_.subfloat)
		lab = _("subfloat: ");

	lab += floatName(params_.type);

	FloatList const & floats = buffer().params().documentClass().floats();

	if (params_.wide && floats.allowsWide(params_.type))
		lab += '*';

	if (params_.sideways && floats.allowsSideways(params_.type))
		lab += _(" (sideways)");

	setLabel(lab);
}


bool InsetFloat::allowsCaptionVariation(std::string const & newtype) const
{
	return !params_.subfloat && newtype != "Unnumbered";
}


docstring InsetFloat::getCaption(OutputParams const & runparams) const
{
	odocstringstream ods;
	otexstream os(ods);
	getCaption(os, runparams);
	return ods.str();
}


void InsetFloat::getCaption(otexstream & os,
							OutputParams const & runparams) const
{
	if (paragraphs().empty())
		return;

	InsetCaption const * ins = getCaptionInset();
	if (ins == 0)
		return;

	ins->getArgs(os, runparams);

	os << '[';
	odocstringstream ods;
	otexstream oss(ods);
	ins->getArgument(oss, runparams);
	docstring arg = ods.str();
	// Protect ']'
	if (arg.find(']') != docstring::npos)
		arg = '{' + arg + '}';
	os.append(arg, move(oss.texrow()));
	os << ']';
}


void InsetFloat::string2params(string const & in, InsetFloatParams & params)
{
	params = InsetFloatParams();
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetFloat::string2params");
	params.read(lex);
}


string InsetFloat::params2string(InsetFloatParams const & params)
{
	ostringstream data;
	params.write(data);
	return data.str();
}


} // namespace lyx
