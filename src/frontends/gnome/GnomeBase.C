/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2000 The LyX Team.
 *
 * ================================================= */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "GnomeBase.h"
#include "support/LAssert.h"
#include "debug.h"
#include "support/filetools.h"
#include <glib.h>

#include <gnome--/dialog.h>

GnomeBase::GnomeBase(ControlButtons & c, 
		 string const & name)
	: ViewBC<gnomeBC>(c)
	, file_(name + ".glade"), widget_name_(name), xml_(0)
	, dialog_(0)
{}
	
GnomeBase::~GnomeBase()
{
	if (xml_)
		gtk_object_unref(GTK_OBJECT(xml_));
}

	
void GnomeBase::loadXML() const
{
#ifdef WITH_WARNINGS
#warning Change this before declaring it production code! (be 20010325)
#endif
	string const path("src/frontends/gnome/dialogs/;frontends/gnome/dialogs/;/home/baruch/prog/lyx/graphics/src/frontends/gnome/dialogs/");
	string const file = FileOpenSearch(path, file_, "glade");

	if (file.empty()) {
		lyxerr << "Cannot find glade file. Aborting." << std::endl;
		lyx::Assert(true);
	}
	
	lyxerr[Debug::GUI] << "Glade file to open is " << file << '\n';
	
	xml_ = glade_xml_new(file.c_str(), widget_name_.c_str());
}


void GnomeBase::show()
{
	if (!dialog_) {
		dialog_ = dialog();
		build();
	}

	update();
	dialog_->show();
}


void GnomeBase::hide()
{
	if (dialog_)
		dialog_->hide();
}

bool GnomeBase::validate()
{
	return true;
}

void GnomeBase::OKClicked() 
{ 
	lyxerr[Debug::GUI] << "OKClicked()\n";
	OKButton(); 
}

void GnomeBase::CancelClicked() 
{ 
	CancelButton(); 
}

void GnomeBase::ApplyClicked() 
{ 
	ApplyButton(); 
}

void GnomeBase::RestoreClicked() 
{ 
	RestoreButton(); 
}

void GnomeBase::InputChanged() 
{ 
	bc().valid(validate()); 
}

Gnome::Dialog * GnomeBase::dialog()
{
	if (!dialog_) 
		dialog_ = getWidget<Gnome::Dialog>(widget_name_.c_str());
		
	return dialog_;
}
