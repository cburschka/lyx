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
#include <gtkmm.h>

#include <libglademm.h>

#include "ControlTabularCreate.h"
#include "GTableCreate.h"
#include "support/filetools.h"
#include "support/lstrings.h"

using std::string;

GTableCreate::GTableCreate(Dialog & parent)
	: GViewCB<ControlTabularCreate, GViewGladeB>(parent, _("Insert Table"))
{
}

void GTableCreate::doBuild()
{
	string const gladeName =
		lyx::support::LibFileSearch("glade", "tableCreate", "glade");
	xml_ = Gnome::Glade::Xml::create(gladeName);
	Gtk::Button * ok;
	Gtk::Button * apply;
	Gtk::Button * cancel;
	xml_->get_widget("Ok", ok);
	xml_->get_widget("Apply", apply);
	xml_->get_widget("Cancel", cancel);
	xml_->get_widget("Rows", rows_);
	xml_->get_widget("Columns", columns_);
	bcview().setOK(ok);
	bcview().setCancel(cancel);
	bcview().setApply(apply);
	ok->signal_clicked().connect(
		SigC::slot(*this, &GViewBase::onOK));
	apply->signal_clicked().connect(
		SigC::slot(*this, &GViewBase::onApply));
	cancel->signal_clicked().connect(
		SigC::slot(*this, &GViewBase::onCancel));
}

void GTableCreate::apply()
{
	int rows = rows_->get_value_as_int();
	int columns = columns_->get_value_as_int();
	controller().params() = std::make_pair(columns, rows);
}
