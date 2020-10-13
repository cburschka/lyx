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
	bool initialiseParams(std::string const & data) override;
	void updateView() override;
	void enableView(bool enable) override;
	bool wantInitialFocus() const override { return false; }
	void closeEvent(QCloseEvent * event) override;
	bool isClosing() const override { return is_closing_; }

public:
	/// clean-up on hide.
	void clearParams() override {}
	///
	void dispatchParams() override;
	///
	bool isBufferDependent() const override { return true; }
	///
	void doDispatch(Cursor & cur, FuncRequest const & fr, DispatchResult & dr);
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
