/**
 * \file KDEFormBase.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef KDEFORMBASE_H
#define KDEFORMBASE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "boost/smart_ptr.hpp"
 
#include "ViewBase.h"
#include "ButtonPolicies.h"
#include "kdeBC.h"

class QWidget;
 
class KFormDialogBase : public ViewBC<kdeBC> {
public:
	///
	KFormDialogBase(ControlButtons & c);

	// Functions accessible to the Controller

	/// build dialog
	virtual void build() = 0;
	/// create dialog if necessary, update and display
	void show();
	/// hide dialog
	virtual void hide();

private:
	/// get dialog
	virtual QWidget * dialog() const = 0;
};

template <class Controller, class Dialog>
	class KFormBase : public KFormDialogBase {
public:
	///
	KFormBase(ControlButtons & c);
 
protected:
	/// parent controller 
	Controller & controller() const;

	/// get dialog 
	virtual QWidget * dialog() const { return dialog_.get(); };
 
	/// dialog implemenation 
	boost::scoped_ptr<Dialog> dialog_; 
};

template <class Controller, class Dialog>
	KFormBase<Controller, Dialog>::KFormBase(ControlButtons & c) 
	: KFormDialogBase(c)
{
}


template <class Controller, class Dialog>
	Controller & KFormBase<Controller, Dialog>::controller() const
{
	return static_cast<Controller &>(controller_);
}
 
 
#endif // KDEFORMBASE_H
