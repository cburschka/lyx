/**
 * \file QInclude.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/os.h"

#include "QInclude.h"

#include "checkedwidgets.h"
#include "Qt2BC.h"
#include "qt_helpers.h"
#include "validators.h"

#include "lyxrc.h"

#include "controllers/ControlInclude.h"

#include <QPushButton>
#include <QCheckBox>
#include <QCloseEvent>
#include <QLineEdit>

using std::string;

using lyx::support::os::internal_path;


namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// QIncludeDialog
//
/////////////////////////////////////////////////////////////////////

QIncludeDialog::QIncludeDialog(QInclude * form)
	: form_(form)
{
	setupUi(this);
	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));

	connect(visiblespaceCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(filenameED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(loadPB, SIGNAL(clicked()), this, SLOT(loadClicked()));
	connect(browsePB, SIGNAL(clicked()), this, SLOT(browseClicked()));
	connect(typeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(typeCO, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
	connect(previewCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));

	filenameED->setValidator(new PathValidator(true, filenameED));
	setFocusProxy(filenameED);
}


void QIncludeDialog::show()
{
	QDialog::show();
}


void QIncludeDialog::change_adaptor()
{
	form_->changed();
}


void QIncludeDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QIncludeDialog::typeChanged(int v)
{
	switch (v) {
		//case Include
		case 0:
			visiblespaceCB->setEnabled(false);
			visiblespaceCB->setChecked(false);
			previewCB->setEnabled(false);
			previewCB->setChecked(false);
			break;
		//case Input
		case 1:
			visiblespaceCB->setEnabled(false);
			visiblespaceCB->setChecked(false);
			previewCB->setEnabled(true);
			break;
		//case Verbatim
		default:
			visiblespaceCB->setEnabled(true);
			previewCB->setEnabled(false);
			previewCB->setChecked(false);
			break;
	}
}


void QIncludeDialog::loadClicked()
{
	form_->load();
}


void QIncludeDialog::browseClicked()
{
	form_->browse();
}


/////////////////////////////////////////////////////////////////////
//
// QInclude
//
/////////////////////////////////////////////////////////////////////


typedef QController<ControlInclude, QView<QIncludeDialog> > IncludeBase;


QInclude::QInclude(Dialog & parent)
	: IncludeBase(parent, _("Child Document"))
{}


void QInclude::build_dialog()
{
	dialog_.reset(new QIncludeDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->filenameED);
	bcview().addReadOnly(dialog_->browsePB);
	bcview().addReadOnly(dialog_->visiblespaceCB);
	bcview().addReadOnly(dialog_->typeCO);

	addCheckedLineEdit(bcview(), dialog_->filenameED, dialog_->filenameLA);
}


void QInclude::update_contents()
{
	PathValidator * path_validator = getPathValidator(dialog_->filenameED);
	if (path_validator)
		path_validator->setChecker(kernel().docType(), lyxrc);

	InsetCommandParams const & params = controller().params();

	dialog_->filenameED->setText(toqstr(params["filename"]));

	dialog_->visiblespaceCB->setChecked(false);
	dialog_->visiblespaceCB->setEnabled(false);
	dialog_->previewCB->setChecked(false);
	dialog_->previewCB->setEnabled(false);

	string cmdname = controller().params().getCmdName();
	if (cmdname != "include" &&
	    cmdname != "verbatiminput" &&
	    cmdname != "verbatiminput*")
		cmdname = "input";

	if (cmdname == "include") {
		dialog_->typeCO->setCurrentIndex(0);

	} else if (cmdname == "input") {
		dialog_->typeCO->setCurrentIndex(1);
		dialog_->previewCB->setEnabled(true);
		dialog_->previewCB->setChecked(params.preview());

	} else if (cmdname == "verbatiminput*") {
		dialog_->typeCO->setCurrentIndex(2);
		dialog_->visiblespaceCB->setEnabled(true);
		dialog_->visiblespaceCB->setChecked(true);

	} else if (cmdname == "verbatiminput") {
		dialog_->typeCO->setCurrentIndex(2);
		dialog_->visiblespaceCB->setEnabled(true);
	}
}


void QInclude::apply()
{
	InsetCommandParams params = controller().params();

	params["filename"] = from_utf8(internal_path(fromqstr(dialog_->filenameED->text())));
	params.preview(dialog_->previewCB->isChecked());

	int const item = dialog_->typeCO->currentIndex();
	if (item == 0)
		params.setCmdName("include");
	else if (item == 1)
		params.setCmdName("input");
	else {
		if (dialog_->visiblespaceCB->isChecked())
			params.setCmdName("verbatiminput*");
		else
			params.setCmdName("verbatiminput");
	}
	controller().setParams(params);
}


void QInclude::browse()
{
	ControlInclude::Type type;

	int const item = dialog_->typeCO->currentIndex();
	if (item == 0)
		type = ControlInclude::INCLUDE;
	else if (item == 1)
		type = ControlInclude::INPUT;
	else
		type = ControlInclude::VERBATIM;

	docstring const & name =
		controller().browse(qstring_to_ucs4(dialog_->filenameED->text()), type);
	if (!name.empty())
		dialog_->filenameED->setText(toqstr(name));
}


void QInclude::load()
{
	if (isValid()) {
		string const file = fromqstr(dialog_->filenameED->text());
		slotOK();
		controller().load(file);
	}
}


bool QInclude::isValid()
{
	return !dialog_->filenameED->text().isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "QInclude_moc.cpp"
