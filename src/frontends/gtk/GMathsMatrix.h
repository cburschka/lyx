// -*- C++ -*-
/**
 * \file GMathsMatrix.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GMATHSMATRIX_H
#define GMATHSMATRIX_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlMath;

/**
 * This class provides an GTK implementation of the maths matrix dialog.
 */
class GMathsMatrix
	: public GViewCB<ControlMath, GViewGladeB> {
public:
	GMathsMatrix(Dialog &);
	int AlignFilter(char const *, int);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	void updateHorzAlignEntry();
	int countbars(Glib::ustring str);

	Gtk::RadioButton * topradio_;
	Gtk::RadioButton * bottomradio_;
	Gtk::RadioButton * centerradio_;
	Gtk::SpinButton * rowsspin_;
	Gtk::SpinButton * columnsspin_;
	Gtk::Entry * horzalignentry_;
	bool ignoreHorzAlign_;
};

} // namespace frontend
} // namespace lyx

#endif //  GMATHSMATRIX_H

