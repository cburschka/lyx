// -*- C++ -*-
/*
 * \file FeedbackController.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 **/

/* A common interface for posting feedback messages to a message widget in
 * xforms.
 * Derive FormBase and FormBaseDeprecated from it, so daughter classes of
 * either can interface tooltips in the same way.
 */

#ifndef FEEDBACKCONTROLLER_H
#define FEEDBACKCONTROLLER_H

#include FORMS_H_LOCATION
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

class FeedbackController
{
public:
	///
	FeedbackController();
	///
	virtual ~FeedbackController();

	/** Input callback function, invoked only by the xforms callback
	    interface. Is defined by FormBase, FormBaseDeprecated. */
	virtual void InputCB(FL_OBJECT *, long) = 0;

	/** Message callback function, invoked only by the xforms callback
	    interface */
	void MessageCB(FL_OBJECT *, int event);

	/** Prehandler callback function, invoked only by the xforms callback
	    interface */
	void PrehandlerCB(FL_OBJECT * ob, int event, int key);

protected:
	/** Pass the class a pointer to the message_widget so that it can
	    post the message */
	void setMessageWidget(FL_OBJECT * message_widget);

	/** Send the warning message from the daughter class to the
	    message_widget direct. The message will persist till the mouse
	    movesto a new object. */
	void postWarning(string const & warning);
	/// Reset the message_widget_
	void clearMessage();

private:
	/** Get the feedback message for ob.
	    Called if warning_posted_ == false. */
	virtual string const getFeedback(FL_OBJECT * /* ob */)
		{ return string(); }

	/// Post the feedback message for ob to message_widget_
	void postMessage(string const & message);

	/** Variable used to decide whether to remove the existing feedback
	    message or not (if it is in fact a warning) */
	bool warning_posted_;

	/// The widget to display the feedback
	FL_OBJECT * message_widget_;
};

#endif // FEEDBACKCONTROLLER_H
