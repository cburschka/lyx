/**
 * \file FormBaseDeprecated.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "FormBaseDeprecated.h"
#include "xformsBC.h"
#include "xforms_resize.h"
#include "GUIRunTime.h"
#include "Tooltips.h"
#include "lyxrc.h"

#include "frontends/LyXView.h"

#include "support/LAssert.h"

#include <boost/bind.hpp>

extern "C" {

// Callback function invoked by xforms when the dialog is closed by the
// window manager
static int C_WMHideCB(FL_FORM *, void *);

// Callback function invoked by the xforms pre- and post-handler routines
static int C_PrehandlerCB(FL_OBJECT *, int, FL_Coord, FL_Coord, int, void *);

} // extern "C"


FormBaseDeprecated::FormBaseDeprecated(LyXView * lv, Dialogs * d,
				       string const & t, bool allowResize)
	: lv_(lv), d_(d), title_(t),
	  minw_(0), minh_(0), allow_resize_(allowResize),
	  tooltips_(new Tooltips)
{
	lyx::Assert(lv && d);
}


FormBaseDeprecated::~FormBaseDeprecated()
{
	delete tooltips_;
}


Tooltips & FormBaseDeprecated::tooltips()
{
	return *tooltips_;
}


void FormBaseDeprecated::redraw()
{
	if (form() && form()->visible)
		fl_redraw_form(form());
}


void FormBaseDeprecated::connect()
{
	fl_set_form_minsize(form(), minw_, minh_);
	r_ = Dialogs::redrawGUI.connect(boost::bind(&FormBaseDeprecated::redraw, this));
}


void FormBaseDeprecated::disconnect()
{
	h_.disconnect();
	r_.disconnect();
}


void FormBaseDeprecated::show()
{
	if (!form()) {
		build();

		double const scale = scale_to_fit_tabs(form());
		if (scale > 1.001)
			scale_form(form(), scale);

		bc().refresh();

		// work around dumb xforms sizing bug
		minw_ = form()->w;
		minh_ = form()->h;

		fl_set_form_atclose(form(), C_WMHideCB, 0);
	}

	fl_freeze_form(form());
	update();
	fl_unfreeze_form(form());

	if (form()->visible) {
		fl_raise_form(form());
		/* This XMapWindow() will hopefully ensure that
		 * iconified dialogs are de-iconified. Mad props
		 * out to those crazy Xlib guys for forgetting a
		 * XDeiconifyWindow(). At least WindowMaker, when
		 * being notified of the redirected MapRequest will
		 * specifically de-iconify. From source, fvwm2 seems
		 * to do the same.
		 */
		XMapWindow(fl_get_display(), form()->window);
	} else {
		connect();

		// calls to fl_set_form_minsize/maxsize apply only to the next
		// fl_show_form(), so this comes first.
		fl_set_form_minsize(form(), minw_, minh_);
		if (!allow_resize_)
			fl_set_form_maxsize(form(), minw_, minh_);

		fl_show_form(form(),
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     (lyxrc.dialogs_iconify_with_main ? FL_TRANSIENT : 0),
			     title_.c_str());
	}

	tooltips().set();
}


void FormBaseDeprecated::hide()
{
	// xforms sometimes tries to process a hint-type MotionNotify, and
	// use XQueryPointer, without verifying if the window still exists.
	// So we try to clear out motion events in the queue before the
	// DestroyNotify
	XSync(fl_get_display(), false);
	GUIRunTime::processEvents();

	if (form() && form()->visible) {
		// some dialogs might do things to the form first
		// such as the nested tabfolder problem in Preferences
		disconnect();
		fl_hide_form(form());
	}
}


void FormBaseDeprecated::setPrehandler(FL_OBJECT * ob)
{
	lyx::Assert(ob);
	fl_set_object_prehandler(ob, C_PrehandlerCB);
}


void FormBaseDeprecated::WMHideCB()
{
	hide();
	bc().hide();
}


void FormBaseDeprecated::ApplyCB()
{
	apply();
	bc().apply();
}


void FormBaseDeprecated::OKCB()
{
	ok();
	bc().ok();
}


void FormBaseDeprecated::CancelCB()
{
	cancel();
	bc().cancel();
}


void FormBaseDeprecated::InputCB(FL_OBJECT * ob, long data)
{
	// It is possible to set the choice to 0 when using the
	// keyboard shortcuts. This work-around deals with the problem.
	if (ob && ob->objclass == FL_CHOICE && fl_get_choice(ob) < 1) {
		fl_set_choice(ob, 1);
	}

	bc().valid(input(ob, data));
}


void FormBaseDeprecated::RestoreCB()
{
	bc().restore();
	restore();
}


FormBaseBI::FormBaseBI(LyXView * lv, Dialogs * d, string const & t,
		       bool allowResize)
	: FormBaseDeprecated(lv, d, t, allowResize)
{}


void FormBaseBI::connect()
{
	h_ = d_->hideAll.connect(boost::bind(&FormBaseBI::hide, this));
	FormBaseDeprecated::connect();
}


FormBaseBD::FormBaseBD(LyXView * lv, Dialogs * d, string const & t,
		       bool allowResize)
	: FormBaseDeprecated(lv, d, t, allowResize)
{}


void FormBaseBD::connect()
{
	u_ = d_->updateBufferDependent.
		connect(boost::bind(&FormBaseBD::updateSlot, this, _1));
	h_ = d_->hideBufferDependent.
		connect(boost::bind(&FormBaseBD::hide, this));
	FormBaseDeprecated::connect();
}


void FormBaseBD::disconnect()
{
	u_.disconnect();
	FormBaseDeprecated::disconnect();
}


namespace {

FormBaseDeprecated * GetForm(FL_OBJECT * ob)
{
	lyx::Assert(ob && ob->form && ob->form->u_vdata);
	FormBaseDeprecated * ptr =
		static_cast<FormBaseDeprecated *>(ob->form->u_vdata);
	return ptr;
}

} // namespace anon


extern "C" {

void C_FormBaseDeprecatedApplyCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->ApplyCB();
}


void C_FormBaseDeprecatedOKCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->OKCB();
}


void C_FormBaseDeprecatedCancelCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->CancelCB();
}


void C_FormBaseDeprecatedInputCB(FL_OBJECT * ob, long d)
{
	GetForm(ob)->InputCB(ob, d);
}


void C_FormBaseDeprecatedRestoreCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->RestoreCB();
}

static int C_WMHideCB(FL_FORM * form, void *)
{
	// Close the dialog cleanly, even if the WM is used to do so.
	lyx::Assert(form && form->u_vdata);
	FormBaseDeprecated * ptr =
		static_cast<FormBaseDeprecated *>(form->u_vdata);
	ptr->WMHideCB();
	return FL_CANCEL;
}

static int C_PrehandlerCB(FL_OBJECT * ob, int event,
			  FL_Coord, FL_Coord, int key, void *)
{
	// Note that the return value is important in the pre-emptive handler.
	// Don't return anything other than 0.
	lyx::Assert(ob);

	// Don't Assert this one, as it can happen quite naturally when things
	// are being deleted in the d-tor.
	//Assert(ob->form);
	if (!ob->form) return 0;

	FormBaseDeprecated * ptr =
		static_cast<FormBaseDeprecated *>(ob->form->u_vdata);

	if (ptr)
		ptr->PrehandlerCB(ob, event, key);

	return 0;
}

} // extern "C"
