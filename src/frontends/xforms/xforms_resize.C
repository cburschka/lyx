/**
 * \file xforms_resize.C
 * Copyright 2000-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xforms_resize.h"

#include "LString.h"
#include <algorithm>
#include <vector>

using std::vector;

namespace {

// Returns the FL_OBJECT * of class FL_TABFOLDER if present in the form.
// Else 0.
FL_OBJECT * find_tabfolder(FL_FORM * form);

// Takes an FL_OBJECT * folder of class FL_TABFOLDER and calculates how much it
// should be scaled to display all the tabs.
// Returns 1 if the tabs are alll fully visible.
// Nested tabfolders are not considered.
double scale_to_fit_tabs(FL_OBJECT * folder);

// Returns the leaves making up a tabfolder.
vector<FL_FORM *> const leaves_in_tabfolder(FL_OBJECT * folder);

// The labels of each tab in the folder.
vector<string> const tab_names(FL_OBJECT * folder);

} // namespace anon


double scale_to_fit_tabs(FL_FORM * form)
{
	if (!form)
		return 1;

	FL_OBJECT * folder = find_tabfolder(form);
	if (!folder)
		return 1;

	double scale = scale_to_fit_tabs(folder);

	vector<FL_FORM *> const leaves = leaves_in_tabfolder(folder);
	vector<FL_FORM *>::const_iterator it  = leaves.begin();
	vector<FL_FORM *>::const_iterator end = leaves.end();

	for (; it != end; ++it) {
		folder = find_tabfolder(*it);
		if (!folder)
			continue;
		double folder_scale = scale_to_fit_tabs(folder);
		scale = std::max(scale, folder_scale);
	}

	return scale;
}


void scale_form(FL_FORM * form, double scale_factor)
{
	if (!form)
		return;

	fl_scale_form(form, scale_factor, 1);

	FL_OBJECT * folder = find_tabfolder(form);
	if (!folder)
		return;

	vector<FL_FORM *> const leaves = leaves_in_tabfolder(folder);
	vector<FL_FORM *>::const_iterator it  = leaves.begin();
	vector<FL_FORM *>::const_iterator end = leaves.end();

	for (; it != end; ++it) {
		fl_scale_form(*it, scale_factor, 1);
	}
}


namespace {

FL_OBJECT * find_tabfolder(FL_FORM * form)
{
	for (FL_OBJECT * ob = form->first; ob; ob = ob->next) {
		if (ob->objclass == FL_TABFOLDER)
			return ob;
	}

	return 0;
}


// Return the scaling factor needed to render all the tab labels.
double scale_to_fit_tabs(FL_OBJECT * folder)
{
	if (folder->objclass != FL_TABFOLDER)
		return 1;

	// The problem here is that we can access the names of a tabfolder's
	// tabs directly (through fl_get_folder_name(folder)), but we
	// can't access directly the tabs themselves. Nonetheless, they are
	// made visible as buttons on the parent form, so we can obtain
	// the style info by a judicious camparison of names. This style info
	// (label font size, style) is needed to ascertain how much space
	// the label takes up.

	// The tabfolder makes the tabs (buttons) visible to the parent form,
	// so to identify them, we need their names.
	vector<string> const names = tab_names(folder);
	if (names.empty())
		return 1;

	FL_FORM * parent = folder->form;
	if (!parent)
		return 1;

	// Ascertain the style parameters of the tabs.
	int label_style = FL_BOLD_STYLE;
	int label_size  = FL_NORMAL_SIZE;
	int box_width   = 1;
	// Hard-coded within xforms' fl_create_tabfolder in tabfolder.c.
	int const tab_padding =  12;

	for (FL_OBJECT * ob = parent->first; ob; ob = ob->next) {
		if (!ob->label)
			continue;

		vector<string>::const_iterator it  = names.begin();
		vector<string>::const_iterator end = names.end();
		it = std::find(it, end, ob->label);

		if (it == end)
			continue;

		label_style = ob->lstyle;
		label_size  = ob->lsize;
		
		if (ob->boxtype == FL_UP_BOX || ob->boxtype == FL_DOWN_BOX)
			box_width = FL_abs(ob->bw);

		// achieved all we set out to achieve, so break.
		break;
	}
	// Hard coded in xforms' get_tabsize in tabfolder.c as
	// (2 + fudge) * box_width, where fudge is always 1.
	int const box_width_factor = 3 * box_width;

	// Loop over the names and calculate how much space is needed to display
	// them.
	int length = 0;

	vector<string>::const_iterator it  = names.begin();
	vector<string>::const_iterator end = names.end();

	for (; it != end; ++it) {
		int sw, sh;

		fl_get_string_dimension(label_style, label_size,
					it->c_str(), int(it->size()),
					&sw, &sh);

		// This is the minimum width the object must be to contain
		// the label
		length += sw + tab_padding + box_width_factor;
	}

	// Compare this length to the width of the tabfolder
	double scale = double(length) / double(folder->w);
	return std::max(double(1), scale);
}


// Returns the leaves making up a tabfolder.
vector<FL_FORM *> const leaves_in_tabfolder(FL_OBJECT * folder)
{
	if (folder->objclass != FL_TABFOLDER)
		return vector<FL_FORM *>();

	fl_freeze_form(folder->form);
	int const folder_id = fl_get_folder_number(folder);

	int const size = fl_get_tabfolder_numfolders(folder);
	vector<FL_FORM *> leaves(size);

	for (int i = 0; i < size; ++i) {
		fl_set_folder_bynumber(folder, i+1);
		leaves[i] = fl_get_folder(folder);
	}

	fl_set_folder_bynumber(folder, folder_id);
	fl_unfreeze_form(folder->form);

	return leaves;
}


// The labels of each tab in the folder.
vector<string> const tab_names(FL_OBJECT * folder)
{
	if (folder->objclass != FL_TABFOLDER)
		return vector<string>();

	fl_freeze_form(folder->form);
	int const folder_id = fl_get_folder_number(folder);

	int const size = fl_get_tabfolder_numfolders(folder);
	vector<string> names(size);

	for (int i = 0; i < size; ++i) {
		fl_set_folder_bynumber(folder, i+1);

		const char * const name = fl_get_folder_name(folder);
		if (name)
			names[i] = name;
	}

	fl_set_folder_bynumber(folder, folder_id);
	fl_unfreeze_form(folder->form);

	return names;
}

} // namespace anon
