// -*- C++ -*-
/**
 * \file InsetParamsWidget.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_PARAMS_WIDGET_H
#define INSET_PARAMS_WIDGET_H

#include "insets/InsetCode.h"
#include "insets/Inset.h"

#include "qt_helpers.h"

#include "FuncCode.h"

#include "support/strfwd.h"

#include <QWidget>

class QLineEdit;

namespace lyx {

class Inset;

namespace frontend {

/// CheckedWidget
// FIXME: Get rid of CheckedLineEdit in ButtonController and rename this one
// to it.
class CheckedWidget
{
public:
	CheckedWidget(QLineEdit * input, QWidget * label = 0);
	///	
	bool check() const;

private:
	// non-owned
	QLineEdit * input_;
	QWidget * label_;
};


typedef QList<CheckedWidget> CheckedWidgets;

class InsetParamsWidget : public QWidget
{
	Q_OBJECT

Q_SIGNALS:
	void changed();
public:
	InsetParamsWidget(QWidget * parent);
	/// This is a base class; destructor must exist and be virtual.
	virtual ~InsetParamsWidget() {}
	///
	virtual QString dialogTitle() const { return toqstr(insetName(insetCode())); }
	///
	virtual InsetCode insetCode() const = 0;
	///
	virtual FuncCode creationCode() const = 0;
	///
	virtual void paramsToDialog(Inset const *) = 0;
	///
	virtual docstring dialogToParams() const = 0;
	///
	virtual bool initialiseParams(std::string const & /*data*/)
		{ return false; }

	/// \return true if all CheckedWidgets are in a valid state.
	virtual bool checkWidgets(bool readonly = false) const;

protected:
	/// Add a widget to the list of all widgets whose validity should
	/// be checked explicitly when the buttons are refreshed.
	void addCheckedWidget(QLineEdit * input, QWidget * label = 0);
private:
	///
	CheckedWidgets checked_widgets_;
};

} // namespace frontend
} // namespace lyx

#endif // INSET_PARAMS_WIDGET_H
