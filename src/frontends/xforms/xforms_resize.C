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

#ifdef __GNUG__
#pragma implementation
#endif

#include "xforms_resize.h"
#include "support/LAssert.h"
#include <algorithm> // std::max. Use FL_max in .c code...
#include FORMS_H_LOCATION

namespace {

// A nasty hack for older xforms versions
int get_tabfolder_numfolders(FL_OBJECT * folder)
{
#if FL_VERSION > 0 || FL_REVISION > 88
	return fl_get_tabfolder_numfolders(folder);
#else
	if (folder->objclass != FL_TABFOLDER)
		return 0;

	fl_freeze_form(folder->form);
	int const saved_folder_id = fl_get_folder_number(folder);

	int num_folders = 0;
	FL_FORM const * old_leaf = 0;
	for (;;) {
		int const id = num_folders + 1;
		fl_set_folder_bynumber(folder, id);
		FL_FORM const * const leaf = fl_get_folder(folder);
		if (!leaf || leaf == old_leaf) {
			// unable to increment succesfully.
			break;
		}
		old_leaf = leaf;
		++num_folders;
	}

	fl_set_folder_bynumber(folder, saved_folder_id);
	fl_unfreeze_form(folder->form);

	return num_folders;
#endif
}


double get_scaling_factor(FL_FORM * form)
{
	// fl_adjust_form_size gives us the info we desire but also resizes
	// the form, which we don't want :-(
	double factor = fl_adjust_form_size(form);
	if (factor > 1.001) {
		double const inv = 1.0 / factor;
		fl_scale_form(form, inv, inv);
	}
	return factor;
}


double get_tabfolder_scale_to_fit(FL_OBJECT * folder)
{
	lyx::Assert(folder && folder->objclass == FL_TABFOLDER);

	fl_freeze_form(folder->form);
	int const saved_folder_id = fl_get_folder_number(folder);

	double factor = 1.0;
	int const size = get_tabfolder_numfolders(folder);
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

	int const size = get_tabfolder_numfolders(folder);
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
