// -*- C++ -*-
/*
 * /file GPrint.h
 */

#ifndef GPRINT_H
#define GPRINT_H

#include "LString.h"
#include "GViewBase.h"

class ControlPrint;

class GPrint : public GViewCB<ControlPrint, GViewGladeB>
{
public:
	GPrint(Dialog & parent, string title = "Print Document");
private:
	virtual void apply();
	virtual void update();
	virtual void doBuild();
	void updateUI();
	void onBrowse();
	void onTargetEdit(Gtk::Entry const * who);
	void onFromToEdit();

	Gtk::RadioButton * printer_;
	Gtk::RadioButton * file_;
	Gtk::RadioButton * all_;
	Gtk::RadioButton * fromTo_;
	Gtk::CheckButton * odd_;
	Gtk::CheckButton * even_;
	Gtk::CheckButton * reverse_;
	Gtk::SpinButton * number_;
	Gtk::CheckButton * sorted_;
	Gtk::Entry * printerEntry_;
	Gtk::Entry * fileEntry_;
	Gtk::Entry * fromEntry_;
	Gtk::Entry * toEntry_;
};

#endif

