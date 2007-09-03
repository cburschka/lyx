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

#include "GuiDialogView.h"
#include "ui_GraphicsUi.h"
#include "ControlGraphics.h"

#include <QDialog>

#include <vector>

class QString;

namespace lyx {
namespace frontend {

class GuiGraphics;

class GuiGraphicsDialog : public QDialog, public Ui::GraphicsUi
{
	Q_OBJECT
public:
	GuiGraphicsDialog(GuiGraphics * form);
	virtual void setAutoText();
	virtual void show();
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void change_bb();
	virtual void on_browsePB_clicked();
	virtual void on_getPB_clicked();
	virtual void on_editPB_clicked();
	virtual void on_filename_textChanged(const QString &);
	virtual void on_scaleCB_toggled(bool);
	virtual void on_WidthCB_toggled(bool);
	virtual void on_HeightCB_toggled(bool);
	virtual void on_angle_textChanged(const QString &);
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiGraphics * form_;
};


class GuiGraphics : public GuiView<GuiGraphicsDialog>
{
public:
	///
	friend class GuiGraphicsDialog;
	///
	GuiGraphics(GuiDialog &);
	/// parent controller
	ControlGraphics & controller()
	{ return static_cast<ControlGraphics &>(this->getController()); }
	/// parent controller
	ControlGraphics const & controller() const
	{ return static_cast<ControlGraphics const &>(this->getController()); }
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
	/// get bounding box from file
	void getBB();

	/// Store the LaTeX names for the rotation origins.
	std::vector<std::string> origin_ltx;

};

} // namespace frontend
} // namespace lyx

#endif // GUIGRAPHICS_H
