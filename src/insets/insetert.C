/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1998 The LyX Team.
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetert.h"
#include "gettext.h"
#include "lyxfont.h"
#include "buffer.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "lyx_gui_misc.h"
#include "BufferView.h"
#include "LyXView.h"

using std::ostream;

void InsetERT::init()
{
	setButtonLabel();
	labelfont = LyXFont(LyXFont::ALL_SANE);
	labelfont.decSize();
	labelfont.decSize();
	labelfont.setColor(LColor::latex);
	setInsetName("ERT");
}


InsetERT::InsetERT() : InsetCollapsable()
{
	init();
}


InsetERT::InsetERT(InsetERT const & in, bool same_id)
	: InsetCollapsable(in, same_id)
{
	init();
}


Inset * InsetERT::clone(Buffer const &, bool same_id) const
{
	return new InsetERT(*const_cast<InsetERT *>(this), same_id);
}


InsetERT::InsetERT(string const & contents, bool collapsed)
	: InsetCollapsable(collapsed)
{
	LyXFont font(LyXFont::ALL_INHERIT);
	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);
	string::const_iterator cit = contents.begin();
	string::const_iterator end = contents.end();
	Paragraph::size_type pos = 0;
	for (; cit != end; ++cit) {
		inset.paragraph()->insertChar(pos++, *cit, font);
	}
	// the init has to be after the initialization of the paragraph
	// because of the label settings (draw_label for ert insets).
	init();
}


void InsetERT::read(Buffer const * buf, LyXLex & lex)
{
	InsetCollapsable::read(buf, lex);

	setButtonLabel();
}


void InsetERT::write(Buffer const * buf, ostream & os) const 
{
	os << getInsetName() << "\n";
	InsetCollapsable::write(buf, os);
}


string const InsetERT::editMessage() const 
{
	return _("Opened ERT Inset");
}


bool InsetERT::insertInset(BufferView *, Inset *)
{
	return false;
}


void InsetERT::setFont(BufferView *, LyXFont const &, bool, bool selectall)
{
	// if selectall is activated then the fontchange was an outside general
	// fontchange and this messages is not needed
	if (!selectall)
		WriteAlert(_("Impossible Operation!"),
		           _("Not permitted to change font-types inside ERT-insets!"),
		           _("Sorry."));
}


void InsetERT::edit(BufferView * bv, int x, int y, unsigned int button)
{
	InsetCollapsable::edit(bv, x, y, button);

	LyXFont font(LyXFont::ALL_INHERIT);
	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);

	inset.setFont(bv, font);
}


void InsetERT::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, 0);
}


void InsetERT::insetButtonRelease(BufferView * bv,
				  int x, int y, int button)
{
	if ((x >= 0)  && (x < button_length) &&
	    (y >= button_top_y) &&  (y <= button_bottom_y)) {
		if (collapsed_) {
			setLabel(_("ERT"));
		} else {
			setLabel(get_new_label());
		}
	}
	InsetCollapsable::insetButtonRelease(bv, x, y, button);
}


int InsetERT::latex(Buffer const *, std::ostream & os, bool /*fragile*/,
		    bool /*free_spc*/) const
{
	Paragraph * par = inset.paragraph();
	while (par) {
		Paragraph::size_type siz = inset.paragraph()->size();
		for (Paragraph::size_type i = 0; i != siz; ++i) {
			char c = inset.paragraph()->getChar(i);
			switch (c) {
			case Paragraph::META_NEWLINE:
				os << '\n';
				break;
			default:
				os << c;
				break;
			}
		}
		par = par->next();
	}
	
	return 1;
}


int InsetERT::ascii(Buffer const *,
		    std::ostream &, int /*linelen*/) const 
{
	return 0;
}


int InsetERT::linuxdoc(Buffer const *, std::ostream &) const
{
	return 0;
}


int InsetERT::docBook(Buffer const *, std::ostream &) const
{
	return 0;
}


UpdatableInset::RESULT
InsetERT::localDispatch(BufferView * bv, kb_action action, string const & arg)
{
	UpdatableInset::RESULT result = DISPATCHED_NOUPDATE;
	
	switch(action) {
	case LFUN_LAYOUT:
		bv->owner()->setLayout(inset.paragraph()->getLayout());
		break;
	default:
		result = InsetCollapsable::localDispatch(bv, action, arg);
	}
	switch(action) {
	case LFUN_BREAKPARAGRAPH:
	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
	{
		LyXFont font(LyXFont::ALL_INHERIT);
		font.setFamily(LyXFont::TYPEWRITER_FAMILY);
		font.setColor(LColor::latex);
		inset.setFont(bv, font);
	}
	break;
	
	default:
		break;
	}
	return result;
}


string const InsetERT::get_new_label() const
{
	string la;
	Paragraph::size_type const max_length = 15;

	Paragraph::size_type const p_siz = inset.paragraph()->size();
	Paragraph::size_type const n = std::min(max_length, p_siz);
	int i = 0;
	int j = 0;
	for(; i < n && j < p_siz; ++j) {
		if (inset.paragraph()->isInset(j))
			continue;
		la += inset.paragraph()->getChar(j);
		++i;
	}
	if (i > 0 && j < p_siz) {
		la += "...";
	}
	if (la.empty()) {
		la = _("ERT");
	}
	return la;
}


void InsetERT::setButtonLabel() 
{
	if (collapsed_) {
		setLabel(get_new_label());
	} else {
		setLabel(_("ERT"));
	}
}


bool InsetERT::checkInsertChar(LyXFont & font)
{
	LyXFont f(LyXFont::ALL_INHERIT);
	font = f;
	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);
	return true;
}
