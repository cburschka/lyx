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

using std::string;

GMathPanel::GMathPanel(Dialog & parent)
	: GViewCB<ControlMath, GViewGladeB>(parent, _("Math Panel"))
{
}


void GMathPanel::doBuild()
{
	string const gladeName =
		lyx::support::LibFileSearch("glade", "mathPanel", "glade");
	xml_ = Gnome::Glade::Xml::create(gladeName);
	Gtk::Button * close;

	//1
	Gtk::Button * sqrt;
	Gtk::Button * frac;
	Gtk::Button * delim;
	Gtk::Button * deco;
	Gtk::Button * space;

	//2
	Gtk::Button * super;
	Gtk::Button * sub;
	Gtk::Button * style;
	Gtk::Button * matrix;
	Gtk::Button * equation;

	//3
	Gtk::Button * boperator;
	Gtk::Button * brelats;
	Gtk::Button * arrow;

	//4
	Gtk::Button * varSize;
	Gtk::Button * dots;
	Gtk::Button * misc;

	//5
	Gtk::Button * greek;
	Gtk::Button * amsArrows;
	Gtk::Button * amsBrel;

	//6
	Gtk::Button * amsNrel;
	Gtk::Button * amsOps;
	Gtk::Button * amsMisc;

	xml_->get_widget("Close", close);
	close->signal_clicked().connect(
		SigC::slot(*this, &GViewBase::onCancel));

	//1
	xml_->get_widget("Sqrt", sqrt);
	xml_->get_widget("Frac", frac);
	xml_->get_widget("Delim", delim);
	xml_->get_widget("Deco", deco);
	xml_->get_widget("Space", space);

	//2
	xml_->get_widget("Super", super);
	xml_->get_widget("Sub", sub);
	xml_->get_widget("Style", style);
	xml_->get_widget("Matrix", matrix);
	xml_->get_widget("Equation", equation);

	//3
	xml_->get_widget("Boperator", boperator);
	xml_->get_widget("Brelate", brelats);
	xml_->get_widget("Arrow", arrow);

	//4
	xml_->get_widget("VarSize", varSize);
	xml_->get_widget("Dots", dots);
	xml_->get_widget("Misc", misc);

	//5
	xml_->get_widget("Greek", greek);
	xml_->get_widget("AmsArrow", amsArrows);
	xml_->get_widget("AmsBrel", amsBrel);

	//6
	xml_->get_widget("AmsNrel", amsNrel);
	xml_->get_widget("AmsOps", amsOps);
	xml_->get_widget("AmsMisc", amsMisc);

	Glib::RefPtr<Gdk::Bitmap> mask;
	Glib::RefPtr<Gdk::Pixmap> pixmap;
	Glib::RefPtr<Gdk::Colormap> clrmap = window()->get_colormap();

	pixmap = Gdk::Pixmap::create_from_xpm(clrmap, mask, ::sqrt_xpm);
	Gtk::Image * iSqrt = SigC::manage(new Gtk::Image(pixmap, mask));
	iSqrt->show();
	sqrt->add(*iSqrt);

	pixmap = Gdk::Pixmap::create_from_xpm(clrmap, mask, ::frac);
	Gtk::Image * iFrac = SigC::manage(new Gtk::Image(pixmap, mask));
	iFrac->show();
	frac->add(*iFrac);

	pixmap = Gdk::Pixmap::create_from_xpm(clrmap, mask, ::delim);
	Gtk::Image * iDelim = SigC::manage(new Gtk::Image(pixmap, mask));
	iDelim->show();
	delim->add(*iDelim);

	pixmap = Gdk::Pixmap::create_from_xpm(clrmap, mask, ::deco);
	Gtk::Image * iDeco = SigC::manage(new Gtk::Image(pixmap, mask));
	iDeco->show();
	deco->add(*iDeco);

	pixmap = Gdk::Pixmap::create_from_xpm(clrmap, mask, ::space_xpm);
	Gtk::Image * iSpace = SigC::manage(new Gtk::Image(pixmap, mask));
	iSpace->show();
	space->add(*iSpace);

	pixmap = Gdk::Pixmap::create_from_xpm(clrmap, mask, ::super_xpm);
	Gtk::Image * iSuper = SigC::manage(new Gtk::Image(pixmap, mask));
	iSuper->show();
	super->add(*iSuper);

	pixmap = Gdk::Pixmap::create_from_xpm(clrmap, mask, ::sub_xpm);
	Gtk::Image * iSub = SigC::manage(new Gtk::Image(pixmap, mask));
	iSub->show();
	sub->add(*iSub);

	pixmap = Gdk::Pixmap::create_from_xpm(clrmap, mask, ::style_xpm);
	Gtk::Image * iStyle = SigC::manage(new Gtk::Image(pixmap, mask));
	iStyle->show();
	style->add(*iStyle);

	pixmap = Gdk::Pixmap::create_from_xpm(clrmap, mask, ::matrix);
	Gtk::Image * iMatrix = SigC::manage(new Gtk::Image(pixmap, mask));
	iMatrix->show();
	matrix->add(*iMatrix);

	pixmap = Gdk::Pixmap::create_from_xpm(clrmap, mask, ::equation);
	Gtk::Image * iEquation = SigC::manage(new Gtk::Image(pixmap, mask));
	iEquation->show();
	equation->add(*iEquation);

	arrow->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"matharrows"));
	deco->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathaccents"));
	boperator->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathoperators"));
	brelats->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathrelations"));
	greek->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathgreek"));
	misc->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathmisc"));
	dots->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathdots"));
	varSize->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathbigoperators"));
	amsMisc->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathamsmisc"));
	amsArrows->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathamsarrows"));
	amsBrel->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathamsrelations"));
	amsNrel->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathamsnegatedrelations"));
	amsOps->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathamsoperators"));
	delim->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathdelimiter"));
	matrix->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathmatrix"));
	space->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathspace"));
	style->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onShowDialog),
			"mathstyle"));

	super->signal_clicked().connect(
		SigC::slot(*this, &GMathPanel::onSuperClicked));
	sub->signal_clicked().connect(
		SigC::slot(*this, &GMathPanel::onSubClicked));
	equation->signal_clicked().connect(
		SigC::slot(*this, &GMathPanel::onEquationClicked));
	
	frac->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onInsert),
			"frac"));
	sqrt->signal_clicked().connect(
		SigC::bind<char const *>(
			SigC::slot(*this,
				   &GMathPanel::onInsert),
			"sqrt"));

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


void GMathPanel::onFunctionSelected()
{
	Gtk::TreeModel::iterator it = listSel_->get_selected();
	Glib::ustring sel = (*it)[listCol_];
	controller().dispatchInsert(
		Glib::locale_from_utf8(sel));
}


void GMathPanel::onShowDialog(char const * dialogName)
{
	controller().showDialog(dialogName);
}


void GMathPanel::onSuperClicked()
{
	controller().dispatchSuperscript();
}


void GMathPanel::onSubClicked()
{
	controller().dispatchSubscript();
}


void GMathPanel::onEquationClicked()
{
	controller().dispatchToggleDisplay();
}


void GMathPanel::onInsert(char const * what)
{
	controller().dispatchInsert(what);
}
