// -*- C++ -*-
/**
 * \file QShowFile.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSHOWFILE_H
#define QSHOWFILE_H

#include "QDialogView.h"

namespace lyx {
namespace frontend {

class ControlShowFile;
class QShowFileDialog;


class QShowFile
	: public QController<ControlShowFile, QView<QShowFileDialog> >
{
public:
	friend class QShowFileDialog;

	QShowFile(Dialog &);
private:
	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // QSHOWFILE_H
