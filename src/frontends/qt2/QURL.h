// -*- C++ -*-
/**
 * \file QURL.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QURL_H
#define QURL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlUrl;
class QURLDialog;


class QURL :
	public Qt2CB<ControlUrl, Qt2DB<QURLDialog> >
{
	friend class QURLDialog;
public:
	QURL();

protected:
	virtual bool isValid();

private:
	/// apply dialog
	virtual void apply();
	/// build dialog
	virtual void build_dialog();
	/// update dialog
	virtual void update_contents();
};

#endif // QURL_H
