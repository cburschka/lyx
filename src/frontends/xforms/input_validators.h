/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 *           This file Copyright 1999-2001
 *           Allan Rae
 * ====================================================== */

/** Input validators and filters for xforms.
 * A collection of input filtering and validating functions for use in
 * XForms dialogs.  Mainly meant for filtering input boxes although may
 * be extended to include other generally useful xforms-specific tools.
 */

#ifndef INPUT_VALIDATORS_H
#define INPUT_VALIDATORS_H

#if defined(__cplusplus)
extern "C"
{
#endif

    /** Only allow whole numbers no '+' or '-' signs or exponents. */
int fl_unsigned_int_filter(FL_OBJECT *, char const *, char const *, int);
    /** Only allow whole numbers no '+' or '-' signs or exponents. */
int fl_unsigned_float_filter(FL_OBJECT *, char const *, char const *, int);

    /** Only allow lowercase letters. */
int fl_lowercase_filter(FL_OBJECT *, char const *, char const *, int);

#if defined(__cplusplus)
}
#endif

#endif
