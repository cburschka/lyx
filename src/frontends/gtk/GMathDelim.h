// -*- C++ -*-
/**
 * \file GMathDelim.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GMATH_DELIM_H
#define GMATH_DELIM_H

#include "GViewBase.h"
#include "support/lstrings.h"
#include "GXpmBtnTbl.h"

namespace lyx {
namespace frontend {

class ControlMath;

class GMathDelim : public GViewCB<ControlMath, GViewGladeB>
{
public:
	GMathDelim(Dialog & parent);
private:
	virtual void apply();
	virtual void update();
	virtual void doBuild();
	void setDemoPixmap();
	void updateDemoPixmap();
	void onDelimTblClicked(int row, int col);
	void onRadioClicked();
	GXpmBtnTbl delimTbl_;
	Gtk::Button * demo_;
	Gtk::RadioButton * left_;
	Gtk::RadioButton * right_;
	Gtk::RadioButton * both_;
	Glib::RefPtr<Gdk::Pixmap> pixmap_;
	Glib::RefPtr<Gdk::Bitmap> mask_;
	Glib::RefPtr<Gdk::GC> gcMask_;
	int leftSel_;
	int rightSel_;
};

} // namespace frontend
} // namespace lyx

#endif
