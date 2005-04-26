/**
 * \file insetcharstyle.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetcharstyle.h"

#include "BufferView.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "cursor.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "LColor.h"
#include "lyxlex.h"
#include "lyxtext.h"
#include "metricsinfo.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "sgml.h"

#include "frontends/font_metrics.h"
#include "frontends/Painter.h"

#include <sstream>


using std::string;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;


void InsetCharStyle::init()
{
	setInsetName("CharStyle");
	setStatus(Inlined);
	setDrawFrame(false);
	has_label_ = true;
}


InsetCharStyle::InsetCharStyle(BufferParams const & bp,
				CharStyles::iterator cs)
	: InsetCollapsable(bp)
{
	params_.type = cs->name;
	params_.latextype = cs->latextype;
	params_.latexname = cs->latexname;
	params_.latexparam = cs->latexparam;
	params_.font = cs->font;
	params_.labelfont = cs->labelfont;
	init();
}


InsetCharStyle::InsetCharStyle(InsetCharStyle const & in)
	: InsetCollapsable(in), params_(in.params_)
{
	init();
}


auto_ptr<InsetBase> InsetCharStyle::doClone() const
{
	return auto_ptr<InsetBase>(new InsetCharStyle(*this));
}


string const InsetCharStyle::editMessage() const
{
	return _("Opened CharStyle Inset");
}


void InsetCharStyle::write(Buffer const & buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetCharStyle::read(Buffer const & buf, LyXLex & lex)
{
	InsetCollapsable::read(buf, lex);
	setStatus(Inlined);
}


void InsetCharStyle::metrics(MetricsInfo & mi, Dimension & dim) const
{
	LyXFont tmpfont = mi.base.font;
	getDrawFont(mi.base.font);
	mi.base.font.reduce(LyXFont(LyXFont::ALL_SANE));
	mi.base.font.realize(tmpfont);
	mi.base.textwidth -= 2 * TEXT_TO_INSET_OFFSET;
	InsetText::metrics(mi, dim);
	mi.base.font = tmpfont;
	dim.asc += TEXT_TO_INSET_OFFSET;
	dim.des += TEXT_TO_INSET_OFFSET;
	dim.wid += 2 * TEXT_TO_INSET_OFFSET;
	mi.base.textwidth += 2 * TEXT_TO_INSET_OFFSET;
	dim_ = dim;
	if (has_label_)
		dim_.des += ascent();
}


void InsetCharStyle::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);

	LyXFont tmpfont = pi.base.font;
	getDrawFont(pi.base.font);
	// I don't understand why the above .reduce and .realize aren't
	//needed, or even wanted, here. It just works. -- MV 10.04.2005
	InsetText::draw(pi, x, y);
	pi.base.font = tmpfont;

	int desc = InsetText::descent();
	if (has_label_)
		desc -= ascent();

	pi.pain.line(x, y + desc - 4, x, y + desc, params_.labelfont.color());
	pi.pain.line(x, y + desc, x + dim_.wid - 3, y + desc,
		params_.labelfont.color());
	pi.pain.line(x + dim_.wid - 3, y + desc, x + dim_.wid - 3, y + desc - 4,
		params_.labelfont.color());

	// the name of the charstyle. Can be toggled.
	if (has_label_) {
		LyXFont font(params_.labelfont);
		font.realize(LyXFont(LyXFont::ALL_SANE));
		font.decSize();
		font.decSize();
		int w = 0;
		int a = 0;
		int d = 0;
		font_metrics::rectText(params_.type, font, w, a, d);
		pi.pain.rectText(x + (dim_.wid - w) / 2, y + desc + a,
			params_.type, font, LColor::none, LColor::none);
	}

	// a visual clue when the cursor is inside the inset
	LCursor & cur = pi.base.bv->cursor();
	if (cur.isInside(this)) {
		y -= ascent();
		pi.pain.line(x, y + 4, x, y, params_.labelfont.color());
		pi.pain.line(x + 4, y, x, y, params_.labelfont.color());
		pi.pain.line(x + dim_.wid - 3, y + 4, x + dim_.wid - 3, y,
			params_.labelfont.color());
		pi.pain.line(x + dim_.wid - 7, y, x + dim_.wid - 3, y,
			params_.labelfont.color());
	}
}


void InsetCharStyle::getDrawFont(LyXFont & font) const
{
	font = params_.font;
}


void InsetCharStyle::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	setStatus(Inlined);
	switch (cmd.action) {
		case LFUN_MOUSE_PRESS:
			if (cmd.button() == mouse_button::button3)
				has_label_ = !has_label_;
			else
				InsetText::doDispatch(cur, cmd);
			break;

		default:
			InsetCollapsable::doDispatch(cur, cmd);
			break;
	}
}


bool InsetCharStyle::getStatus(LCursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
		// paragraph breaks not allowed in charstyle insets
		case LFUN_BREAKPARAGRAPH:
		case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		case LFUN_BREAKPARAGRAPH_SKIP:
			status.enabled(false);
			return true;

		default:
			return InsetCollapsable::getStatus(cur, cmd, status);
		}
}


int InsetCharStyle::latex(Buffer const & buf, ostream & os,
		     OutputParams const & runparams) const
{
	os << "\\" << params_.latexname;
	if (!params_.latexparam.empty())
		os << params_.latexparam;
	os << "{";
	int i = InsetText::latex(buf, os, runparams);
	os << "}";
	return i;
}


int InsetCharStyle::linuxdoc(Buffer const & buf, ostream & os,
			     OutputParams const & runparams) const
{
	sgml::openTag(os, params_.latexname, params_.latexparam);
	int i = InsetText::linuxdoc(buf, os, runparams);
	sgml::closeTag(os, params_.latexname);
	return i;
}


int InsetCharStyle::docbook(Buffer const & buf, ostream & os,
			    OutputParams const & runparams) const
{
	ParagraphList::const_iterator par = paragraphs().begin();
        ParagraphList::const_iterator end = paragraphs().end();

	sgml::openTag(os, params_.latexname, par->getID(buf, runparams) + params_.latexparam);

        for (; par != end; ++par) {
		par->simpleDocBookOnePar(buf, os, runparams,
					 outerFont(par - paragraphs().begin(),
						   paragraphs()));
        }

	sgml::closeTag(os, params_.latexname);
	return 0;
}


int InsetCharStyle::plaintext(Buffer const & buf, ostream & os,
			      OutputParams const & runparams) const
{
	return InsetText::plaintext(buf, os, runparams);
}


void InsetCharStyle::validate(LaTeXFeatures & features) const
{
	features.require(params_.type);
	InsetText::validate(features);
}


void InsetCharStyleParams::write(ostream & os) const
{
	os << "CharStyle " << type << "\n";
}


void InsetCharStyleParams::read(LyXLex & lex)
{
	if (lex.isOK()) {
		lex.next();
		string token = lex.getString();
	}

	if (lex.isOK()) {
		lex.next();
		type = lex.getString();
	}
}
