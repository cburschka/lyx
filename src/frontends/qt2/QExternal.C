/**
 * \file QExternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "ControlExternal.h"
#include "qt_helpers.h"

#include "insets/ExternalTemplate.h"
#include "insets/insetexternal.h"

#include "support/lstrings.h"
#include "support/tostr.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qtextview.h>

#include "QExternalDialog.h"
#include "QExternal.h"
#include "Qt2BC.h"

using lyx::support::strToInt;
using lyx::support::trim;

using std::string;


typedef QController<ControlExternal, QView<QExternalDialog> > base_class;

QExternal::QExternal(Dialog & parent)
	: base_class(parent, _("LyX: External Material"))
{
}


void QExternal::build_dialog()
{
	dialog_.reset(new QExternalDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->externalCO);
	bcview().addReadOnly(dialog_->fileED);
	bcview().addReadOnly(dialog_->browsePB);
	bcview().addReadOnly(dialog_->extraED);

	std::vector<string> templates(controller().getTemplates());

	for (std::vector<string>::const_iterator cit = templates.begin();
		cit != templates.end(); ++cit) {
		dialog_->externalCO->insertItem(toqstr(*cit), -1);
	}
}


void QExternal::update_contents()
{
	InsetExternalParams const & params = controller().params();

	string const name =
		params.filename.outputFilename(kernel().bufferFilepath());
	dialog_->fileED->setText(toqstr(name));

	dialog_->externalCO->setCurrentItem(controller()
					    .getTemplateNumber(params.templatename()));
	updateTemplate();

	int item = 0;
	switch (params.display) {
		case lyx::external::DefaultDisplay: item = 0; break;
		case lyx::external::MonochromeDisplay: item = 1; break;
		case lyx::external::GrayscaleDisplay: item = 2; break;
		case lyx::external::ColorDisplay: item = 3; break;
		case lyx::external::PreviewDisplay: item = 4; break;
		case lyx::external::NoDisplay: item = 0; break;
	}
	dialog_->showCB->setCurrentItem(item);
	dialog_->showCB->setEnabled(params.display != lyx::external::NoDisplay &&
				    !readOnly());
	dialog_->displayCB->setChecked(params.display != lyx::external::NoDisplay);
	dialog_->displayscale->setEnabled(params.display != lyx::external::NoDisplay &&
					  !readOnly());
	dialog_->displayscale->setText(toqstr(tostr(params.lyxscale)));

	isValid();
}


void QExternal::updateTemplate()
{
	namespace external = lyx::external;

	dialog_->externalTV->setText(toqstr(helpText()));

	// Ascertain whether the template has any formats supporting
	// the 'Extra' option
	QLineEdit * const input = dialog_->extraED;
	QComboBox * const combo = dialog_->extraFormatCB;

	extra_.clear();
	input->clear();
	combo->clear();

	external::Template templ =
		controller().getTemplate(dialog_->externalCO->currentItem());
	external::Template::Formats::const_iterator it  = templ.formats.begin();
	external::Template::Formats::const_iterator end = templ.formats.end();
	for (; it != end; ++it) {
		if (it->second.option_transformers.find(external::Extra) ==
		    it->second.option_transformers.end())
			continue;
		string const format = it->first;
		string const opt = controller().params().extradata.get(format);
		combo->insertItem(toqstr(format));
		extra_[format] = toqstr(opt);
	}

	bool const enabled = combo->count()  > 0;

	input->setEnabled(enabled && !kernel().isBufferReadonly());
	combo->setEnabled(enabled);

	if (enabled) {
		combo->setCurrentItem(0);
		input->setText(extra_[fromqstr(combo->currentText())]);
	}
}


string const QExternal::helpText() const
{
	lyx::external::Template templ =
		controller().getTemplate(dialog_->externalCO->currentItem());
	return templ.helpText;
}


void QExternal::apply()
{
	InsetExternalParams params = controller().params();

	params.filename.set(fromqstr(dialog_->fileED->text()),
			    kernel().bufferFilepath());

	params.settemplate(
		controller().getTemplate(dialog_->externalCO->currentItem()).lyxName);

	switch (dialog_->showCB->currentItem()) {
		case 0: params.display = lyx::external::DefaultDisplay; break;
		case 1: params.display = lyx::external::MonochromeDisplay; break;
		case 2: params.display = lyx::external::GrayscaleDisplay; break;
		case 3: params.display = lyx::external::ColorDisplay; break;
		case 4: params.display = lyx::external::PreviewDisplay; break;
		default:;
	}

	if (!dialog_->displayCB->isChecked())
		params.display = lyx::external::NoDisplay;

	params.lyxscale = strToInt(fromqstr(dialog_->displayscale->text()));

	std::map<string, QString>::const_iterator it  = extra_.begin();
	std::map<string, QString>::const_iterator end = extra_.end();
	for (; it != end; ++it)
		params.extradata.set(it->first, trim(fromqstr(it->second)));

	controller().setParams(params);
}
