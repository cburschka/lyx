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


#include "converter.h"
#include "format.h"

#include "Qt2Base.h"
#include "ControlPrefs.h"

#include <vector>

class QPrefsDialog;

class QPrefs
	: public Qt2CB<ControlPrefs, Qt2DB<QPrefsDialog> >
{
public:

	friend class QPrefsDialog;

	QPrefs();

private:
	/// Apply changes
	virtual void apply();

	/// update (do we need this?)
	virtual void update_contents();

	/// build the dialog
	virtual void build_dialog();

	/// languages
	std::vector<string> lang_;

	/// converters
	Converters converters_;

	/// formats
	Formats formats_;
};

#endif // QPREFS_H
