// -*- C++ -*-
/**
 * \file xforms_resize.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
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

/** Ascertains the scaling factor needed to ensure that all labels fit
    within their button.

    Tabfolders are searched also by recursively calling this function.
 */
double get_scale_to_fit(FL_FORM * form);

/** A wrapper for fl_scale_form(form, scale, 1) that scales the parent form
    horizontally, but also scales the leaves (FL_FORMs) of any nested
    tabfolder, should one exist.

    Tabfolders are scaled also by recursively calling this function.
 */
void scale_form_horizontally(FL_FORM * form, double factor);

#endif // XFORMS_RESIZE_H
