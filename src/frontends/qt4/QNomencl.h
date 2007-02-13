// -*- C++ -*-
/**
 * \file QNomencl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QNOMENCL_H
#define QNOMENCL_H

#include "QDialogView.h"
#include "QNomenclDialog.h"

namespace lyx {
namespace frontend {

class ControlCommand;

class QNomencl :
	public QController<ControlCommand, QView<QNomenclDialog> >
{
public:
	friend class QNomenclDialog;

	QNomencl(Dialog &, docstring const & title);
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

#endif // QNOMENCL_H
