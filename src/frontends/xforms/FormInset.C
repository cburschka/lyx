/**
 * \file FormInset.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming <leeming@lyx.org>
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormInset.h"

#include "frontends/Dialogs.h"

#include <boost/bind.hpp>

FormInset::FormInset(LyXView & lv, Dialogs & d, string const & t)
	: FormBaseBD(lv, d, t)
{}


void FormInset::connect()
{
	u_ = d_.updateBufferDependent.
		connect(boost::bind(&FormInset::updateSlot, this, _1));
	h_ = d_.hideBufferDependent.
		connect(boost::bind(&FormInset::hide, this));
	FormBaseDeprecated::connect();
}


void FormInset::disconnect()
{
	ih_.disconnect();
	FormBaseBD::disconnect();
}


void FormInset::updateSlot(bool switched)
{
	if (switched)
		hide();
	else
		update();
}
