// -*- C++ -*-
/* DialogBase.h
 * Abstract base class of all dialogs.
 * Author: Allan Rae <rae@lyx.org>
 * This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file Copyright 2000
 *           Allan Rae
 * ======================================================
 */
#ifndef DIALOGBASE_H
#define DIALOGBASE_H

#include <sigc++/signal_system.h>
#include <boost/utility.hpp>

/** Abstract base class of all dialogs.
    The outside world only needs some way to tell a dialog when to show, hide
    or update itself.  A dialog then takes whatever steps are necessary to
    satisfy that request.  Thus a dialog will have to "pull" the necessary
    details from the core of the program.
 */
class DialogBase : public SigC::Object, public boost::noncopyable
{
public:
	/**@name Constructors and Deconstructors */
	//@{
	/// Virtual base destructor
	virtual ~DialogBase() {}
	//@}

	/**@name Signal Targets */
	//@{
	///
	//virtual void show() = 0;
	///
	virtual void hide() = 0;
	///
	virtual void update() {};
	///
	virtual void close() {};
	//@}
};

#endif // DIALOGBASE_H
