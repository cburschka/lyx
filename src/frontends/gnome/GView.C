/**
 * \file GView.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GView.h"
#include "debug.h"
#include "support/filetools.h"
#include "ControlButtons.h"

#include <glib.h>
#include <gtkmm/dialog.h>

GView::GView(string const & name)
	: ViewBase(),
	  updating_(false),
	  file_(name + ".glade"),
	  title_(name),
	  widget_name_(name),
	  xml_(0),
	  dialog_(0)
{
	loadXML();
}

GView::~GView()
{}

gnomeBC & GView::bc()
{
	return static_cast<gnomeBC &>(getController().bc());
}

void GView::loadXML()
{
#ifdef WITH_WARNINGS
#warning Change this before declaring it production code! (be 20010325)
#endif
        string const path("src/frontends/gnome/dialogs/;frontends/gnome/dialogs/;");

	string const file = FileOpenSearch(path, file_, "glade");

	if (file.empty()) {
		lyxerr << "Cannot find glade file. Aborting." << endl;
		BOOST_ASSERT(true);
	}

	lyxerr[Debug::GUI] << "Glade file to open is " << file << endl;

	xml_ = Gnome::Glade::Xml::create (file, widget_name_);
}


void GView::show()
{
	update();
	dialog()->show();
}


void GView::hide()
{
	dialog()->hide();
}

bool GView::isValid()
{
	return true;
}

void GView::OKClicked()
{
	lyxerr[Debug::GUI] << "GView::OKClicked()" << endl;
	getController().OKButton();
}

void GView::CancelClicked()
{
	lyxerr[Debug::GUI] << "GView::CancelClicked()" << endl;
	getController().CancelButton();
}

void GView::ApplyClicked()
{
	lyxerr[Debug::GUI] << "GView::ApplyClicked()" << endl;
	getController().ApplyButton();
}

void GView::RestoreClicked()
{
	lyxerr[Debug::GUI] << "GView::RestoreClicked()" << endl;
	getController().RestoreButton();
}

void GView::InputChanged()
{
	bc().valid(isValid());
}

Gtk::Dialog * GView::dialog()
{
	if (!dialog_)
		dialog_ = getWidget<Gtk::Dialog>(widget_name_);

	return dialog_;
}

bool GView::isVisible() const
{
	return dialog_ && dialog_->is_visible();
}
