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
 * either can use the same interface.
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
	: warning_posted_(false), message_widget_(0)
{}


FeedbackController::~FeedbackController()
{}


void FeedbackController::setMessageWidget(FL_OBJECT * ob)
{
	lyx::Assert(ob && ob->objclass == FL_TEXT);
	message_widget_ = ob;
	fl_set_object_lsize(message_widget_, FL_SMALL_SIZE);
}


// preemptive handler for feedback messages
void FeedbackController::MessageCB(FL_OBJECT * ob, int event)
{
	lyx::Assert(ob);

	switch (event) {
	case FL_ENTER:
	{
		string const feedback = getFeedback(ob);
		if (feedback.empty() && warning_posted_)
			break;

		warning_posted_ = false;
		postMessage(getFeedback(ob));
		break;
	}

	case FL_LEAVE:
		if (!warning_posted_)
			clearMessage();
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

	} else if (message_widget_ &&
		   (event == FL_ENTER || event == FL_LEAVE)) {
		// Post feedback as the mouse enters the object,
		// remove it as the mouse leaves.
		MessageCB(ob, event);
	}
}


void FeedbackController::postWarning(string const & warning)
{
	warning_posted_ = true;
	postMessage(warning);
}


void FeedbackController::clearMessage()
{
	lyx::Assert(message_widget_);

	warning_posted_ = false;

	string const existing = message_widget_->label
		? message_widget_->label : string();
	if (existing.empty())
		return;

	// This trick is needed to get xforms to clear the label...
	fl_set_object_label(message_widget_, "");
	fl_hide_object(message_widget_);
}


void FeedbackController::postMessage(string const & message)
{
	lyx::Assert(message_widget_);

	string str;
	if (warning_posted_)
		str = _("WARNING! ") + message;
	else
		str = message;

	str = formatted(str, message_widget_->w-10, FL_SMALL_SIZE);

	fl_set_object_label(message_widget_, str.c_str());
	FL_COLOR const label_color = warning_posted_ ? FL_TOMATO : FL_BLACK;
	fl_set_object_lcol(message_widget_, label_color);

	if (!message_widget_->visible)
		fl_show_object(message_widget_);
}
