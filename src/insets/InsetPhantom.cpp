/**
 * \file InsetPhantom.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetPhantom.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "BufferParams.h"
#include "Counters.h"
#include "Cursor.h"
#include "Dimension.h"
#include "DispatchResult.h"
#include "Exporter.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetIterator.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "TextClass.h"

#include "support/docstream.h"
#include "support/gettext.h"
#include "support/Translator.h"

#include "frontends/Application.h"
#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include <algorithm>
#include <sstream>

using namespace std;

namespace lyx {

namespace {

typedef Translator<string, InsetPhantomParams::Type> PhantomTranslator;
typedef Translator<docstring, InsetPhantomParams::Type> PhantomTranslatorLoc;

PhantomTranslator const init_phantomtranslator()
{
	PhantomTranslator translator("Phantom", InsetPhantomParams::Phantom);
	translator.addPair("HPhantom", InsetPhantomParams::HPhantom);
	translator.addPair("VPhantom", InsetPhantomParams::VPhantom);
	return translator;
}


PhantomTranslatorLoc const init_phantomtranslator_loc()
{
	PhantomTranslatorLoc translator(_("Phantom"), InsetPhantomParams::Phantom);
	translator.addPair(_("HPhantom"), InsetPhantomParams::HPhantom);
	translator.addPair(_("VPhantom"), InsetPhantomParams::VPhantom);
	return translator;
}


PhantomTranslator const & phantomtranslator()
{
	static PhantomTranslator translator = init_phantomtranslator();
	return translator;
}


PhantomTranslatorLoc const & phantomtranslator_loc()
{
	static PhantomTranslatorLoc translator = init_phantomtranslator_loc();
	return translator;
}

} // anon


InsetPhantomParams::InsetPhantomParams()
	: type(Phantom)
{}


void InsetPhantomParams::write(ostream & os) const
{
	string const label = phantomtranslator().find(type);
	os << "Phantom " << label << "\n";
}


void InsetPhantomParams::read(Lexer & lex)
{
	string label;
	lex >> label;
	if (lex)
		type = phantomtranslator().find(label);
}


/////////////////////////////////////////////////////////////////////
//
// InsetPhantom
//
/////////////////////////////////////////////////////////////////////

InsetPhantom::InsetPhantom(Buffer const & buf, string const & label)
	: InsetCollapsable(buf)
{
	params_.type = phantomtranslator().find(label);
}


InsetPhantom::~InsetPhantom()
{
	hideDialogs("phantom", this);
}


docstring InsetPhantom::editMessage() const
{
	return _("Opened Phantom Inset");
}


docstring InsetPhantom::name() const 
{
	return from_ascii("Phantom:" + phantomtranslator().find(params_.type));
}


Inset::DisplayType InsetPhantom::display() const
{
	return Inline;
}


void InsetPhantom::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetText::metrics(mi, dim);

	// cache the inset dimension
	setDimCache(mi, dim);
}


void InsetPhantom::draw(PainterInfo & pi, int x, int y) const
{
	// draw the text
	InsetText::draw(pi, x, y);

	// draw the inset marker
	drawMarkers(pi, x, y);
	
	// draw the arrow(s)
	static int const arrow_size = 4;
	ColorCode const origcol = pi.base.font.color();
	pi.base.font.setColor(Color_special);
	pi.base.font.setColor(origcol);
	Dimension const dim = dimension(*pi.base.bv);

	if (params_.type == InsetPhantomParams::Phantom ||
		params_.type == InsetPhantomParams::VPhantom) {
		// y1---------
		//           / \.
		// y2-----  / | \.
		//            |
		//            |
		// y3-----  \ | /
		//           \ /
		// y4---------
		//          | | |
		//         /  |  \.
		//        x1  x2 x3

		int const x2 = x + dim.wid / 2;
		int const x1 = x2 - arrow_size;
		int const x3 = x2 + arrow_size;

		int const y1 = y - dim.asc;
		int const y2 = y1 + arrow_size;
		int const y4 = y + dim.des;
		int const y3 = y4 - arrow_size;

		// top arrow
		pi.pain.line(x2, y1, x1, y2, Color_added_space);
		pi.pain.line(x2, y1, x3, y2, Color_added_space);

		// bottom arrow
		pi.pain.line(x2, y4, x1, y3, Color_added_space);
		pi.pain.line(x2, y4, x3, y3, Color_added_space);

		// joining line
		pi.pain.line(x2, y1, x2, y4, Color_added_space);
	}

	if (params_.type == InsetPhantomParams::Phantom ||
		params_.type == InsetPhantomParams::HPhantom) {
		// y1----   /          \.
		//        /              \.
		// y2--- <---------------->
		//        \              /
		// y3----   \          /
		//       |   |        |   |
		//      x1  x2       x3  x4

		x = x + TEXT_TO_INSET_OFFSET;
		int const x1 = x;
		int const x2 = x + arrow_size;
		int const x4 = x + dim.wid - 2 * TEXT_TO_INSET_OFFSET;
		int const x3 = x4 - arrow_size;

		int const y2 = y + (dim.des - dim.asc) / 2;
		int const y1 = y2 - arrow_size;
		int const y3 = y2 + arrow_size;

		// left arrow
		pi.pain.line(x1, y2, x2, y3, Color_added_space);
		pi.pain.line(x1, y2, x2, y1, Color_added_space);

		// right arrow
		pi.pain.line(x4, y2, x3, y3, Color_added_space);
		pi.pain.line(x4, y2, x3, y1, Color_added_space);

		// joining line
		pi.pain.line(x1, y2, x4, y2, Color_added_space);
	}
}


void InsetPhantom::write(ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(os);
}


void InsetPhantom::read(Lexer & lex)
{
	params_.read(lex);
	InsetCollapsable::read(lex);
}


void InsetPhantom::setButtonLabel()
{
	docstring const label = phantomtranslator_loc().find(params_.type);
	setLabel(label);
}


bool InsetPhantom::showInsetDialog(BufferView * bv) const
{
	bv->showDialog("phantom", params2string(params()),
		const_cast<InsetPhantom *>(this));
	return true;
}


void InsetPhantom::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY:
		string2params(to_utf8(cmd.argument()), params_);
		setLayout(buffer().params());
		break;

	case LFUN_INSET_DIALOG_UPDATE:
		cur.bv().updateDialog("phantom", params2string(params()));
		break;

	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetPhantom::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY:
		flag.setEnabled(true);
		return true;

	case LFUN_INSET_DIALOG_UPDATE:
		flag.setEnabled(true);
		return true;

	default:
		return InsetCollapsable::getStatus(cur, cmd, flag);
	}
}


int InsetPhantom::latex(odocstream & os, OutputParams const & runparams_in) const
{
	OutputParams runparams(runparams_in);
	if (params_.type == InsetPhantomParams::Phantom)
		os << "\\phantom{";
	else if (params_.type == InsetPhantomParams::HPhantom)
		os << "\\hphantom{";
	else if (params_.type == InsetPhantomParams::VPhantom)
		os << "\\vphantom{";
	int const i = InsetText::latex(os, runparams);
	os << "}";
	runparams_in.encoding = runparams.encoding;

	return i + 2;
}


int InsetPhantom::plaintext(odocstream & os,
			 OutputParams const & runparams_in) const
{
	OutputParams runparams(runparams_in);
	if (params_.type == InsetPhantomParams::Phantom)
		os << '[' << buffer().B_("phantom") << ":";
	else if (params_.type == InsetPhantomParams::HPhantom)
		os << '[' << buffer().B_("hphantom") << ":";
	else if (params_.type == InsetPhantomParams::VPhantom)
		os << '[' << buffer().B_("vphantom") << ":";
	InsetText::plaintext(os, runparams);
	os << "]";

	return PLAINTEXT_NEWLINE;
}


int InsetPhantom::docbook(odocstream & os, OutputParams const & runparams_in) const
{
	OutputParams runparams(runparams_in);
	string cmdname;
	if (params_.type == InsetPhantomParams::Phantom)
		cmdname = "phantom";
	else if (params_.type == InsetPhantomParams::HPhantom)
		cmdname = "phantom";
	else if (params_.type == InsetPhantomParams::VPhantom)
		cmdname = "phantom";
	os << "<" + cmdname + ">";
	int const i = InsetText::docbook(os, runparams);
	os << "</" + cmdname + ">";

	return i;
}


docstring InsetPhantom::contextMenu(BufferView const &, int, int) const
{
	return from_ascii("context-phantom");
}


string InsetPhantom::params2string(InsetPhantomParams const & params)
{
	ostringstream data;
	data << "phantom" << ' ';
	params.write(data);
	return data.str();
}


void InsetPhantom::string2params(string const & in, InsetPhantomParams & params)
{
	params = InsetPhantomParams();

	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetPhantom::string2params");
	lex >> "phantom" >> "Phantom";

	params.read(lex);
}


} // namespace lyx
