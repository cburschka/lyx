/**
 * \file GMathsMatrix.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GMathsMatrix.h"
#include "ControlMath.h"

#include "GViewBase.h"
#include "ghelpers.h"

#include <sstream>


using std::ostringstream;
using std::string;

namespace lyx {
namespace frontend {


GMathsMatrix::GMathsMatrix(Dialog & parent)
	: GViewCB<ControlMath, GViewGladeB>(parent, _("Math Matrix"), false)
{}


void GMathsMatrix::doBuild()
{
	string const gladeName = findGladeFile("mathMatrix");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * button;
	xml_->get_widget("Cancel",button);
	setCancel(button);
	xml_->get_widget("Insert",button);
	setOK(button);
	// No inserting matrices into readonly docs!
	bcview().addReadOnly(button);

	// Get widget pointers
	xml_->get_widget("Top", topradio_);
	xml_->get_widget("Bottom", bottomradio_);
	xml_->get_widget("Center", centerradio_);
	xml_->get_widget("Columns", columnsspin_);
	xml_->get_widget("Rows", rowsspin_);
	xml_->get_widget("HorzAlign", horzalignentry_);

	// Make center vertical alignment the default
	centerradio_->set_active(true);

	// Allow only [clr], keep length as number of cols
	ignoreHorzAlign_ = false;
	horzalignentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GMathsMatrix::updateHorzAlignEntry));
	columnsspin_->signal_value_changed().connect(
		sigc::mem_fun(*this, &GMathsMatrix::updateHorzAlignEntry));
}


void GMathsMatrix::apply()
{
	string const h_align = horzalignentry_->get_text();
	int const nx =
		static_cast<int>(columnsspin_->get_adjustment()->get_value());
	int const ny =
		static_cast<int>(rowsspin_->get_adjustment()->get_value());
	char v_align = 'c';
	if (topradio_->get_active())
		v_align = 't';
	else if (centerradio_->get_active())
		v_align = 'c';
	else if (bottomradio_->get_active())
		v_align = 'b';

	ostringstream os;
	os << nx << ' ' << ny << ' ' << v_align << ' ' << h_align;
	controller().dispatchMatrix(os.str());
}


void GMathsMatrix::update()
{
	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_VALID;
	bc().input(activate);
}


void GMathsMatrix::updateHorzAlignEntry()
{
	if (ignoreHorzAlign_) return;

	Glib::ustring orig = horzalignentry_->get_text();
  Glib::ustring stripped;

  Glib::ustring::iterator cur;
  for (cur = orig.begin(); cur != orig.end(); ++cur) {
	  if (*cur == 'c' || *cur == 'l' ||
	      *cur == 'r' || *cur == '|')
	    stripped += *cur;
	}

	int barcount = countbars(stripped);
	while (stripped.length() - barcount >
		     columnsspin_->get_adjustment()->get_value()) {
		// erase last character of stripped
		stripped = stripped.erase(stripped.length() - 1,1);
		barcount = 	countbars(stripped);
	}

	while (stripped.length() - barcount <
	       columnsspin_->get_adjustment()->get_value()) {
		stripped = stripped + "c";
		barcount = 	countbars(stripped);
	}

  if (orig.compare(stripped) != 0) {
  	ignoreHorzAlign_ = true;
		horzalignentry_->set_text(stripped);
		ignoreHorzAlign_ = false;
	}
}

int GMathsMatrix::countbars(Glib::ustring str)
{
	int barcount = 0;
  Glib::ustring::iterator cur = str.begin();
  Glib::ustring::iterator end = str.end();
	for (; cur != end; ++cur) {
		if (*cur == '|')
			++barcount;
	}
	return barcount;
}


} // namespace frontend
} // namespace lyx
