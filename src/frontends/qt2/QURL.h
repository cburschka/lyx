/**
 * \file QURL.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef QURL_H
#define QURL_H

#include "Qt2Base.h"

class ControlUrl;
class QURLDialog;
class Dialogs;

class QURL :
	public Qt2CB<ControlUrl, Qt2DB<QURLDialog> >
{
	friend class QURLDialog;
public:
	QURL(ControlUrl & c, Dialogs &);

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
