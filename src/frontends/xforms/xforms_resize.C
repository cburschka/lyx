/**
 * \file xforms_resize.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "xforms_resize.h"
#include "support/LAssert.h"
#include <algorithm> // std::max. Use FL_max in .c code...
#include FORMS_H_LOCATION

namespace {

/* This is hacked straight out of the xforms source.
   It is fl_adjust_form_size without the last few lines that do the
   adjusting.
 */
double get_scaling_factor(FL_FORM * form)
{
    FL_OBJECT *ob;
    float xfactor, yfactor, max_factor, factor;
    int sw, sh, osize;
    float xm = 0.5f, ym = 0.5f;
    int bw;

//      if (fl_no_connection)
//	return 1.0f;

    max_factor = factor = 1.0f;
    for (ob = form->first; ob; ob = ob->next)
    {
	if ((ob->align == FL_ALIGN_CENTER || (ob->align & FL_ALIGN_INSIDE) ||
	     ob->objclass == FL_INPUT) &&
	    !ob->is_child && *(ob->label) && ob->label[0] != '@' &&
	    ob->boxtype != FL_NO_BOX &&
	    (ob->boxtype != FL_FLAT_BOX || ob->objclass == FL_MENU))
	{
	    fl_get_string_dimension(ob->lstyle, ob->lsize, ob->label,
		       strlen(ob->label), &sw, &sh);

	    bw = (ob->boxtype == FL_UP_BOX || ob->boxtype == FL_DOWN_BOX) ?
		FL_abs(ob->bw) : 1;

	    if (ob->objclass == FL_BUTTON &&
		(ob->type == FL_RETURN_BUTTON || ob->type == FL_MENU_BUTTON))
		sw += FL_min(0.6f * ob->h, 0.6f * ob->w) - 1;

	    if (ob->objclass == FL_BUTTON && ob->type == FL_LIGHTBUTTON)
		sw += FL_LIGHTBUTTON_MINSIZE + 1;

	    if (sw <= (ob->w - 2 * (bw + xm)) && sh <= (ob->h - 2 * (bw + ym)))
		continue;

	    if ((osize = ob->w - 2 * (bw + xm)) <= 0)
		osize = 1;
	    xfactor = (float) sw / (float)osize;

	    if ((osize = ob->h - 2 * (bw + ym)) <= 0)
		osize = 1;
	    yfactor = (float) sh / osize;

	    if (ob->objclass == FL_INPUT)
	    {
		xfactor = 1.0f;
		yfactor = (sh + 1.6f) / osize;
	    }

	    if ((factor = FL_max(xfactor, yfactor)) > max_factor)
	    {
		max_factor = factor;
	    }
	}
    }

    if (max_factor <= 1.0f)
	return 1.0f;

    max_factor = 0.01f * (int) (max_factor * 100.0f);

    if (max_factor > 1.25f)
	max_factor = 1.25f;

    return max_factor;
}


double get_tabfolder_scale_to_fit(FL_OBJECT * folder)
{
	lyx::Assert(folder && folder->objclass == FL_TABFOLDER);

	fl_freeze_form(folder->form);
	int const saved_folder_id = fl_get_folder_number(folder);

	double factor = 1.0;
	int const size = fl_get_tabfolder_numfolders(folder);
	for (int i = 0; i < size; ++i) {
		fl_set_folder_bynumber(folder, i+1);
		FL_FORM * leaf = fl_get_folder(folder);
		factor = std::max(factor, get_scale_to_fit(leaf));
	}

	fl_set_folder_bynumber(folder, saved_folder_id);
	fl_unfreeze_form(folder->form);

	return factor;
}


void scale_tabfolder_horizontally(FL_OBJECT * folder, double factor)
{
	lyx::Assert(folder && folder->objclass == FL_TABFOLDER);

	fl_freeze_form(folder->form);
	int const saved_folder_id = fl_get_folder_number(folder);

	int const size = fl_get_tabfolder_numfolders(folder);
	for (int i = 0; i < size; ++i) {
		fl_set_folder_bynumber(folder, i+1);
		FL_FORM * leaf = fl_get_folder(folder);
		scale_form_horizontally(leaf, factor);
	}

	fl_set_folder_bynumber(folder, saved_folder_id);
	fl_unfreeze_form(folder->form);
}

} // namespace anon


double get_scale_to_fit(FL_FORM * form)
{
	lyx::Assert(form);

	double factor = get_scaling_factor(form);
	for (FL_OBJECT * ob = form->first; ob; ob = ob->next) {
		if (ob->objclass == FL_TABFOLDER)
			factor = std::max(factor,
					  get_tabfolder_scale_to_fit(ob));
	}
	return factor;
}


void scale_form_horizontally(FL_FORM * form, double factor)
{
	lyx::Assert(form);

	if (factor <= 1.0)
		return;

	fl_scale_form(form, factor, 1);

	for (FL_OBJECT * ob = form->first; ob; ob = ob->next) {
		if (ob->objclass == FL_TABFOLDER)
			scale_tabfolder_horizontally(ob, factor);
	}
}
