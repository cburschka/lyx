/**
 * \file FormExternal.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon
 */

#include <config.h>
#include <utility>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "debug.h"
#include "gettext.h"
#include "support/LAssert.h"
#include "lyx_gui_misc.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "buffer.h"
#include "FormExternal.h"
#include "form_external.h"
#include "frontends/FileDialog.h"
#include "LString.h"
#include "support/filetools.h"

using std::pair;
using std::make_pair;
using std::endl;

FormExternal::FormExternal(LyXView * lv, Dialogs * d)
	: FormBaseBD(lv, d, _("Edit external file")),
	inset_(0), ih_(0)
{
	d->showExternal.connect(slot(this, &FormExternal::showInset));
}


extern "C" void ExternalTemplateCB(FL_OBJECT * ob, long data)
{
	FormExternal::templateCB(ob, data);
}


extern "C" void ExternalBrowseCB(FL_OBJECT * ob, long data)
{
	FormExternal::browseCB(ob, data);
}


extern "C" void ExternalEditCB(FL_OBJECT * ob, long data)
{
	FormExternal::editCB(ob, data);
}


extern "C" void ExternalViewCB(FL_OBJECT * ob, long data)
{
	FormExternal::viewCB(ob, data);
}


extern "C" void ExternalUpdateCB(FL_OBJECT * ob, long data)
{
	FormExternal::updateCB(ob, data);
}


FL_FORM * FormExternal::form() const
{
	if (dialog_.get())
		return dialog_->form;
	return 0;
}


void FormExternal::connect()
{
	u_ = d_->updateBufferDependent.
		 connect(slot(this, &FormExternal::updateSlot));
	h_ = d_->hideBufferDependent.
		 connect(slot(this, &FormExternal::hide));
	FormBaseDeprecated::connect();
}


void FormExternal::disconnect()
{
	inset_ = 0;
	ih_.disconnect();
	FormBaseBD::disconnect();
}


void FormExternal::updateSlot(bool switched)
{
	if (switched)
		hide();
	else
		update();
}


void FormExternal::showInset(InsetExternal * inset)
{
	Assert(inset);

	// If connected to another inset, disconnect from it.
	if (inset_)
		ih_.disconnect();

	inset_ = inset;
	params_ = inset_->params();

	ih_ = inset->hideDialog.connect(slot(this, &FormExternal::hide));
	show();
}


void FormExternal::build()
{
	dialog_.reset(build_external());

	fl_addto_choice(dialog_->choice_template,
			getTemplatesComboString().c_str());

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	bc_.setOK(dialog_->button_ok);
	bc_.setCancel(dialog_->button_cancel);
	bc_.refresh();

	bc_.addReadOnly(dialog_->input_filename);
	bc_.addReadOnly(dialog_->button_filenamebrowse);
	bc_.addReadOnly(dialog_->input_parameters);
}


string const FormExternal::getTemplatesComboString() const
{
	string result;
	bool first = true;
	ExternalTemplateManager::Templates::const_iterator i1, i2;
	i1 = ExternalTemplateManager::get().getTemplates().begin();
	i2 = ExternalTemplateManager::get().getTemplates().end();
	for (; i1 != i2; ++i1) {
		if (!first)
			result += "|";
		else
			first = false;

		result += (*i1).second.lyxName;
	}
	return result;
}


int FormExternal::getTemplateComboNumber(string const & name) const
{
	int i = 1;
	ExternalTemplateManager::Templates::const_iterator i1, i2;
	i1 = ExternalTemplateManager::get().getTemplates().begin();
	i2 = ExternalTemplateManager::get().getTemplates().end();
	for (; i1 != i2; ++i1) {
		if (i1->second.lyxName == name)
			return i;
		++i;
	}
	// we can get here if a LyX document has a template not installed
	// on this machine.
	return 0;
}


ExternalTemplate FormExternal::getTemplate(int i) const
{
	ExternalTemplateManager::Templates::const_iterator i1;
	i1 = ExternalTemplateManager::get().getTemplates().begin();
	for (int n = 1; n < i; ++n)
		++i1;

	return (*i1).second;
}


void FormExternal::update()
{
	fl_set_input(dialog_->input_filename, params_.filename.c_str());
	fl_set_input(dialog_->input_parameters, params_.parameters.c_str());

	fl_set_choice(dialog_->choice_template, getTemplateComboNumber(params_.templ.lyxName));

	updateComboChange();

	bc_.valid();
}


void FormExternal::updateComboChange()
{
	// Update the help text
	fl_clear_browser(dialog_->browser_helptext);
	fl_addto_browser(dialog_->browser_helptext, params_.templ.helpText.c_str());
	fl_set_browser_topline(dialog_->browser_helptext, 0);

	if (params_.templ.automaticProduction) {
		fl_deactivate_object(dialog_->button_update);
		fl_set_object_lcol(dialog_->button_update, FL_INACTIVE);
	} else {
		fl_activate_object(dialog_->button_update);
		fl_set_object_lcol(dialog_->button_update, FL_BLACK);
	}
}


bool FormExternal::input(FL_OBJECT *, long)
{
	// FIXME: anything to do here ?
	return true;
}


void FormExternal::apply()
{
	Assert(inset_);

	if (lv_->buffer()->isReadonly())
		return;

	params_.filename = fl_get_input(dialog_->input_filename);
	params_.parameters = fl_get_input(dialog_->input_parameters);
	params_.templ = getTemplate(fl_get_choice(dialog_->choice_template));

	inset_->setFromParams(params_);
	lv_->view()->updateInset(inset_, true);
}


void FormExternal::templateCB(FL_OBJECT * ob, long)
{
	FormExternal * form = static_cast<FormExternal*>(ob->form->u_vdata);

	// set to the chosen template
	form->params_.templ = form->getTemplate(fl_get_choice(form->dialog_->choice_template));

	form->updateComboChange();
}


void FormExternal::browseCB(FL_OBJECT * ob, long)
{
	FormExternal * form = static_cast<FormExternal*>(ob->form->u_vdata);

	static string current_path;
	static int once = 0;
	
	string p = fl_get_input(form->dialog_->input_filename);
	string buf = MakeAbsPath(form->lv_->buffer()->fileName());
	string buf2 = OnlyPath(buf);

	if (!p.empty()) {
		buf = MakeAbsPath(p, buf2);
		buf = OnlyPath(buf);
	} else {
		buf = OnlyPath(form->lv_->buffer()->fileName());
	}
    
	FileDialog fileDlg(form->lv_, _("Select external file"),
		LFUN_SELECT_FILE_SYNC,
		make_pair(string(_("Document")), string(buf)));
	
	/// Determine the template file extension
	ExternalTemplate const & et = form->params_.templ;

	string regexp = et.fileRegExp;
	if (regexp.empty())
		regexp = "*";

	// FIXME: a temporary hack until the FileDialog interface is updated
	regexp += "|";

	while (1) {
		string const path = (once) ? current_path : buf;
		FileDialog::Result result = fileDlg.Select(path, regexp, fl_get_input(form->dialog_->input_filename));

		if (result.second.empty())
			return;

		string p = result.second;

		buf = MakeRelPath(p, buf2);
		current_path = OnlyPath(p);
		once = 1;
		
		if (contains(p, "#") || contains(p, "~") || contains(p, "$")
		    || contains(p, "%")) {
			WriteAlert(_("Filename can't contain any "
				     "of these characters:"),
				   // xgettext:no-c-format
				   _("'#', '~', '$' or '%'."));
		} else
			break;
	}

	fl_set_input(form->dialog_->input_filename, buf.c_str());
}


void FormExternal::editCB(FL_OBJECT * ob, long)
{
	FormExternal * form = static_cast<FormExternal*>(ob->form->u_vdata);

	form->apply();
	form->inset_->editExternal();
}


void FormExternal::viewCB(FL_OBJECT * ob, long)
{
	FormExternal * form = static_cast<FormExternal*>(ob->form->u_vdata);
	
	form->apply();
	form->inset_->viewExternal();
}


void FormExternal::updateCB(FL_OBJECT * ob, long)
{
	FormExternal * form = static_cast<FormExternal*>(ob->form->u_vdata);
	
	form->apply();
	form->inset_->updateExternal();
}
