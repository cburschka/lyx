// -*- C++ -*-
/**
 * \file QTexinfo.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#ifndef QTEXINFO_H
#define QTEXINFO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

#include "ControlTexinfo.h"

class QTexinfoDialog;

///
class QTexinfo
	: public Qt2CB<ControlTexinfo, Qt2DB<QTexinfoDialog> >
{
public:
	///
	friend class QTexinfoDialog;
	///
	QTexinfo(ControlTexinfo &);
private:
	/// Apply changes
	virtual void apply() { };
	/// update (do we need this?)
	virtual void update_contents() {};
	/// build the dialog
	virtual void build_dialog();
	///
	void updateStyles(ControlTexinfo::texFileSuffix);
	///
	void updateStyles();
	///
	bool warningPosted;
	///
	ControlTexinfo::texFileSuffix activeStyle;

};

#endif // QTEXINFO_H
