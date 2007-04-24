// -*- C++ -*-
/**
 * \file QRef.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QREF_H
#define QREF_H

#include "QDialogView.h"
#include "QRefDialog.h"

#include <vector>

namespace lyx {
namespace frontend {

class ControlRef;

// full qualification because qt4 has also a ControlRef type
class QRef
	: public QController<lyx::frontend::ControlRef, QView<QRefDialog> >
{
public:
	friend class QRefDialog;

	QRef(Dialog &);

protected:
	virtual bool isValid();

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

	/// went to a reference ?
	bool at_ref_;
	
	/// the last reference entered or examined
	QString last_reference_;

	/// store the buffer settings
	int restored_buffer_;

	/// the references
	std::vector<lyx::docstring> refs_;
};

} // namespace frontend
} // namespace lyx

#endif // QREF_H
