/*
 * \file Tooltips.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 *
 * Tooltips for xforms. xforms 0.89 supports them directly, but 0.88 needs
 * a bit of jiggery pokery. This class wraps it all up in a neat interface.
 * Based on code originally in Toolbar_pimpl.C that appears to have been
 * written by Matthias Ettrich and Jean-Marc Lasgouttes.
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Tooltips.h"
#include "support/LAssert.h"

//#if FL_REVISION >= 89
// Usually, this is all that is needed for xforms 0.89
// However, I can't see an easy way to change Tooltips on the fly
// with this method, so for now use the jiggery pokery below. ;-)
// Angus 6 Feb 2002

/*
void Tooltips::activateTooltip(FL_OBJECT * ob)
{
	lyx::Assert(ob);

	string const help(getTooltip(ob));
	if (!help.empty())
		fl_set_object_helper(ob, help.c_str());	
}
*/

//#else // if FL_REVISION < 89

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
 

void Tooltips::activateTooltip(FL_OBJECT * ob)
{
	lyx::Assert(ob);

	if (!tooltip_timer_) {
		lyx::Assert(ob->form);
		fl_addto_form(ob->form);
		tooltip_timer_ = fl_add_timer(FL_HIDDEN_TIMER, 0, 0, 0, 0, "");
		fl_end_form();
	}

	fl_set_object_posthandler(ob, C_TooltipHandler);
	ob->u_cdata = reinterpret_cast<char *>(tooltip_timer_);
	tooltip_timer_->u_vdata = this;
}


namespace {

void TooltipTimerCB(FL_OBJECT * timer, long data)
{
	FL_OBJECT * ob = reinterpret_cast<FL_OBJECT*>(data);
	lyx::Assert(ob && ob->form && timer && timer->u_vdata);
	FL_FORM * form = ob->form;
	Tooltips * tooltip = static_cast<Tooltips *>(timer->u_vdata);
	
	string const help = tooltip->getTooltip(ob);
	if (help.empty())
		return;

	fl_show_oneliner(help.c_str(),
			 form->x + ob->x, form->y + ob->y + ob->h);
}


// post_handler for bubble-help (Matthias)
int TooltipHandler(FL_OBJECT *ob, int event)
{
	lyx::Assert(ob);
	FL_OBJECT * timer = reinterpret_cast<FL_OBJECT *>(ob->u_cdata);
	lyx::Assert(timer);

	// We do not test for empty help here, since this can never happen
	if (event == FL_ENTER){
		fl_set_object_callback(timer,
				       C_TooltipTimerCB,
				       reinterpret_cast<long>(ob));
		fl_set_timer(timer, 1);
	}
	else if (event != FL_MOTION){
		fl_set_timer(timer, 0);
		fl_hide_oneliner();
	}
	return 0;
}

} // namespace anon

//#endif // FL_REVISION < 89
