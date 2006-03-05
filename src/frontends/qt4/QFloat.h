// -*- C++ -*-
/**
 * \file QFloat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QFLOAT_H
#define QFLOAT_H

#include "QDialogView.h"

namespace lyx {
namespace frontend {


class ControlFloat;
class QFloatDialog;

///
class QFloat : public QController<ControlFloat, QView<QFloatDialog> > {
public:
	///
	friend class QFloatDialog;
	///
	QFloat(Dialog &);
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

#endif // QFLOAT_H
