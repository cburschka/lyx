/**
 * \file insetfloat.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */
#include <config.h>


#include "insetfloat.h"
#include "insettext.h"

#include "buffer.h"
#include "BufferView.h"
#include "debug.h"
#include "Floating.h"
#include "FloatList.h"
#include "funcrequest.h"
#include "gettext.h"
#include "iterators.h"
#include "LaTeXFeatures.h"
#include "lyxfont.h"
#include "lyxlex.h"
#include "lyxtext.h"

#include "support/LOstream.h"
#include "support/lstrings.h"

#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"

using std::ostream;
using std::endl;

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

namespace {

// this should not be hardcoded, but be part of the definition
// of the float (JMarc)
string const caplayout("Caption");

string floatname(string const & type, BufferParams const & bp)
{
	FloatList const & floats = bp.getLyXTextClass().floats();
	FloatList::const_iterator it = floats[type];
	if (it == floats.end())
		return type;

	return _(it->second.name());
}

} // namespace anon


InsetFloat::InsetFloat(BufferParams const & bp, string const & type)
	: InsetCollapsable(bp)
{
	string lab(_("float: "));
	lab += floatname(type, bp);
	setLabel(lab);
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::collapsable);
	setLabelFont(font);
	params_.type = type;
	setInsetName(type);
	LyXTextClass const & tclass = bp.getLyXTextClass();
	if (tclass.hasLayout(caplayout))
		inset.paragraph()->layout(tclass[caplayout]);
}


InsetFloat::InsetFloat(InsetFloat const & in, bool same_id)
	: InsetCollapsable(in, same_id), params_(in.params_)
{}


InsetFloat::~InsetFloat()
{
	InsetFloatMailer mailer(*this);
	mailer.hideDialog();
}


dispatch_result InsetFloat::localDispatch(FuncRequest const & cmd)
{
	Inset::RESULT result = UNDISPATCHED;

	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		InsetFloatParams params;
		InsetFloatMailer::string2params(cmd.argument, params);

		params_.placement = params.placement;
		params_.wide      = params.wide;

		cmd.view()->updateInset(this, true);
		result = DISPATCHED;
	}
	break;
	default:
		result = InsetCollapsable::localDispatch(cmd);
	}

	return result;
}


void InsetFloatParams::write(ostream & os) const
{
	os << "Float " // getInsetName()
	   << type << '\n';

	if (!placement.empty()) {
		os << "placement " << placement << "\n";
	}
	if (wide) {
		os << "wide true\n";
	} else {
		os << "wide false\n";
	}
}


void InsetFloatParams::read(LyXLex & lex)
{
	if (lex.isOK()) {
		lex.next();
		string token = lex.getString();
		if (token == "placement") {
			lex.next();
			placement = lex.getString();
		} else {
			// take countermeasures
			lex.pushToken(token);
		}
		lex.next();
		token = lex.getString();
		if (token == "wide") {
			lex.next();
			string const tmptoken = lex.getString();
			wide = (tmptoken == "true");
		} else {
			lyxerr << "InsetFloat::Read:: Missing wide!"
			       << endl;
			// take countermeasures
			lex.pushToken(token);
		}
	}
}


void InsetFloat::write(Buffer const * buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetFloat::read(Buffer const * buf, LyXLex & lex)
{
	params_.read(lex);
	wide(params_.wide, buf->params);
	InsetCollapsable::read(buf, lex);
}


void InsetFloat::validate(LaTeXFeatures & features) const
{
	if (contains(placement(), "H")) {
		features.require("float");
	}

	features.useFloat(params_.type);
	InsetCollapsable::validate(features);
}


Inset * InsetFloat::clone(Buffer const &, bool same_id) const
{
	return new InsetFloat(*const_cast<InsetFloat *>(this), same_id);
}


string const InsetFloat::editMessage() const
{
	return _("Opened Float Inset");
}


int InsetFloat::latex(Buffer const * buf,
		      ostream & os, bool fragile, bool fp) const
{
	FloatList const & floats = buf->params.getLyXTextClass().floats();
	string const tmptype = (params_.wide ? params_.type + "*" : params_.type);
	// Figure out the float placement to use.
	// From lowest to highest:
	// - float default placement
	// - document wide default placement
	// - specific float placement
	string placement;
	string const buf_placement = buf->params.float_placement;
	string const def_placement = floats.defaultPlacement(params_.type);
	if (!params_.placement.empty()
	    && params_.placement != def_placement) {
		placement = params_.placement;
	} else if (params_.placement.empty()
		   && !buf_placement.empty()
		   && buf_placement != def_placement) {
		placement = buf_placement;
	}

	// The \n is used to force \begin{<floatname>} to appear in a new line.
	// The % is needed to prevent two consecutive \n chars in the case
	// when the current output line is empty.
	os << "%\n\\begin{" << tmptype << '}';
	// We only output placement if different from the def_placement.
	if (!placement.empty()) {
		os << '[' << placement << ']';
	}
	os << '\n';

	int const i = inset.latex(buf, os, fragile, fp);

	// The \n is used to force \end{<floatname>} to appear in a new line.
	// In this case, we do not case if the current output line is empty.
	os << "\n\\end{" << tmptype << "}\n";

	return i + 4;
}


int InsetFloat::docbook(Buffer const * buf, ostream & os, bool mixcont) const
{
	os << '<' << params_.type << '>';
	int const i = inset.docbook(buf, os, mixcont);
	os << "</" << params_.type << '>';

	return i;
}


bool InsetFloat::insetAllowed(Inset::Code code) const
{
	if (code == Inset::FLOAT_CODE)
		return false;
	if (inset.getLockingInset() != const_cast<InsetFloat *>(this))
		return inset.insetAllowed(code);
	if ((code == Inset::FOOT_CODE) || (code == Inset::MARGIN_CODE))
		return false;
	return true;
}


bool InsetFloat::showInsetDialog(BufferView * bv) const
{
	if (!inset.showInsetDialog(bv)) {
		InsetFloat * tmp = const_cast<InsetFloat *>(this);
		InsetFloatMailer mailer(*tmp);
		mailer.showDialog();
	}
	return true;
}


string const & InsetFloat::type() const
{
	return params_.type;
}


void InsetFloat::placement(string const & p)
{
	// FIX: Here we should only allow the placement to be set
	// if a valid value.
	params_.placement = p;
}


string const & InsetFloat::placement() const
{
	return params_.placement;
}


void InsetFloat::wide(bool w, BufferParams const & bp)
{
	params_.wide = w;

	string lab(_("float:"));
	lab += floatname(params_.type, bp);

	if (params_.wide)
		lab += '*';

	setLabel(lab);
}


bool InsetFloat::wide() const
{
	return params_.wide;
}


void InsetFloat::addToToc(toc::TocList & toclist, Buffer const * buf) const
{
	ParIterator pit(inset.paragraph());
	ParIterator end;

	// Find a caption layout in one of the (child inset's) pars
	for (; pit != end; ++pit) {
		Paragraph * tmp = *pit;

		if (tmp->layout()->name() == caplayout) {
			string const name = floatname(type(), buf->params);
			string const str =
				tostr(toclist[name].size() + 1)
				+ ". " + tmp->asString(buf, false);
			toc::TocItem const item(tmp->id(), 0 , str);
			toclist[name].push_back(item);
		}
	}
}


InsetFloatMailer::InsetFloatMailer(InsetFloat & inset)
	: name_("float"), inset_(inset)
{}


string const InsetFloatMailer::inset2string() const
{
	return params2string(name(), inset_.params());
}


void InsetFloatMailer::string2params(string const & in,
				     InsetFloatParams & params)
{
	params = InsetFloatParams();

	string name;
	string body = split(in, name, ' ');

	if (name != "float" || body.empty())
		return;

	// This is part of the inset proper that is usually swallowed
	// by Buffer::readInset
	body = split(body, name, '\n');
	if (!prefixIs(name, "Float "))
		return;

	istringstream data(body);
	LyXLex lex(0,0);
	lex.setStream(data);

	params.read(lex);
}


string const
InsetFloatMailer::params2string(string const & name,
				InsetFloatParams const & params)
{
	ostringstream data;
	data << name << ' ';
	params.write(data);

	return data.str();
}
