/* input_validators.C
 * A collection of input filtering and validating functions for use in
 * XForms popups.  Mainly meant for filtering input boxes although may
 * be extended to include other generally useful xforms-specific tools.
 */

#include "config.h"
#include FORMS_H_LOCATION
#include "input_validators.h"

#if defined(__cplusplus)
extern "C"
{
#endif

int fl_unsigned_int_filter(FL_OBJECT * ob,
			   char const * not_used,
			   char const * unused,
			   int c)
{
	if (c == 0 /* final test before handing contents to app */
	    || strchr("0123456789", c)) {
		/* since we only accept numerals then it must be valid */
		return FL_VALID;
	}
	return FL_INVALID|FL_RINGBELL;
}


int fl_lowercase_filter(FL_OBJECT * ob,
			char const * not_used,
			char const * unused,
			int c)
{
	if (c == 0 /* final test before handing contents to app */
	    || strchr("abcdefghijklmnopqrstuvwxyz", c)) {
		/* since we only accept numerals then it must be valid */
		return FL_VALID;
	}
	return FL_INVALID|FL_RINGBELL;
}


#if 0
/* I've just moved this code here and written a few comments.
   still to complete it.  ARRae 20000518 */

void fl_print_range_filter(FL_OBJECT * ob,
			   char const * not_used,
			   char const * unused,
			   int c)
{
	/* Started life as changes to PrintApplyCB by Stephan Witt
	   (stephan.witt@beusen.de), 19-Jan-99
	   User may give a page (range) list */

	if (strchr("0123456789", c)) {
		/* Numerals are always valid */
		return FL_VALID;
	} else if (strchr("-,", c)) {
		/* make sure that the character can go there */
	} else if (c == 0) {
		/* final test before handing contents to app
		   make sure the last char isn't a "-,"
		   That might be acceptable if there was a "to_page"
		   entry however if you start making a page range in the "from"
		   field you can do it all in the "from" field.  That is, a
		   range in the "from" field immmediately blanks the "to" 
		   field. */
	}
	return FL_INVALID|FL_RINGBELL;

	/* The code above should do the same sort of checking as the
	   code below. */

	string pages = subst(fl_get_input(fd_form_print->input_pages), ';',',');
	pages = subst(pages, '+',',');
	pages = frontStrip(strip(pages)) ;
	while (!pages.empty()) { // a page range was given
		string piece ;
		pages = split (pages, piece, ',') ;
		piece = strip(piece) ;
		piece = frontStrip(piece) ;
		if ( !stringOnlyContains (piece, "0123456789-") ) {
			WriteAlert(_("ERROR!  Unable to print!"),
				   _("Check 'range of pages'!"));
			return;
		}
		if (piece.find('-') == string::npos) { // not found
			pageflag += lyxrc.print_pagerange_flag + piece + '-' + piece + ' ' ;
		} else if (suffixIs(piece, "-") ) { // missing last page
			pageflag += lyxrc.print_pagerange_flag + piece + "1000 ";
		} else if (prefixIs(piece, "-") ) { // missing first page
			pageflag += lyxrc.print_pagerange_flag + '1' + piece + ' ' ;
		} else {
			pageflag += lyxrc.print_pagerange_flag + piece + ' ' ;
		}
	}
}
#endif 

#if defined(__cplusplus)
}
#endif
