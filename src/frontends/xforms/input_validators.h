// -*- C++ -*-
/**
 * \file input_validators.h
 * Copyright 1999-2001 Allan Rae
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS
 */

/* Input validators and filters for xforms.
 * A collection of input filtering and validating functions for use in
 * XForms dialogs.  Mainly meant for filtering input boxes although may
 * be extended to include other generally useful xforms-specific tools.
 */

#ifndef INPUT_VALIDATORS_H
#define INPUT_VALIDATORS_H

#ifdef __GNUG__
#pragma interface
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

    /** Only allow integer numbers,
	possibly preceeded by a +' or '-' sign */
int fl_int_filter(FL_OBJECT *, char const *, char const *, int);
    /** Only allow integer numbers. No '+' or '-' signs. */
int fl_unsigned_int_filter(FL_OBJECT *, char const *, char const *, int);
    /** Only allow floating point numbers,
	possibly preceeded by a +' or '-' sign */
int fl_float_filter(FL_OBJECT *, char const *, char const *, int);
    /** Only allow floating point numbers. No '+' or '-' signs. */
int fl_unsigned_float_filter(FL_OBJECT *, char const *, char const *, int);

    /** Only allow lowercase letters. */
int fl_lowercase_filter(FL_OBJECT *, char const *, char const *, int);

#if defined(__cplusplus)
}
#endif

#endif
