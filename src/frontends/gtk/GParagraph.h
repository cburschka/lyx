// -*- C++ -*-
/**
 * \file GParagraph.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \auther John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GPARAGRAPH_H
#define GPARAGRAPH_H

#include "GViewBase.h"

#include <gtkmm.h>

namespace lyx {
namespace frontend {

class ControlParagraph;

/** This class provides a gtk implementation of the paragraph dialog.
 */
class GParagraph
	: public GViewCB<ControlParagraph, GViewGladeB> {
public:
	GParagraph(Dialog &);
private:
	/// Build the dialog
	virtual void doBuild();
	/// Apply from dialog
	virtual void apply() {}
	/// Update the dialog
	virtual void update();

	Gtk::SpinButton * spacingspin_;
	Gtk::Entry * maxlabelwidthentry_;
	Gtk::CheckButton * indentcheck_;
	Gtk::CheckButton * defaultspacingcheck_;
	Gtk::RadioButton * blockradio_;
	Gtk::RadioButton * leftradio_;
	Gtk::RadioButton * rightradio_;
	Gtk::RadioButton * centerradio_;

	Gtk::Adjustment * spacingadj_;

	void onDefaultSpacingToggled();
	void onMaxLabelWidthChanged();
	void onSpacingChanged();
	void onIndentToggled();
	void onAlignToggled();
};

} // namespace frontend
} // namespace lyx

#endif
