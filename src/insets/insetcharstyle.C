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

#include "support/std_sstream.h"


using std::string;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;


void InsetCharStyle::init()
{
	setInsetName("CharStyle");
	setButtonLabel();
}


InsetCharStyle::InsetCharStyle(BufferParams const & bp,
				CharStyles::iterator cs)
	: InsetCollapsable(bp)
{
	params_.type = cs->name;
	params_.latextype = cs->latextype;
	params_.latexname = cs->latexname;
	params_.font = cs->font;
	params_.labelfont = cs->labelfont;
	init();
}


InsetCharStyle::InsetCharStyle(InsetCharStyle const & in)
	: InsetCollapsable(in), params_(in.params_)
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
	setButtonLabel();
}


void InsetCharStyle::setButtonLabel()
{
	LyXFont font(params_.labelfont);
	font.realize(LyXFont(LyXFont::ALL_SANE));
	string const s = "Style: " + params_.type;
	setLabel(isOpen() ? s : getNewLabel(s) );
	setLabelFont(font);
}


void InsetCharStyle::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetCollapsable::metrics(mi, dim);
	dim_ = dim;
}


void InsetCharStyle::getDrawFont(LyXFont & font) const
{
	font = params_.font;
}


DispatchResult
InsetCharStyle::priv_dispatch(FuncRequest const & cmd,
			idx_type & idx, pos_type & pos)
{
	DispatchResult dr = InsetCollapsable::priv_dispatch(cmd, idx, pos);
	setButtonLabel();
	return dr;
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
	os << "%\n\\" << params_.latexname << "{";
	int i = outputVerbatim(os, inset);
	os << "}%\n";
		i += 2;
	return i;
}


int InsetCharStyle::linuxdoc(Buffer const &, std::ostream & os,
			     OutputParams const &) const
{
	os << "<" << params_.latexname << ">";
	int const i = outputVerbatim(os, inset);
	os << "</" << params_.latexname << ">";
	return i;
}


int InsetCharStyle::docbook(Buffer const &, std::ostream & os,
			    OutputParams const &) const
{
	os << "<" << params_.latexname << ">";
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
