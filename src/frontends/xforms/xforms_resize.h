// -*- C++ -*-
/**
 * \file xforms_resize.h
 * Read the file COPYING
 *
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef XFORMS_RESIZE_H
#define XFORMS_RESIZE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "forms_fwd.h"

/** If the form contains a tabfolder, then by how much should the form be
    scaled (enlarged only) to render the tabs (buttons) visible?

    Returns 1 if the form does not contain a tabfolder.

    This routine works for two levels of nested tabfolders. (I.e., is sufficient
    for our current needs.)
 */
double scale_to_fit_tabs(FL_FORM * form);

/** A wrapper for fl_scale_form(form, scale, 1) that scales the parent form
    horizontally, but also scales the leaves (FL_FORMs) of any nested
    tabfolder, should one exist.
 */
void scale_form(FL_FORM * form, double scale_factor);

#endif // XFORMS_RESIZE_H
