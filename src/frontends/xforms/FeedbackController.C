/**
 * \file FeedbackController.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

/* A common interface for posting feedback messages to a message widget in
 * xforms.
 * Derive FormBase and FormBaseDeprecated from it, so daughter classes of
 * either can interface tooltips in the same way.
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FeedbackController.h"
#include "gettext.h"        // _()
#include "xforms_helpers.h" // formatted
#include "support/LAssert.h"

FeedbackController::FeedbackController()
	: warning_posted_(false)
{}


FeedbackController::~FeedbackController()
{}


void FeedbackController::setMessageWidget(FL_OBJECT * ob)
{
	lyx::Assert(ob && ob->objclass  == FL_TEXT);
	message_widget_ = ob;
}


// preemptive handler for feedback messages
void FeedbackController::MessageCB(FL_OBJECT * ob, int event)
{
	if (!message_widget_) {
		// fail silently.
		return;
	}

	lyx::Assert(ob);

	switch (event) {
	case FL_ENTER:
		warning_posted_ = false;
		postMessage(getFeedback(ob));
		break;

	case FL_LEAVE:
		if (!warning_posted_)
			fl_set_object_label(message_widget_, "");
		break;

	default:
		break;
	}
}


void FeedbackController::PrehandlerCB(FL_OBJECT * ob, int event, int key)
{
	if (event == FL_PUSH && key == 2 && ob->objclass == FL_INPUT) {
		// Trigger an input event when pasting in an xforms input object
		// using the middle mouse button.
		InputCB(ob, 0);

	} else if (event == FL_ENTER || event == FL_LEAVE){
		// Post feedback as the mouse enters the object,
		// remove it as the mouse leaves.
		MessageCB(ob, event);
	}
}


void FeedbackController::postWarning(string const & warning)
{
	lyx::Assert(message_widget_);

	warning_posted_ = true;

	string const str = _("WARNING! ") + warning;
	postMessage(str);
}


void FeedbackController::postMessage(string const & message)
{
	string const str = formatted(message,
				     message_widget_->w-10, FL_SMALL_SIZE);

	fl_set_object_label(message_widget_, str.c_str());
	fl_set_object_lsize(message_widget_, FL_SMALL_SIZE);
}
