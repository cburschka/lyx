// -*- C++ -*-
/**
 * \file QVSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Angus Leeming
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QVSPACE_H
#define QVSPACE_H

#include "QDialogView.h"

#include <vector>

class ControlVSpace;
class QVSpaceDialog;

/** This class provides an Qt implementation of the VSpace dialog.
 */
class QVSpace
	: public QController<ControlVSpace, QView<QVSpaceDialog> >
{
public:
	///
	friend class QVSpaceDialog;
	///
	QVSpace(Dialog &);
private:
	/// Build the dialog
	virtual void build_dialog();
	/// Apply from dialog
	virtual void apply();
	/// Update the dialog
	virtual void update_contents();

	std::vector<std::string> units_;
};

#endif //QVSPACE_H
