/**
 * \file GTableCreate.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GTableCreate.h"
#include "ghelpers.h"

#include "support/lstrings.h"

#include <libglademm.h>

using std::string;

namespace lyx {
namespace frontend {

GTableCreate::GTableCreate(Dialog & parent)
	: GViewCB<ControlTabularCreate, GViewGladeB>(parent, _("Insert Table"))
{
}


void GTableCreate::doBuild()
{
	string const gladeName = findGladeFile("tableCreate");
	xml_ = Gnome::Glade::Xml::create(gladeName);
	Gtk::Button * ok;
	Gtk::Button * apply;
	Gtk::Button * cancel;
	xml_->get_widget("Ok", ok);
	xml_->get_widget("Apply", apply);
	xml_->get_widget("Cancel", cancel);
	xml_->get_widget("Rows", rows_);
	xml_->get_widget("Columns", columns_);
	setOK(ok);
	setCancel(cancel);
	setApply(apply);
}


void GTableCreate::apply()
{
	int const rows = rows_->get_value_as_int();
	int const columns = columns_->get_value_as_int();
	controller().params() = std::make_pair(columns, rows);
}

} // namespace frontend
} // namespace lyx
