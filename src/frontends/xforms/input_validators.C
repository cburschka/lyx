/* input_validators.C
 * A collection of input filtering and validating functions for use in
 * XForms dialogs.  Mainly meant for filtering input boxes although may
 * be extended to include other generally useful xforms-specific tools.
 */

#include <config.h>


#include "lyx_forms.h"
#include "support/lstrings.h"
#include "input_validators.h"

using namespace lyx::support;

#if defined(__cplusplus)
extern "C"
{
#endif

int fl_int_filter(FL_OBJECT * ob,
		  char const *, char const *, int c)
{
	if (c == 0 /* final test before handing contents to app */
	    || strchr("0123456789+-", c)) {
		if (isStrInt(fl_get_input(ob)))
			return FL_VALID;
	}
	return FL_INVALID|FL_RINGBELL;
}


int fl_unsigned_int_filter(FL_OBJECT * /*ob*/,
			   char const *, char const *, int c)
{
	if (c == 0 /* final test before handing contents to app */
	    || strchr("0123456789", c)) {
		/* since we only accept numerals then it must be valid */
		return FL_VALID;
	}
	return FL_INVALID|FL_RINGBELL;
}


int fl_float_filter(FL_OBJECT * ob,
		    char const *, char const *, int c)
{
	if (c == 0 /* final test before handing contents to app */
	    || strchr("0123456789.+-", c)) {
		if (isStrDbl(fl_get_input(ob)))
			return FL_VALID;
	}
	return FL_INVALID|FL_RINGBELL;
}


int fl_unsigned_float_filter(FL_OBJECT * ob,
			     char const * /*not_used*/,
			     char const * /*unused*/,
			     int c)
{
	if (c == 0 /* final test before handing contents to app */
	    || strchr("0123456789.", c)) {
		if (isStrDbl(fl_get_input(ob)))
			return FL_VALID;
	}
	return FL_INVALID|FL_RINGBELL;
}


int fl_lowercase_filter(FL_OBJECT * /*ob*/,
			char const * /*not_used*/,
			char const * /*unused*/,
			int c)
{
	if (c == 0 /* final test before handing contents to app */
	    || strchr("abcdefghijklmnopqrstuvwxyz0123456789", c)) {
		/* since we only accept numerals then it must be valid */
		return FL_VALID;
	}
	return FL_INVALID|FL_RINGBELL;
}


#if defined(__cplusplus)
}
#endif
