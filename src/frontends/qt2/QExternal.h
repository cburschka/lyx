// -*- C++ -*-
/**
 * \file QExternal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QEXTERNAL_H
#define QEXTERNAL_H


#include "QDialogView.h"
#include <map>


class ControlExternal;
class QExternalDialog;


class QExternal
	: public QController<ControlExternal, QView<QExternalDialog> >
{
public:
	friend class QExternalDialog;

	QExternal(Dialog &);

	typedef std::map<std::string, QString> MapType;

private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	/// Helper function called when the template is changed.
	void updateTemplate();
	/// get bounding box from file
	void getBB();

	MapType extra_;
};

#endif // QEXTERNAL_H
