// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995-2000 The LyX Team.
 *
 * ================================================= 
 *
 * \author Baruch Even
 **/

#ifndef GnomeBase_H
#define GnomeBase_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ViewBase.h"
#include "gnomeBC.h"
#include <sigc++/signal_system.h>
#include "gnome_helpers.h"

namespace Gnome {
class Dialog;
};

/**
 * This is a base class for Gnome dialogs. It handles all the common
 * work that is needed for all dialogs.
 */
class GnomeBase : public ViewBC<gnomeBC>, public SigC::Object {
public:
	/// 
	GnomeBase(ControlButtons & c, string const & glade_file, string const & name);
	///
	virtual ~GnomeBase();

protected:
	/// Get the widget named 'name' from the xml representation.
	template <class T>
	T* getWidget(char const * name) const; 

	/// Get the dialog we use.
	Gnome::Dialog * dialog();

	/// Show the dialog.
	void show();
	/// Hide the dialog.
	void hide();
	
	/// Build the dialog. Also connects signals and prepares it for work.
	virtual void build() = 0;

private:
	/// Loads the glade file to memory.
	void loadXML() const;

	/// The glade file name
	const string file_;
	/// The widget name
	const string widget_name_;
	/// The XML representation of the dialogs.
	mutable GladeXML * xml_;

	/** The dialog we work with, since it is managed by libglade, we do not
	 *  need to delete it or destroy it, it will be destroyed with the rest
	 *  of the libglade GladeXML structure.
	 */
	Gnome::Dialog * dialog_;
};


template <class T>
T* GnomeBase::getWidget(char const * name) const
{
	if (xml_ == 0)
		loadXML();
	return getWidgetPtr<T>(xml_, name);
}

/**
 * This class is used to provide a simple automatic casting of the controller.
 * We chose not to make GnomeBase a template since it has some size and we 
 * have no reason to duplicate it by making it a template.
 *
 * Basically the FormCB<Controller> template instantiates GnomeBase and passes
 * the parameters to it and it also adds the controller() method to give us
 * a reference to the controller of the correct type (the type is set by the
 * template parameter).
*/
template <class Controller>
class FormCB : public GnomeBase {
public:
	FormCB(Controller & c, string const & file, string const & name);
protected:
	Controller & controller();
};

template <class Controller>
FormCB<Controller>::FormCB(Controller & c, string const & file, string const & name)
	: GnomeBase(c, file, name)
{}

template <class Controller>
Controller &
FormCB<Controller>::controller()
{
	return static_cast<Controller &>(controller_);
}

#endif
