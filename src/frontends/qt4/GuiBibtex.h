// -*- C++ -*-
/**
 * \file GuiBibtex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIBIBTEX_H
#define GUIBIBTEX_H

#include "GuiDialogView.h"
#include "ButtonController.h"
#include "ControlBibtex.h"
#include "ui_BibtexUi.h"
#include "ui_BibtexAddUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class GuiBibtex;

template<class UI>
class UiDialog : public QDialog, public UI
{
public:
	UiDialog(QWidget * parent=0, bool modal=false, Qt::WFlags f=0)
		: QDialog(parent, f)
	{
		UI::setupUi(this);
		QDialog::setModal(modal);
	}
};


class GuiBibtexDialog : public QDialog, public Ui::BibtexUi {
	Q_OBJECT

public:
	GuiBibtexDialog(GuiBibtex * form);

	UiDialog<Ui::BibtexAddUi> * add_;

protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void browsePressed();
	virtual void browseBibPressed();
	virtual void addPressed();
	virtual void addDatabase();
	virtual void deletePressed();
	virtual void databaseChanged();
	virtual void availableChanged();
	void bibEDChanged();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	GuiBibtex * form_;
	ButtonController add_bc_;
};


class GuiBibtex : public GuiView<GuiBibtexDialog>
{
public:
	friend class GuiBibtexDialog;

	GuiBibtex(Dialog &);
	/// parent controller
	ControlBibtex & controller()
	{ return static_cast<ControlBibtex &>(this->getController()); }
	/// parent controller
	ControlBibtex const & controller() const
	{ return static_cast<ControlBibtex const &>(this->getController()); }
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // GUIBIBTEX_H
