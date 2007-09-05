// -*- C++ -*-
/**
 * \file GuiGraphics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIGRAPHICS_H
#define GUIGRAPHICS_H

#include "GuiDialog.h"
#include "ControlGraphics.h"
#include "ui_GraphicsUi.h"

#include <vector>

class QString;

namespace lyx {
namespace frontend {

class GuiGraphicsDialog : public GuiDialog, public Ui::GraphicsUi
{
	Q_OBJECT
public:
	GuiGraphicsDialog(LyXView & lv);
	void setAutoText();
	void showView();
private Q_SLOTS:
	void change_adaptor();
	void change_bb();
	void on_browsePB_clicked();
	void on_getPB_clicked();
	void on_editPB_clicked();
	void on_filename_textChanged(const QString &);
	void on_scaleCB_toggled(bool);
	void on_WidthCB_toggled(bool);
	void on_HeightCB_toggled(bool);
	void on_angle_textChanged(const QString &);
private:
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlGraphics & controller() const;
	bool isValid();
	/// Apply changes
	void applyView();
	/// update
	void update_contents();
	/// get bounding box from file
	void getBB();

	/// Store the LaTeX names for the rotation origins.
	std::vector<std::string> origin_ltx;
};

} // namespace frontend
} // namespace lyx

#endif // GUIGRAPHICS_H
