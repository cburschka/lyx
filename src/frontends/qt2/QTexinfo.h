// -*- C++ -*-
/**
 * \file QTexinfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QTEXINFO_H
#define QTEXINFO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

#include "ControlTexinfo.h"

#include <vector>

class QTexinfoDialog;

///
class QTexinfo
	: public Qt2CB<ControlTexinfo, Qt2DB<QTexinfoDialog> >
{
public:
	///
	friend class QTexinfoDialog;
	///
	QTexinfo();
private:
	/// Apply changes
	virtual void apply() {}
	/// update (do we need this?)
	virtual void update_contents() {}
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
	///
	std::vector<string> cls_;
	///
	std::vector<string> sty_;
	///
	std::vector<string> bst_;
};

#endif // QTEXINFO_H
