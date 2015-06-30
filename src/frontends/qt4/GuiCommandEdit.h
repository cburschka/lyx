// -*- C++ -*-
/**
 * \file GuiCommandEdit.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICOMMANDEDIT_H
#define GUICOMMANDEDIT_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QEvent>

namespace lyx {
namespace frontend {

class GuiCommandEdit : public QLineEdit
{
	Q_OBJECT

public:
	GuiCommandEdit(QWidget * parent);

Q_SIGNALS:
	/// cancel
	void escapePressed();
	/// up history
	void upPressed();
	/// down history
	void downPressed();
	/// complete
	void tabPressed();
	/// leave and hide command buffer
	void hidePressed();

protected:
	///
	virtual bool event(QEvent * e);
	///
	virtual void keyPressEvent(QKeyEvent * e);
};

} // namespace frontend
} // namespace lyx

#endif // GUICOMMANDEDIT_H
