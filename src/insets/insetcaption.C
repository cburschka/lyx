/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetcaption.h"
#include "Painter.h"
#include "font.h"
#include "BufferView.h"
#include "FloatList.h"
#include "insets/insetfloat.h"
#include "debug.h"
#include "gettext.h"

using std::ostream;
using std::endl;

InsetCaption::InsetCaption()
	: InsetText()
{
	SetAutoBreakRows(true);
	SetDrawFrame(0, InsetText::LOCKED);
	SetFrameColor(0, LColor::footnoteframe);
}


void InsetCaption::Write(Buffer const * buf, ostream & os) const
{
	os << "Caption\n";
	WriteParagraphData(buf, os);
}



void InsetCaption::Read(Buffer const * buf, LyXLex & lex)
{
#if 0
	// We will enably this check again when the compability
	// code is removed from Buffer::Read (Lgb)
	string const token = lex.GetString();
	if (token != "Caption") {
		lyxerr << "InsetCaption::Read: consistency check failed."
		       << endl;
	}
#endif
	InsetText::Read(buf, lex);
}


string const InsetCaption::EditMessage() const 
{
	return _("Opened Caption Inset");
}


void InsetCaption::draw(BufferView * bv, LyXFont const & f,
			int baseline, float & x, bool cleared) const
{
	// We must draw the label, we should get the label string
	// from the enclosing float inset.
	// The question is: Who should draw the label, the caption inset,
	// the text inset or the paragraph?
	// We should also draw the float number (Lgb)

	// See if we can find the name of the float this caption
	// belongs to.
	Inset * i1 = owner();
	Inset * i2 = i1 ? i1->owner() : 0;
	string const type = static_cast<InsetFloat *>(i2)->type();
	string const fl = i2 ? floatList.getType(type).name() : N_("Float");

	// Discover the number...
	// ...
	string const num = "#";

	// Generate the label
	string const label = _(fl) + " " + num + ":";
	
	Painter & pain = bv->painter();
	int const w = lyxfont::width(label, f);
	pain.text(int(x), baseline, label, f);
	x += w;

	InsetText::draw(bv, f, baseline, x, cleared);
}


int InsetCaption::Latex(Buffer const * buf, ostream & os,
			bool fragile, bool free_spc) const
{
	// This is a bit too simplistic to take advantage of
	// caption options we must add more later. (Lgb)
	// This code is currently only able to handle the simple
	// \caption{...}, later we will make it take advantage
	// of the one of the caption packages. (Lgb)
	ostringstream ost;
	int const l = InsetText::Latex(buf, ost, fragile, free_spc);
	os << "\\caption{" << ost.str() << "}\n";
	return l + 1;
}


int InsetCaption::Ascii(Buffer const * /*buf*/,
			ostream & /*os*/, int /*linelen*/) const
{
#warning Implement me!
	return 0;
}


int InsetCaption::DocBook(Buffer const * /*buf*/, ostream & /*os*/) const
{
#warning Implement me!
	return 0;
}
