/*
 * \file Tooltips.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

/* Tooltips for xforms. xforms 0.89 supports them directly, but 0.88 needs
 * a bit of jiggery pokery. This class wraps it all up in a neat interface.
 * Based on code originally in Toolbar_pimpl.C that appears to have been
 * written by Matthias Ettrich and Jean-Marc Lasgouttes.
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Tooltips.h"
#include "xforms_helpers.h" // formatted
#include "gettext.h"
#include "support/lstrings.h"
#include "support/LAssert.h"
#include FORMS_H_LOCATION

#include <boost/bind.hpp>

bool Tooltips::enabled_ = true;

boost::signal0<void> Tooltips::toggled;


#if FL_VERSION > 0 || FL_REVISION >= 89

Tooltips::Tooltips()
{
	toggled.connect(boost::bind(&Tooltips::set, this));
}


void Tooltips::toggleEnabled()
{
	enabled_ = !enabled_;
	toggled();
}


void Tooltips::set()
{
	if (tooltipsMap.empty())
		// There are no objects with tooltips in this dialog, so
		// just go away. Don't change the cursor to a question mark.
		return;

	TooltipsMap::iterator it  = tooltipsMap.begin();
	TooltipsMap::iterator end = tooltipsMap.end();
	for (; it != end; ++it) {
		FL_OBJECT * const ob = it->first;
		char const * const c_str = enabled_ ? it->second.c_str() : 0;
		fl_set_object_helper(ob, c_str);
	}
}


void Tooltips::init(FL_OBJECT * ob, string const & tip)
{
	lyx::Assert(ob && ob->form);

	// Paranoia check!
	TooltipsMap::const_iterator it = tooltipsMap.find(ob);
	if (it != tooltipsMap.end())
		return;

	string const str = trim(tip);
	if (str.empty())
		return;

	// Store the tooltip string
	tooltipsMap[ob] = formatted(str, 400);
}


#else // if FL_REVISION < 89

namespace {

int TooltipHandler(FL_OBJECT *ob, int event);

void TooltipTimerCB(FL_OBJECT * timer, long data);

}

extern "C" {

static int C_TooltipHandler(FL_OBJECT * ob, int event,
				    FL_Coord, FL_Coord, int, void *)
{
	return TooltipHandler(ob, event);
}


static void C_TooltipTimerCB(FL_OBJECT * ob, long data)
{
	TooltipTimerCB(ob, data);
}

}


Tooltips::Tooltips(Dialogs & d)
	: tooltip_timer_(0)
{
	static bool first = true;
	if (first) {
		first = false;
		d.toggleTooltips.connect(boost::bind(&Tooltips::toggleEnabled));
	}
	toggled.connect(boost::bind(&Tooltips::set, this));
}


void Tooltips::toggleEnabled()
{
	enabled_ = !enabled_;
	toggled();
}


void Tooltips::set()
{}


void Tooltips::init(FL_OBJECT * ob, string const & tip)
{
	lyx::Assert(ob && ob->form);

	// Paranoia check!
	TooltipsMap::const_iterator it = tooltipsMap.find(ob);
	if (it != tooltipsMap.end())
		return;

	string const str = trim(tip);
	if (str.empty())
		return;

	// Store the tooltip string
	tooltipsMap[ob] = formatted(str, 400);

	if (!tooltip_timer_) {
		if (fl_current_form && ob->form != fl_current_form)
			fl_end_form();

		bool const open_form = !fl_current_form;
		if (open_form)
			fl_addto_form(ob->form);

		tooltip_timer_ = fl_add_timer(FL_HIDDEN_TIMER, 0, 0, 0, 0, "");

		if (open_form)
			fl_end_form();
	}

	fl_set_object_posthandler(ob, C_TooltipHandler);
	ob->u_cdata = reinterpret_cast<char *>(tooltip_timer_);
	tooltip_timer_->u_vdata = this;
}


string const Tooltips::get(FL_OBJECT * ob) const
{
	TooltipsMap::const_iterator it = tooltipsMap.find(ob);
	if (it == tooltipsMap.end())
		return string();
	return it->second;
}


namespace {

void TooltipTimerCB(FL_OBJECT * timer, long data)
{
	FL_OBJECT * ob = reinterpret_cast<FL_OBJECT*>(data);
	lyx::Assert(ob && ob->form && timer && timer->u_vdata);
	FL_FORM * form = ob->form;
	Tooltips * tooltip = static_cast<Tooltips *>(timer->u_vdata);

	string const help = tooltip->get(ob);
	if (help.empty())
		return;

	fl_show_oneliner(help.c_str(),
			 form->x + ob->x, form->y + ob->y + ob->h);
}


// post_handler for tooltip help
int TooltipHandler(FL_OBJECT * ob, int event)
{
	if (!Tooltips::enabled())
		return 0;

	lyx::Assert(ob);
	FL_OBJECT * timer = reinterpret_cast<FL_OBJECT *>(ob->u_cdata);
	lyx::Assert(timer);

	// We do not test for empty help here, since this can never happen
	if (event == FL_ENTER) {
		fl_set_object_callback(timer,
				       C_TooltipTimerCB,
				       reinterpret_cast<long>(ob));
		fl_set_timer(timer, 1);
	}
	else if (event != FL_MOTION) {
		fl_set_timer(timer, 0);
		fl_hide_oneliner();
	}
	return 0;
}

} // namespace anon

#endif // FL_REVISION >= 89
