/**
 * \file GMathPanel.C
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

#include "ControlMath.h"
#include "GMathPanel.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include "deco.xpm"
#include "delim.xpm"
#include "equation.xpm"
#include "frac.xpm"
#include "matrix.xpm"
#include "space.xpm"
#include "style.xpm"
#include "sqrt.xpm"
#include "sub.xpm"
#include "super.xpm"

#include "gimages/ams_arrow.xpm"
#include "gimages/ams_misc.xpm"
#include "gimages/ams_brel.xpm"
#include "gimages/ams_nrel.xpm"
#include "gimages/ams_ops.xpm"
#include "gimages/arrow.xpm"
#include "gimages/boperator.xpm"
#include "gimages/brelats.xpm"
#include "gimages/dots.xpm"
#include "gimages/greek.xpm"
#include "gimages/misc.xpm"
#include "gimages/varsize.xpm"

using std::string;

namespace {

char const * infoUp[][5] =
{
	//row 1
	{0, 0, "mathdelimiter", "mathaccents", "mathspace"},
	//row 2
	{0, 0, "mathstyle", "mathmatrix", 0}
};

GXpmBtnTbl::XpmData xpmUp[] =
{
	//row 1
	sqrt_xpm, frac, delim, deco, space_xpm,
	//row 2
	super_xpm, sub_xpm, style_xpm, matrix, equation
};

char const * infoDown[][3] =
{
	//row 1
	{"mathoperators", "mathrelations", "matharrows"},
	//row 2
	{"mathbigoperators", "mathdots", "mathmisc"},
	//row 3
	{"mathgreek", "mathamsarrows", "mathamsrelations"},
	//row 4
	{"mathamsnegatedrelations", "mathamsoperators", "mathamsmisc"}
};

GXpmBtnTbl::XpmData xpmDown[] =
{
	//row 1
	boperator_xpm, brelats_xpm, arrow_xpm,
        //row 2
	varsize_xpm, dots_xpm, misc_xpm,
	//row 3
	greek_xpm, ams_arrow_xpm, ams_brel_xpm,
	//row 4
	ams_nrel_xpm, ams_ops_xpm, ams_misc_xpm
};

}


GMathPanel::GMathPanel(Dialog & parent)
	: GViewCB<ControlMath, GViewGladeB>(parent, _("Math Panel")),
	  tableUp_(2, 5, xpmUp), tableDown_(4, 3, xpmDown)
{
}


void GMathPanel::doBuild()
{
	string const gladeName =
		lyx::support::LibFileSearch("glade", "mathPanel", "glade");
	xml_ = Gnome::Glade::Xml::create(gladeName);
	Gtk::Button * close;
	Gtk::VBox * vbox;

	xml_->get_widget("Close", close);
	setCancel(close);

	tableUp_.signalClicked().connect(
		SigC::slot(*this, &GMathPanel::onTableUpClicked));
	tableUp_.show();
	tableDown_.signalClicked().connect(
		SigC::slot(*this, &GMathPanel::onTableDownClicked));
	tableDown_.show();

	xml_->get_widget("Vbox", vbox);
	vbox->children().push_back(Gtk::Box_Helpers::Element(tableUp_));
	vbox->children().push_back(Gtk::Box_Helpers::Element(tableDown_));

	// Functions ListView
	xml_->get_widget("Functions", functions_);
	listCols_.add(listCol_);
	listStore_ = Gtk::ListStore::create(listCols_);
	functions_->set_model(listStore_);
	functions_->append_column("Functions", listCol_);
	listSel_ = functions_->get_selection();
	listSel_->signal_changed().connect(
		SigC::slot(*this, &GMathPanel::onFunctionSelected));
	for (int i = 0; i < nr_function_names; ++i)
		(*listStore_->append())[listCol_] =
			Glib::locale_to_utf8(function_names[i]);
}


void GMathPanel::onTableUpClicked(int row, int col)
{
	if (infoUp[row][col])
		controller().showDialog(infoUp[row][col]);
	else if (row == 0 && col == 0)
		controller().dispatchInsert("sqrt");
	else if (row == 0 && col == 1)
		controller().dispatchInsert("frac");
	else if (row == 1 && col == 0)
	controller().dispatchSuperscript();
	else if (row == 1 && col == 1)
	controller().dispatchSubscript();
	else if (row == 1 && col == 4)
		controller().dispatchToggleDisplay();
}


void GMathPanel::onTableDownClicked(int row, int col)
{
	controller().showDialog(infoDown[row][col]);
}


void GMathPanel::onFunctionSelected()
{
	Gtk::TreeModel::iterator it = listSel_->get_selected();
	Glib::ustring sel = (*it)[listCol_];
	controller().dispatchInsert(
		Glib::locale_from_utf8(sel));
}
