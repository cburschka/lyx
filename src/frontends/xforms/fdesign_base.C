/**
 * \file fdesign_base.C
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

#include "fdesign_base.h"
#include FORMS_H_LOCATION

FD_base::~FD_base()
{
	if (!form)
		return;

	if (form->visible)
		fl_hide_form(form);

	fl_free_form(form);
}
