// -*- C++ -*-
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
		string const & glade_file, string const & name)
	: ViewBC<gnomeBC>(c)
	, file_(glade_file), widget_name_(name), xml_(0)
	, dialog_(0)
{}
	
GnomeBase::~GnomeBase()
{
	if (xml_)
		gtk_object_unref(GTK_OBJECT(xml_));
}

	
void GnomeBase::loadXML() const
{
#warning Change this before declaring it production code! (be 20010325)
	string const path("src/frontends/gnome/dialogs/;frontends/gnome/dialogs/;/home/baruch/prog/lyx/graphics/src/frontends/gnome/dialogs/");
	string const file = FileOpenSearch(path, file_, "glade");

	if (file.empty()) {
		lyxerr << "Cannot find glade file. Aborting." << std::endl;
		Assert(true);
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


Gnome::Dialog * GnomeBase::dialog()
{
	if (!dialog_) 
		dialog_ = getWidget<Gnome::Dialog>(widget_name_.c_str());
		
	return dialog_;
}
