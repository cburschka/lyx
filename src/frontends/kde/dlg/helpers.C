/*
 * helpers.C
 * (C) 2001 LyX Team
 *
 * John Levon, moz@compsoc.man.ac.uk
 */
 
/**
 * \file helpers.C
 * \brief various useful functions for manipulating dialog widgets
 */

#include <config.h>
 
#include "helpers.h"

#include "gettext.h"
#include "debug.h"

using std::endl;

namespace kde_helpers {

bool setComboFromStr(QComboBox *box, const string & str)
{
	for (int i = 0; i < box->count(); i++) {
		if (str == box->text(i)) {
			box->setCurrentItem(i);
			return true;
		}
	}
	
	return false;
}

} // namespace kde_helpers
