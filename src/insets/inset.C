/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "inset.h"
#include "debug.h"
#include "BufferView.h"
#include "support/lstrings.h"
#include "Painter.h"
#include "commandtags.h"
#include "support/lstrings.h"
#include "gettext.h"
#include "lyxfont.h"
#include "lyxcursor.h"
#include "lyxtext.h"

using std::endl;

// Insets default methods

// Initialization of the counter for the inset id's,
unsigned int Inset::inset_id = 0;

Inset::Inset()
	: top_x(0), topx_set(false), top_baseline(0), scx(0),
	  id_(inset_id++), owner_(0), par_owner_(0),
	  background_color_(LColor::inherit)
{}


Inset::Inset(Inset const & in, bool same_id)
	: top_x(0), topx_set(false), top_baseline(0), scx(0), owner_(0),
	  name_(in.name_), background_color_(in.background_color_)
{
	if (same_id)
		id_ = in.id();
	else
		id_ = inset_id++;
}


bool Inset::deletable() const
{
	return true;
}


bool Inset::directWrite() const
{
	return false;
}


Inset::EDITABLE Inset::editable() const
{
	return NOT_EDITABLE;
}


void Inset::validate(LaTeXFeatures &) const
{}


bool Inset::autoDelete() const
{
	return false;
}


void Inset::edit(BufferView *, int, int, unsigned int)
{}


void Inset::edit(BufferView *, bool)
{}


#if 0
LyXFont const Inset::convertFont(LyXFont const & font) const
{
#if 1
	return font;
#else
	return LyXFont(font);
#endif
}
#endif


string const Inset::editMessage() const 
{
	return _("Opened inset");
}


LyXText * Inset::getLyXText(BufferView const * bv, bool const) const
{
	if (owner())
		return owner()->getLyXText(bv, false);
	else
		return bv->text;
}


void Inset::setBackgroundColor(LColor::color color)
{
	background_color_ = color;
}


LColor::color Inset::backgroundColor() const
{
	if (background_color_ == LColor::inherit) {
		if (owner())
			return owner()->backgroundColor();
		else
			return LColor::background;
	} else
		return background_color_;
}


int Inset::id() const
{
	return id_;
}

void Inset::id(int id_arg)
{
	id_ = id_arg;
}

void Inset::setFont(BufferView *, LyXFont const &, bool, bool )
{}

// some stuff for inset locking

UpdatableInset::UpdatableInset()
	: Inset(), cursor_visible_(false), block_drawing_(false)
{}


UpdatableInset::UpdatableInset(UpdatableInset const & in, bool same_id)
	: Inset(in, same_id), cursor_visible_(false), block_drawing_(false)
{}


void UpdatableInset::insetButtonPress(BufferView *, int x, int y, int button)
{
	lyxerr[Debug::INFO] << "Inset Button Press x=" << x
		       << ", y=" << y << ", button=" << button << endl;
}


void UpdatableInset::insetButtonRelease(BufferView *, int x, int y, int button)
{
	lyxerr[Debug::INFO] << "Inset Button Release x=" << x
		       << ", y=" << y << ", button=" << button << endl;
}


void UpdatableInset::insetKeyPress(XKeyEvent *)
{
	lyxerr[Debug::INFO] << "Inset Keypress" << endl;
}


void UpdatableInset::insetMotionNotify(BufferView *, int x, int y, int state)
{
	lyxerr[Debug::INFO] << "Inset Motion Notify x=" << x
		       << ", y=" << y << ", state=" << state << endl;
}


void UpdatableInset::insetUnlock(BufferView *)
{
	lyxerr[Debug::INFO] << "Inset Unlock" << endl;
}


// An updatable inset is highly editable by definition
Inset::EDITABLE UpdatableInset::editable() const
{
	return HIGHLY_EDITABLE;
}


void UpdatableInset::toggleInsetCursor(BufferView *)
{}


void UpdatableInset::showInsetCursor(BufferView *, bool)
{}


void UpdatableInset::hideInsetCursor(BufferView *)
{}


void UpdatableInset::fitInsetCursor(BufferView *) const
{}


void UpdatableInset::edit(BufferView *, int, int, unsigned int)
{}


void UpdatableInset::edit(BufferView *, bool)
{}


void UpdatableInset::draw(BufferView *, LyXFont const &,
			  int /* baseline */, float & x,
			  bool/*cleared*/) const
{
	x += float(scx);
	// ATTENTION: don't do the following here!!!
	//    top_x = int(x);
	//    top_baseline = baseline;
}


void UpdatableInset::scroll(BufferView * bv, float s) const
{
	LyXFont font;
	
	if (!s) {
		scx = 0;
		return;
	}

	int const workW = bv->workWidth();
	int const tmp_top_x = top_x - scx;
	
	if (tmp_top_x > 0 && 
	    (tmp_top_x + width(bv, font)) < workW)
		return;
	if (s > 0 && top_x > 0)
		return;

	// int mx_scx=abs((width(bv,font) - bv->workWidth())/2);
	//int const save_scx = scx;
    
	scx = int(s * workW / 2);
	// if (!display())
	// scx += 20;

	if ((tmp_top_x + scx + width(bv, font)) < (workW / 2)) {
		scx += (workW / 2) - (tmp_top_x + scx + width(bv, font));
	}

	// bv->updateInset(const_cast<UpdatableInset *>(this), false);
}

void UpdatableInset::scroll(BufferView * bv, int offset) const
{
	if (offset > 0) {
		if (!scx && top_x >= 20)
			return;
		if ((top_x + offset) > 20)
			scx = 0;
		// scx += offset - (top_x - scx + offset - 20);
		else
			scx += offset;
	} else {
		LyXFont const font;
		if (!scx && (top_x + width(bv, font)) < (bv->workWidth() - 20))
			return;
		if ((top_x - scx + offset + width(bv, font)) < (bv->workWidth() - 20)) {
			scx = bv->workWidth() - width(bv, font) - top_x + scx - 20; 
		} else {
			scx += offset;
		}
	}
//	bv->updateInset(const_cast<UpdatableInset *>(this), false);
}


///  An updatable inset could handle lyx editing commands
UpdatableInset::RESULT
UpdatableInset::localDispatch(BufferView * bv, 
			      kb_action action, string const & arg) 
{
	if (!arg.empty() && (action==LFUN_SCROLL_INSET)) {
		if (arg.find('.') != arg.npos) {
			float const xx = static_cast<float>(strToDbl(arg));
			scroll(bv, xx);
		} else {
			int const xx = strToInt(arg);
			scroll(bv, xx);
		}
		bv->updateInset(this, false);
		
		return DISPATCHED;
	}
	return UNDISPATCHED; 
}


int UpdatableInset::getMaxWidth(BufferView * bv, UpdatableInset const *) const
{
	int w;
	if (owner()){
		w = static_cast<UpdatableInset*>
			(owner())->getMaxWidth(bv, this);
	} else {
		w = bv->text->workWidth(bv, const_cast<UpdatableInset *>(this));
	}
	if (w < 0) {
		return -1;
	}
	// check for margins left/right and extra right margin "const 5"
	if ((w - ((2 * TEXT_TO_INSET_OFFSET) + 5)) >= 0)
		w -= (2 * TEXT_TO_INSET_OFFSET) + 5;
	if (topx_set && owner()) {
		w -= top_x;
		w += owner()->x();
	}
#if 0 // already handled above now!!!
	else if (!owner()) {
		// give some left margin this should be made better!
		// Idea: LyXText::giveLeftMargin(Inset * inset) will search the
		//       inset in the text and return the LeftMargin of that row!
		lyxerr << "w -= 20\n";
		w -= 20;
	}
#endif
	if (w < 10) {
		w = 10;
	}
	return w;
}


LyXCursor const & Inset::cursor(BufferView * bv) const
{
	return bv->text->cursor;
}


string const UpdatableInset::selectNextWordToSpellcheck(BufferView *bv,
					    float & value) const
{
	// we have to unlock ourself in this function by default!
	bv->unlockInset(const_cast<UpdatableInset *>(this));
	value = 0;
	return string();
}


bool UpdatableInset::searchForward(BufferView * bv, string const &,
                                   bool const &, bool const &)
{
	// we have to unlock ourself in this function by default!
	bv->unlockInset(const_cast<UpdatableInset *>(this));
	return false;
}


bool UpdatableInset::searchBackward(BufferView * bv, string const &,
                                    bool const &, bool const &)
{
	// we have to unlock ourself in this function by default!
	bv->unlockInset(const_cast<UpdatableInset *>(this));
	return false;
}
