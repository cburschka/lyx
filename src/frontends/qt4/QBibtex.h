// -*- C++ -*-
/**
 * \file QBibtex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBIBTEX_CONTROLLER_H
#define QBIBTEX_CONTROLLER_H

#include "QDialogView.h"
#include "QBibtexDialog.h"

namespace lyx {
namespace frontend {

class ControlBibtex;

class QBibtex
	: public QController<ControlBibtex, QView<QBibtexDialog> >
{
public:
	friend class QBibtexDialog;

	QBibtex(Dialog &);
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // QBIBTEX_H
