// -*- C++ -*-
/**
 * \file GuiToc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 * \author Angus Leeming
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUITOC_H
#define GUITOC_H

#include "DockView.h"

namespace lyx {
namespace frontend {

class TocModel;
class TocWidget;

class GuiToc : public DockView
{
	Q_OBJECT

public:
	///
	GuiToc(
		GuiView & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area = Qt::LeftDockWidgetArea, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = {});

	///
	bool initialiseParams(std::string const & data);
	void updateView();
	void enableView(bool enable);
	bool wantInitialFocus() const { return false; }
	void closeEvent(QCloseEvent * event);
	bool isClosing() const { return is_closing_; }

public:
	/// clean-up on hide.
	void clearParams() {}
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }
	///
	void doDispatch(Cursor & cur, FuncRequest const & fr);
	///
	bool getStatus(Cursor & cur, FuncRequest const & fr, FuncStatus & fs) const;

private:
	///
	TocWidget * widget_;
	///
	bool is_closing_;
};

} // namespace frontend
} // namespace lyx

#endif // GUITOC_H
