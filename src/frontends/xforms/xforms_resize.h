// -*- C++ -*-
/**
 * \file xforms_resize.h
 * Copyright 2000-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef XFORMS_RESIZE_H
#define XFORMS_RESIZE_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION

/** Returns 1 if the form does not contain a tabfolder.
    If it does contain a tabfolder, then by how much should the form be
    scaled to render the tabs (buttons) visible.

    This routine works for two levels of nested tabfolders.
 */
double scale_to_fit_tabs(FL_FORM * form);

/** A wrapper for fl_scale_form(form, scale, 1) that scales the parent form
    horizontally, but also scales the leaves (FL_FORMs) of the tabfolder,
    should one exist.
 */
void scale_form(FL_FORM * form, double scale_factor);

#endif // XFORMS_RESIZE_H
