// -*- C++ -*-
/**
 * \file FileDialog_private.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FILEDIALOG_PRIVATE_H
#define FILEDIALOG_PRIVATE_H

#include "frontends/FileDialog.h"

#include <QFileDialog>

class QToolButton;

namespace lyx {

namespace support { class FileFilterList; }

class LyXFileDialog : public QFileDialog
{
	Q_OBJECT
public:
	LyXFileDialog(lyx::docstring const & title,
		      lyx::docstring const & path,
		      lyx::support::FileFilterList const & filters,
		      FileDialog::Button const & b1,
		      FileDialog::Button const & b2);
public Q_SLOTS:
	void buttonClicked();
private:
	QToolButton * b1_;
	lyx::docstring b1_dir_;

	QToolButton * b2_;
	lyx::docstring b2_dir_;
};

} // namespace lyx


#endif // FILEDIALOG_PRIVATE_H
