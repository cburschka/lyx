/**
 * \file GnomeBase.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 *
 * Full author contact details are available in file CREDITS
 */


#include <config.h>
#include "GnomeBase.h"
#include "support/LAssert.h"
#include "debug.h"
#include "support/filetools.h"
#include "ControlButtons.h"
#include <glib.h>
#include <gtkmm/dialog.h>

GnomeBase::GnomeBase(string const & name)
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

GnomeBase::~GnomeBase()
{}

gnomeBC & GnomeBase::bc()
{
	return static_cast<gnomeBC &>(getController().bc());
}

void GnomeBase::loadXML()
{
#ifdef WITH_WARNINGS
#warning Change this before declaring it production code! (be 20010325)
#endif
        string const path("src/frontends/gnome/dialogs/;frontends/gnome/dialogs/;");

	string const file = FileOpenSearch(path, file_, "glade");

	if (file.empty()) {
		lyxerr << "Cannot find glade file. Aborting." << std::endl;
		lyx::Assert(true);
	}

	lyxerr[Debug::GUI] << "Glade file to open is " << file << '\n';

	xml_ = Gnome::Glade::Xml::create (file, widget_name_);
}


void GnomeBase::show()
{
	update();
	dialog()->show();
}


void GnomeBase::hide()
{
	dialog()->hide();
}

bool GnomeBase::isValid()
{
	return true;
}

void GnomeBase::OKClicked()
{
	lyxerr[Debug::GUI] << "GnomeBase::OKClicked()\n";
	getController().OKButton();
}

void GnomeBase::CancelClicked()
{
	lyxerr[Debug::GUI] << "GnomeBase::CancelClicked()\n";
	getController().CancelButton();
}

void GnomeBase::ApplyClicked()
{
	lyxerr[Debug::GUI] << "GnomeBase::ApplyClicked()\n";
	getController().ApplyButton();
}

void GnomeBase::RestoreClicked()
{
	lyxerr[Debug::GUI] << "GnomeBase::RestoreClicked()\n";
	getController().RestoreButton();
}

void GnomeBase::InputChanged()
{
	bc().valid(isValid());
}

Gtk::Dialog * GnomeBase::dialog()
{
	if (!dialog_)
		dialog_ = getWidget<Gtk::Dialog>(widget_name_);

	return dialog_;
}

bool GnomeBase::isVisible() const
{
	return dialog_ && dialog_->is_visible();
}
