/**
 * \file GFloat.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Michael Koziarski <michael@koziarski.org>
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "gnomeBC.h"
#include "GFloat.h"
#include "support/lstrings.h"

#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/box.h>

GFloat::GFloat(ControlFloat & c)
	: FormCB<ControlFloat>(c, "GFloat")
{}


GFloat::~GFloat()
{}


void GFloat::build()
{
	// Connect the buttons.
	close_btn()->signal_clicked().connect(SigC::slot(*this, &GFloat::OKClicked));
	// Manage the buttons state
	bc().setCancel(close_btn());
	bc().refresh();
	connect_signals();
}

void GFloat::apply()
{
	string placement;
	if (here_definitely()->get_active()) {
		placement += "H";
	} else {
		if (top_of_page()->get_active()) {
			placement += "t";
		}
		if (bottom_of_page()->get_active()) {
			placement += "b";
		}
		if (page_of_floats()->get_active()) {
			placement += "p";
		}
		if (here_if_possible()->get_active()) {
			placement += "h";
		}
	}
	controller().params().placement = placement;
}


void GFloat::update()
{
	disconnect_signals();
	bool top = false;
	bool bottom = false;
	bool page = false;
	bool here = false;
	bool forcehere = false;

	string placement(controller().params().placement);

	if (contains(placement, "H")) {
		forcehere = true;
	} else {
		if (contains(placement, "t")) {
			top = true;
		}
		if (contains(placement, "b")) {
			bottom = true;
		}
		if (contains(placement, "p")) {
			page = true;
		}
		if (contains(placement, "h")) {
			here = true;
		}
	}
	
	top_of_page()->set_active(top);
	page_of_floats()->set_active(page);
	bottom_of_page()->set_active(bottom);
	here_if_possible()->set_active(here);
	here_definitely()->set_active(forcehere);
	connect_signals();
}

void GFloat::connect_signals()
{
	conn_top_        = top_of_page()->signal_toggled().connect(
		SigC::slot(*this, &GFloat::ApplyClicked)
		);
	conn_bottom_     = bottom_of_page()->signal_toggled().connect(
		SigC::slot(*this, &GFloat::ApplyClicked)
		);
	conn_page_       = page_of_floats()->signal_toggled().connect(
		SigC::slot(*this, &GFloat::ApplyClicked)
		);
	conn_ifposs_     = here_if_possible()->signal_toggled().connect(
		SigC::slot(*this, &GFloat::ApplyClicked)
		);
	conn_definitely_ = here_definitely()->signal_toggled().connect(
		SigC::slot(*this, &GFloat::ApplyClicked)
		);
	conn_disable_    = here_definitely()->signal_toggled().connect(
		SigC::slot(*this, &GFloat::update_sensitive)
		);
}

void GFloat::disconnect_signals() 
{
	conn_top_.disconnect();
	conn_bottom_.disconnect();
	conn_page_.disconnect();
	conn_ifposs_.disconnect();
	conn_definitely_.disconnect();
	conn_disable_.disconnect();
}

void GFloat::update_sensitive() 
{
	if (here_definitely()->get_active()) 
		other_options()->set_sensitive(false);
	else 
		other_options()->set_sensitive(true);

}

Gtk::HBox * GFloat::other_options() const 
{
        return getWidget<Gtk::HBox>("r_other_options");
}
Gtk::CheckButton * GFloat::page_of_floats() const 
{
        return getWidget<Gtk::CheckButton>("r_page_of_floats");
}
Gtk::CheckButton * GFloat::top_of_page() const 
{
        return getWidget<Gtk::CheckButton>("r_top_of_page");
}
Gtk::CheckButton * GFloat::bottom_of_page() const 
{
        return getWidget<Gtk::CheckButton>("r_bottom_of_page");
}
Gtk::CheckButton * GFloat::here_if_possible() const 
{
        return getWidget<Gtk::CheckButton>("r_here_if_possible");
}
Gtk::RadioButton * GFloat::here_definitely() const 
{
        return getWidget<Gtk::RadioButton>("r_here_definitely");
}

Gtk::Button * GFloat::close_btn() const 
{
        return getWidget<Gtk::Button>("r_close_btn");
}
