/**
 * \file QRef.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef QREF_H
#define QREF_H

#include <config.h>
#include <vector>
#include "LString.h"

#include "Qt2Base.h"

class ControlRef;
class QRefDialog;


class QRef :
	public Qt2CB<ControlRef, Qt2DB<QRefDialog> >
{
	friend class QRefDialog;

public:
	QRef();

private:
	/// apply changes
	virtual void apply();
	/// build dialog
	virtual void build_dialog();
	/// update dialog
	virtual void update_contents();

	/// is name allowed for this ?
	bool nameAllowed();

	/// is type allowed for this ?
	bool typeAllowed();

	/// go to current reference
	void gotoRef();

	/// set go back button
	void setGoBack();

	/// set goto ref button
	void setGotoRef();

	/// re-enter references
	void redoRefs();

	/// update references
	void updateRefs();

	/// sort or not persistent state
	bool sort_;

	/// at a reference ?
	bool at_ref_;

	/// the references
	std::vector<string> refs_;
};

#endif // QREF_H
