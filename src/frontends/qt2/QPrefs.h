// -*- C++ -*-
/**
 * \file QPrefs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QPREFS_H
#define QPREFS_H

#include "QDialogView.h"
#include <vector>

class Formats;

namespace lyx {
namespace frontend {

class QPrefsDialog;
class Controllers;

class QPrefs
	: public QController<ControlPrefs, QView<QPrefsDialog> >
{
public:

	friend class QPrefsDialog;

	QPrefs(Dialog &);

private:
	/// Apply changes
	virtual void apply();

	/// update (do we need this?)
	virtual void update_contents();

	/// build the dialog
	virtual void build_dialog();

	Converters & converters();
	Formats & formats();

	/// languages
	std::vector<std::string> lang_;
};

} // namespace frontend
} // namespace lyx

#endif // QPREFS_H
