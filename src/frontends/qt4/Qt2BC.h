// -*- C++ -*-
/**
 * \file Qt2BC.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming
 * \author Baruch Even
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QT2BC_H
#define QT2BC_H

#include "BCView.h"
#include "gettext.h"

class QWidget;
class QPushButton;

namespace lyx {
namespace frontend {

/** General purpose button controller for up to four buttons.
    Controls the activation of the OK, Apply and Cancel buttons.
    Actually supports 4 buttons in all and it's up to the user to decide on
    the activation policy and which buttons correspond to which output of the
    state machine.
*/

class Qt2BC : public BCView
{
public:
	///
	Qt2BC(ButtonController const & parent);

	//@{
	/** Store pointers to these widgets.
	 */
	void setOK(QPushButton * obj) { okay_ = obj; }
	void setApply(QPushButton * obj) { apply_ = obj; }
	void setCancel(QPushButton * obj) { cancel_ = obj; }
	void setRestore(QPushButton * obj) { restore_ = obj; }
	//@}

	/** Add a pointer to the list of widgets whose activation
	 *  state is dependent upon the read-only status of the
	 *  underlying buffer.
	 */
	void addReadOnly(QWidget * obj) { read_only_.push_back(obj); }

	/// Refresh the status of the Ok, Apply, Restore, Cancel buttons.
	virtual void refresh() const;
	/// Refresh the status of any widgets in the read_only list
	virtual void refreshReadOnly() const;

private:
	/// Updates the widget sensitivity (enabled/disabled)
	void setWidgetEnabled(QWidget *, bool enabled) const;

	QPushButton * okay_;
	QPushButton * apply_;
	QPushButton * cancel_;
	QPushButton * restore_;

	typedef std::list<QWidget *> Widgets;
	Widgets read_only_;
};

} // namespace frontend
} // namespace lyx

#endif // QT2BC_H
