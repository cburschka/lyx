// -*- C++ -*-
/**
 * \file GViewBase.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GVIEWBASE_H
#define GVIEWBASE_H

#include <gtkmm.h>
#include <libglademm.h>
#include <boost/scoped_ptr.hpp>
#include "Dialog.h"
#include "ButtonPolicies.h"
#include "GBC.h"

namespace lyx {
namespace frontend {

class GViewBase : public Dialog::View, public sigc::trackable {
public:
	GViewBase(Dialog &, std::string const &, bool allowResize);
	virtual ~GViewBase();
	void setCancel(Gtk::Button * cancel);
	void setApply(Gtk::Button * apply);
	void setOK(Gtk::Button * ok);
	void setRestore(Gtk::Button * restore);
protected:
	// Build the dialog
	virtual void build();
	virtual void doBuild() = 0;
	// Hide the dialog
	virtual void hide();
	// Create the dialog if necessary, update it and display it.
	virtual void show();
	//
	virtual bool isVisible() const;
	GBC & bcview();
	void onApply();
	void onOK();
	void onCancel();
	void onRestore();
	bool onDeleteEvent(GdkEventAny *);
private:
	virtual Gtk::Window * window() = 0;
	virtual Gtk::Window const * window() const = 0;
	bool allowResize_;
};


template <class Dialog>
class GViewDB : public GViewBase {
protected:
	GViewDB(Dialog &, std::string const &, bool allowResize);
	virtual const Gtk::Window * window() const;
	virtual Gtk::Window * window();
	boost::scoped_ptr<Dialog> dialog_;
};


template <class Dialog>
GViewDB<Dialog>::GViewDB(Dialog & parent, std::string const & t, bool allowResize) :
	GViewBase(parent, t, allowResize)
{
}


template <class Dialog>
Gtk::Window * GViewDB<Dialog>::window()
{
	return dialog_.get();
}


template <class Dialog>
const Gtk::Window * GViewDB<Dialog>::window() const
{
	return dialog_.get();
}


class GViewGladeB : public GViewBase {
protected:
	GViewGladeB(Dialog & parent, std::string const & t, bool allowResize);
	virtual Gtk::Window const * window() const;
	virtual Gtk::Window * window();
	Glib::RefPtr<Gnome::Glade::Xml> xml_;
};


template <class Controller, class Base>
class GViewCB : public Base {
public:
	Controller & controller();
	Controller const & controller() const;
protected:
	GViewCB(Dialog & parent, std::string const & t, bool allowResize = false);
};


template <class Controller, class Base>
GViewCB<Controller, Base>::GViewCB(Dialog & parent, std::string const & t,
				   bool allowResize) :
	Base(parent, t, allowResize)
{
}


template <class Controller, class Base>
Controller & GViewCB<Controller, Base>::controller()
{
	return static_cast<Controller &>(getController());
}


template <class Controller, class Base>
Controller const & GViewCB<Controller, Base>::controller() const
{
	return static_cast<Controller const &>(getController());
}

} // namespace frontend
} // namespace lyx

#endif
