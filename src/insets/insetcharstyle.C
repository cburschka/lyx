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
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "LColor.h"
#include "lyxlex.h"
#include "lyxtext.h"
#include "metricsinfo.h"
#include "paragraph.h"

#include "frontends/font_metrics.h"
#include "frontends/Painter.h"
#include "support/std_sstream.h"


using std::string;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;


void InsetCharStyle::init()
{
	setInsetName("CharStyle");
	setStatus(Inlined);
}


InsetCharStyle::InsetCharStyle(BufferParams const & bp,
				CharStyles::iterator cs)
	: InsetCollapsable(bp), has_label_(true)
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
	: InsetCollapsable(in), params_(in.params_), has_label_(true)
{
	init();
}


auto_ptr<InsetBase> InsetCharStyle::clone() const
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
	InsetCollapsable::metrics(mi, dim);
	mi.base.font = tmpfont;
	dim_ = dim;
	if (has_label_)
		dim_.des += ascent();
}


void InsetCharStyle::draw(PainterInfo & pi, int x, int y) const
{
	xo_ = x;
	yo_ = y;

	// FIXME: setStatus(Inlined); this is not a const operation
	LyXFont tmpfont = pi.base.font;
	inset.setDrawFrame(InsetText::NEVER);
	getDrawFont(pi.base.font);
	inset.draw(pi, x, y);
	pi.base.font = tmpfont;

	pi.pain.line(x + 2, y + inset.descent() - 4, x + 2,
		y + inset.descent(), params_.labelfont.color());
	pi.pain.line(x + 2, y + inset.descent(), x + dim_.wid - 2,
		y + inset.descent(), params_.labelfont.color());
	pi.pain.line(x + dim_.wid - 2, y + inset.descent(), x + dim_.wid - 2,
		y + inset.descent() - 4, params_.labelfont.color());

	if (has_label_) {
		if (!owner())
			x += scroll();

	LyXFont font(params_.labelfont);
	font.realize(LyXFont(LyXFont::ALL_SANE));
	font.decSize();
	font.decSize();
	int w = 0;
	int a = 0;
	int d = 0;
	font_metrics::rectText(params_.type, font, w, a, d);
	pi.pain.rectText(x + 0.5 * (dim_.wid - w), 
		y + inset.descent() + a,
		params_.type, font, LColor::none, LColor::none);
	}
}


void InsetCharStyle::getDrawFont(LyXFont & font) const
{
	font = params_.font;
}


DispatchResult
InsetCharStyle::priv_dispatch(FuncRequest const & cmd,
			idx_type & idx, pos_type & pos)
{
	setStatus(Inlined);
	switch (cmd.action) {
		case LFUN_MOUSE_PRESS:
			if (cmd.button() == mouse_button::button3) {
				has_label_ = !has_label_;
				return DispatchResult(true);
			}
			inset.dispatch(cmd);
			return DispatchResult(true, true);
			break;
		default:
			return InsetCollapsable::priv_dispatch(cmd, idx, pos);
	}
}


namespace {

int outputVerbatim(std::ostream & os, InsetText inset)
{
	int lines = 0;
	ParagraphList::iterator par = inset.paragraphs().begin();
	ParagraphList::iterator end = inset.paragraphs().end();
	while (par != end) {
		lyx::pos_type siz = par->size();
		for (lyx::pos_type i = 0; i < siz; ++i) {
			if (par->isNewline(i)) {
				os << '\n';
				++lines;
			} else {
				os << par->getChar(i);
			}
		}
		++par;
		if (par != end) {
			os << "\n";
			lines ++;
		}
	}
	return lines;
}

} // namespace anon


int InsetCharStyle::latex(Buffer const &, ostream & os,
		     OutputParams const &) const
{
	os << "%\n\\" << params_.latexname;
	if (!params_.latexparam.empty())
		os << params_.latexparam;
	os << "{";
	int i = outputVerbatim(os, inset);
	os << "}%\n";
		i += 2;
	return i;
}


int InsetCharStyle::linuxdoc(Buffer const &, std::ostream & os,
			     OutputParams const &) const
{
	os << "<" << params_.latexname;
	if (!params_.latexparam.empty())
		os << " " << params_.latexparam;
	os << ">";
	int const i = outputVerbatim(os, inset);
	os << "</" << params_.latexname << ">";
	return i;
}


int InsetCharStyle::docbook(Buffer const &, std::ostream & os,
			    OutputParams const &) const
{
	os << "<" << params_.latexname;
	if (!params_.latexparam.empty())
		os << " " << params_.latexparam;
	os << ">";
	int const i = outputVerbatim(os, inset);
	os << "</" << params_.latexname << ">";
	return i;
}


int InsetCharStyle::plaintext(Buffer const &, std::ostream & os,
			      OutputParams const & runparams) const
{
	return outputVerbatim(os, inset);
}


void InsetCharStyle::validate(LaTeXFeatures & features) const
{
	features.require(params_.type);
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
