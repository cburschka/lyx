// -*- C++ -*-
/**
 * \file UrlView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QT4_URL_VIEW_H
#define QT4_URL_VIEW_H

#include "GuiDialogView.h"
#include "GuiURLDialog.h"

namespace lyx {
namespace frontend {

class ControlCommand;

class UrlView :
	public QController<ControlCommand, GuiView<GuiURLDialog> >
{
public:
	friend class QURLDialog;
	UrlView(Dialog &);
protected:
	virtual bool isValid();
private:
	/// apply dialog
	virtual void apply();
	/// build dialog
	virtual void build_dialog();
	/// update dialog
	virtual void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // QT4_URL_VIEW_H
