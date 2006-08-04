// -*- C++ -*-
/**
 * \file QViewSource.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QVIEWSOURCE_H
#define QVIEWSOURCE_H

#include "QDialogView.h"

namespace lyx {
namespace frontend {

class ControlViewSource;
class QViewSourceDialog;

///
class QViewSource
	: public QController<ControlViewSource, QView<QViewSourceDialog> >
{
public:
	///
	friend class QViewSourceDialog;
	///
	QViewSource(Dialog &);
private:
	/// Apply changes
	virtual void apply() {}
	///
	void update_source();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // QVIEWSOURCE_H
