// -*- C++ -*-
/**
 * \file GnomeBase.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GnomeBase_H
#define GnomeBase_H


#include "gnome_helpers.h"

#include <sigc++/sigc++.h>
#include <libglademm/xml.h>

#include "ViewBase.h"
#include "gnomeBC.h"

namespace Gtk {
	class Dialog;
};

/**
 * This is a base class for Gnome dialogs. It handles all the common
 * work that is needed for all dialogs.
 */
class GnomeBase : public ViewBase, public SigC::Object {
public:
	///
        GnomeBase(string const & name);
	///
	virtual ~GnomeBase();

protected:
	/// Get the widget named 'name' from the xml representation.
	template <class T>
	T* getWidget(const string & name) const;

	/// Get the dialog we use.
	Gtk::Dialog * dialog();

	/// Show the dialog.
	void show();
	/// Hide the dialog.
	void hide();
	/// Build the dialog. Also connects signals and prepares it for work.
	virtual void build() = 0;
	/// Dialog is valid
	virtual bool isValid();
	/// dialog is visible
	virtual bool isVisible() const;
	/// Default OK behaviour
	virtual void OKClicked();
	/// Default Cancel behaviour
	virtual void CancelClicked();
	/// Default Restore behaviour
	virtual void RestoreClicked();
	/// Default apply behaviour
	virtual void ApplyClicked();
	/// Default changed input behaviour
	virtual void InputChanged();

	///
	gnomeBC & bc();

	/// are we updating ?
	bool updating_;
private:
	/// Loads the glade file to memory.
	void loadXML();

	/// The glade file name
	const string file_;
	/// The widget name
	const string widget_name_;
	/// The XML representation of the dialogs.
	Glib::RefPtr<Gnome::Glade::Xml>  xml_;

	/** The dialog we work with, since it is managed by libglade, we do not
	 *  need to delete it or destroy it, it will be destroyed with the rest
	 *  of the libglade GladeXML structure.
	 */
	Gtk::Dialog * dialog_;

	/// dialog title, displayed by WM.
	string title_;
};


template <class T>
T* GnomeBase::getWidget(const string & name) const
{
	return dynamic_cast<T*>(xml_->get_widget(name));
}

/**
 * This class is used to provide a simple automatic casting of the controller.
 * We chose not to make GnomeBase a template since it has some size and we
 * have no reason to duplicate it by making it a template.
 *
 * Basically the GnomeCB<Controller> template instantiates GnomeBase and passes
 * the parameters to it and it also adds the controller() method to give us
 * a reference to the controller of the correct type (the type is set by the
 * template parameter).
*/
template <class Controller>
class GnomeCB : public GnomeBase {
public:
  	GnomeCB(string const & name);
protected:
	Controller & controller();
};

template <class Controller>
GnomeCB<Controller>::GnomeCB(string const & name)
	: GnomeBase(name)
{}

template <class Controller>
Controller &
GnomeCB<Controller>::controller()
{
	return static_cast<Controller &>(getController());
}

#endif
