/**
 * \file GMathDelim.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <gtkmm.h>
#include <libglademm.h>
#include <sstream>

#include "ControlMath.h"
#include "GMathDelim.h"
#include "ghelpers.h"

#include "support/lstrings.h"

#include "delim.xbm"
#include "delim0.xpm"

using std::string;

namespace lyx {
namespace frontend {

namespace
{

enum enumDelimType {LEFT, RIGHT, SINGLE};


int const delimType[] = {
	//row 1
	LEFT, RIGHT, LEFT, RIGHT, SINGLE, SINGLE, LEFT, RIGHT,LEFT, RIGHT,
	SINGLE, SINGLE,
	//row 2
	LEFT, RIGHT, LEFT, RIGHT, SINGLE, SINGLE, LEFT, RIGHT, SINGLE, SINGLE,
	SINGLE
};


int const delimRevert[] = {
	1,0,3,2,4,5,7,6,9,8,10,11,
	13,12,15,14,16,17,19,18,20,21,22
};


char const * delimValues[] = {
	"(", ")", "lceil",  "rceil",  "uparrow",  "Uparrow",
	"[", "]", "lfloor", "rfloor", "updownarrow", "Updownarrow",
	"{", "}",  "/", "backslash",  "downarrow",  "Downarrow",
	"langle",  "rangle", "|", "Vert", ".", 0
};

int const delimTblRows = 2;

int const delimTblCols = 12;

int const delimMax = 23;


GXpmBtnTbl::XbmData xbm =
{
	delim_bits,
	delim_width,
	delim_height,
	{0, 0, 0, 65535}
};


inline int index(int row, int col)
{
	return row * delimTblCols + col;
}


inline int indexToRow(int index)
{
	return index / delimTblCols;
}


inline int indexToCol(int index)
{
	return index % delimTblCols;
}

}


GMathDelim::GMathDelim(Dialog & parent) :
	GViewCB<ControlMath, GViewGladeB>(parent, _("Math Delimiters")),
	delimTbl_(delimTblRows, delimTblCols, xbm)
{
}


void GMathDelim::doBuild()
{
	string const gladeName = findGladeFile("mathDelim");
	xml_ = Gnome::Glade::Xml::create(gladeName);
	Gtk::Button * ok;
	Gtk::Button * apply;
	Gtk::Button * close;
	Gtk::Box * box;
	xml_->get_widget("Left", left_);
	xml_->get_widget("Right", right_);
	xml_->get_widget("Both", both_);
	xml_->get_widget("OK", ok);
	xml_->get_widget("Apply", apply);
	xml_->get_widget("Close", close);
	xml_->get_widget("Demo", demo_);
	setOK(ok);
	setApply(apply);
	setCancel(close);
	// Initialize demo button pixmap to "()" as found in images/delim0.xpm
	setDemoPixmap();
	leftSel_ = 0;
	rightSel_ = 1;
	xml_->get_widget("Box", box);
	delimTbl_.signalClicked().connect(
		SigC::slot(*this, &GMathDelim::onDelimTblClicked));
	delimTbl_.show();
	box->children().push_back(
		Gtk::Box_Helpers::Element(delimTbl_));
	bcview().addReadOnly(&delimTbl_);
	bcview().addReadOnly(left_);
	bcview().addReadOnly(right_);
	bcview().addReadOnly(both_);
	bcview().addReadOnly(demo_);
	left_->signal_clicked().connect(
		SigC::slot(*this, &GMathDelim::onRadioClicked));
	right_->signal_clicked().connect(
		SigC::slot(*this, &GMathDelim::onRadioClicked));
	both_->signal_clicked().connect(
		SigC::slot(*this, &GMathDelim::onRadioClicked));
}


void GMathDelim::setDemoPixmap()
{
	Gtk::Image * image;
	pixmap_ = Gdk::Pixmap::create_from_xpm(demo_->get_colormap(),
					       mask_,
					       delim0);
	image = SigC::manage(new Gtk::Image(pixmap_, mask_));
	image->show();
	demo_->add(*image);
	gcMask_ = Gdk::GC::create(mask_);
}


void GMathDelim::apply()
{
	std::ostringstream os;
	os << delimValues[leftSel_] << ' ' << delimValues[rightSel_];
	controller().dispatchDelim(os.str());
}


void GMathDelim::update()
{
	bc().valid();
}


void GMathDelim::onDelimTblClicked(int row, int col)
{
	int const sel = index(row, col);
	if (sel >= delimMax)
		return;
	bool left = left_->get_active();
	bool right = right_->get_active();
	bool both = both_->get_active();
	if (left)
		leftSel_ = sel;
	else if (right)
		rightSel_ = sel;
	else if (both)
		if (delimType[sel] == LEFT) {
			leftSel_ = sel;
			rightSel_ = delimRevert[sel];
		} else if (delimType[sel] == RIGHT) {
			rightSel_ = sel;
			leftSel_ = delimRevert[sel];
		} else {
			leftSel_ = rightSel_ = sel;
		}
	updateDemoPixmap();
}


void GMathDelim::updateDemoPixmap()
{
	int const delimWidth = delim_width / delimTblCols;
	Glib::RefPtr<Gdk::Pixmap> pixmap;
	Glib::RefPtr<Gdk::Bitmap> mask;
	GXpmBtnTbl::GXpmBtn * btn =  delimTbl_.getBtn(indexToRow(leftSel_),
						       indexToCol(leftSel_));
	pixmap = btn->getPixmap();
	mask = btn->getMask();
	pixmap_->draw_drawable(left_->get_style()->get_black_gc(),
			       pixmap,
			       0, 0,
			       0, 0);
	mask_->draw_drawable(gcMask_,
			     mask,
			     0, 0,
			     0, 0);
	btn =  delimTbl_.getBtn(indexToRow(rightSel_),
				indexToCol(rightSel_));
	pixmap = btn->getPixmap();
	mask = btn->getMask();
	pixmap_->draw_drawable(left_->get_style()->get_black_gc(),
			       pixmap,
			       0, 0,
			       delimWidth, 0);
	mask_->draw_drawable(gcMask_,
			     mask,
			     0, 0,
			     delimWidth, 0);
	int x, y, width, height, depth;
	demo_->get_window()->get_geometry(x, y, width, height, depth);
	demo_->get_window()->invalidate_rect(
		Gdk::Rectangle(x, y, width, height), true);
	bc().valid();
}


void GMathDelim::onRadioClicked()
{
	bc().valid();
}

} // namespace frontend
} // namespace lyx
