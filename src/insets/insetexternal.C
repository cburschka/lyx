/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION
#include <cstdio>

#include "insetexternal.h"
#include "ExternalTemplate.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB
#include "BufferView.h"
#include "buffer.h"
#include "filedlg.h"
#include "lyx_main.h"
#include "LaTeXFeatures.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/path.h"
#include "support/syscall.h"
#include "frontends/Dialogs.h" // redrawGUI

#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif

using std::endl;


InsetExternal::InsetExternal() 
	: form_external(0)
{
	tempname = lyx::tempName(); //TmpFileName();
	r_ = Dialogs::redrawGUI.connect(slot(this, &InsetExternal::redraw));
}


InsetExternal::~InsetExternal()
{
	lyx::unlink(tempname);
	r_.disconnect();
}


void InsetExternal::redraw()
{
	if (form_external && form_external->form_external->visible)
		fl_redraw_form(form_external->form_external);
}


extern "C"
void ExternalTemplateCB(FL_OBJECT * ob, long data)
{
	InsetExternal::templateCB(ob, data);
}


extern "C"
void ExternalBrowseCB(FL_OBJECT * ob, long data)
{
	InsetExternal::browseCB(ob, data);
}


extern "C"
void ExternalEditCB(FL_OBJECT * ob, long data)
{
	InsetExternal::editCB(ob, data);
}


extern "C"
void ExternalViewCB(FL_OBJECT * ob, long data)
{
	InsetExternal::viewCB(ob, data);
}


extern "C"
void ExternalUpdateCB(FL_OBJECT * ob, long data)
{
	InsetExternal::updateCB(ob, data);
}


extern "C"
void ExternalOKCB(FL_OBJECT * ob, long data)
{
	InsetExternal::okCB(ob, data);
}


extern "C"
void ExternalCancelCB(FL_OBJECT * ob, long data)
{
	InsetExternal::cancelCB(ob, data);
}


void InsetExternal::templateCB(FL_OBJECT * ob, long)
{
	Holder * holder = static_cast<Holder*>(ob->form->u_vdata);
	InsetExternal * inset = holder->inset;
	ExternalTemplate et = inset->getTemplate(inset->getCurrentTemplate());
	// Update the help text
	fl_clear_browser(inset->form_external->helptext);
	fl_addto_browser(inset->form_external->helptext, et.helpText.c_str());
	fl_set_browser_topline(inset->form_external->helptext, 0);
}


void InsetExternal::browseCB(FL_OBJECT * ob, long)
{
	Holder * holder = static_cast<Holder*>(ob->form->u_vdata);
	InsetExternal * inset = holder->inset;

	static string current_path;
	static int once = 0;
	LyXFileDlg fileDlg;
	
	string p = inset->filename;
	string buf = MakeAbsPath(holder->view->buffer()->fileName());
	string buf2 = OnlyPath(buf);
	if (!p.empty()) {
		buf = MakeAbsPath(p, buf2);
		buf = OnlyPath(buf);
	} else {
		buf = OnlyPath(holder->view->buffer()->fileName());
	}
       
	fileDlg.SetButton(0, _("Document"), buf); 

	/// Determine the template file extension
	ExternalTemplate et = inset->getTemplate(inset->getCurrentTemplate());
	string regexp = et.fileRegExp;
	if (regexp.empty()) {
		regexp = "*";
	}

	bool error = false;
	do {
		if (once) {
			p = fileDlg.Select(_("External inset file"),
					   current_path,
					   regexp, string());
		} else {
			p = fileDlg.Select(_("External inset file"), buf,
					   regexp, string());
		}

		if (p.empty()) return;

		buf = MakeRelPath(p, buf2);
		current_path = OnlyPath(p);
		once = 1;
		
		if (contains(p, "#") || contains(p, "~") || contains(p, "$")
		    || contains(p, "%")) {
			WriteAlert(_("Filename can't contain any "
				     "of these characters:"),
				   // xgettext:no-c-format
				   _("'#', '~', '$' or '%'.")); 
			error = true;
		}
	} while (error);

	if (inset->form_external) 
		fl_set_input(inset->form_external->filename, buf.c_str());
	
}


void InsetExternal::editCB(FL_OBJECT * ob, long)
{
	Holder * holder = static_cast<Holder*>(ob->form->u_vdata);
	InsetExternal * inset = holder->inset;
	inset->doApply(holder->view);
	inset->doEdit(holder->view);
}


void InsetExternal::viewCB(FL_OBJECT * ob, long)
{
	Holder * holder = static_cast<Holder*>(ob->form->u_vdata);
	InsetExternal * inset = holder->inset;
	inset->doApply(holder->view);
	inset->doView(holder->view);
}


void InsetExternal::updateCB(FL_OBJECT * ob, long)
{
	Holder * holder = static_cast<Holder*>(ob->form->u_vdata);
	InsetExternal * inset = holder->inset;
	inset->doApply(holder->view);
	inset->doUpdate(holder->view);
}


void InsetExternal::okCB(FL_OBJECT * ob, long data)
{
	Holder * holder = static_cast<Holder*>(ob->form->u_vdata);
	InsetExternal * inset = holder->inset;
	inset->doApply(holder->view);
	cancelCB(ob,data);
}


void InsetExternal::doApply(BufferView * bufview)
{
	bool update = false;
	if (templatename != getCurrentTemplate()) {
		templatename = getCurrentTemplate();
		update = true;
	}
	if (filename != fl_get_input(form_external->filename)) {
		filename = fl_get_input(form_external->filename);
		update = true;
	}
	if (parameters != fl_get_input(form_external->parameters)) {
		parameters = fl_get_input(form_external->parameters);
		update = true;
	}

	if (update) {
		// The text might have change,
		// so we should update the button look
		bufview->updateInset(this, true);
	}
}


void InsetExternal::cancelCB(FL_OBJECT * ob, long)
{
	Holder * holder = static_cast<Holder*>(ob->form->u_vdata);

	InsetExternal * inset = holder->inset;
	// BufferView * bv = holder->view;

	if (inset->form_external) {
		fl_hide_form(inset->form_external->form_external);
		fl_free_form(inset->form_external->form_external);
		inset->form_external = 0;
	}
}


string const InsetExternal::EditMessage() const
{
	ExternalTemplate const & et = getTemplate(templatename);
	return doSubstitution(0, et.guiName);
}


void InsetExternal::Edit(BufferView * bv,
			 int /*x*/, int /*y*/, unsigned int /*button*/)
{
	static int ow = -1, oh;

	if (bv->buffer()->isReadonly())
		WarnReadonly(bv->buffer()->fileName());

	if (!form_external) {
		form_external = create_form_form_external();
		holder.inset = this;
		//		form_external->ok->u_vdata = &holder;
		form_external->form_external->u_vdata = &holder;
		fl_set_form_atclose(form_external->form_external,
				    CancelCloseBoxCB, 0);
	}
	holder.view = bv;
	fl_addto_choice(form_external->templatechoice,
			getTemplateString().c_str());
	fl_set_input(form_external->filename, filename.c_str());
	fl_set_input(form_external->parameters, parameters.c_str());
	if (!templatename.empty()) {
		fl_set_choice(form_external->templatechoice,
			      getTemplateNumber(templatename));
	}
	// Update the help text
	templateCB(form_external->templatechoice, 0);

	ExternalTemplate const & et = getTemplate(templatename);
	if (et.automaticProduction) {
		fl_deactivate_object(form_external->update);
		fl_set_object_lcol(form_external->update, FL_INACTIVE);
	} else {
		fl_activate_object(form_external->update);
		fl_set_object_lcol(form_external->update, FL_BLACK);
	}

	if (form_external->form_external->visible) {
		fl_raise_form(form_external->form_external);
	} else {
		fl_show_form(form_external->form_external,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_TRANSIENT,
			     _("Insert external inset"));
		if (ow < 0) {
			ow = form_external->form_external->w;
			oh = form_external->form_external->h;
		}
		fl_set_form_minsize(form_external->form_external, ow, oh);
	}
}


void InsetExternal::Write(Buffer const *, std::ostream & os) const
{
	os << "External " << templatename << ",\"" << filename 
	   << "\",\"" << parameters << "\"\n";
}


void InsetExternal::Read(Buffer const *, LyXLex & lex)
{
	lex.EatLine();
	string const format = lex.GetString();
	string::size_type const pos1 = format.find(",");
	templatename = format.substr(0, pos1);
	string::size_type const pos2 = format.find("\",\"", pos1);
	filename = format.substr(pos1 + 2, pos2 - (pos1 + 2));
	parameters = format.substr(pos2 + 3, format.length() - (pos2 + 4));

	lyxerr << templatename << " " << filename << " " << parameters << endl;
}


int InsetExternal::write(string const & format,
			 Buffer const * buf, std::ostream & os) const
{
	ExternalTemplate const & et = getTemplate(templatename);
	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find(format);
	if (cit == et.formats.end()) {
		lyxerr << "External template format '" << format
		       << "' not specified in template " << templatename
		       << endl;
		return 0;
	}
	
	if (et.automaticProduction) {
		executeCommand(doSubstitution(buf,
					      (*cit).second.updateCommand),
			       buf);
	}
	
	os << doSubstitution(buf, (*cit).second.product);
	return 0; // CHECK
}


int InsetExternal::Latex(Buffer const * buf,
			 std::ostream & os, bool, bool) const
{
	return write("LaTeX", buf, os);
}


int InsetExternal::Ascii(Buffer const * buf, std::ostream & os, int) const
{
	return write("Ascii", buf, os);
}


int InsetExternal::Linuxdoc(Buffer const * buf, std::ostream & os) const
{
	return write("LinuxDoc", buf, os);
}


int InsetExternal::DocBook(Buffer const * buf, std::ostream & os) const
{
	return write("DocBook", buf, os);
}


void InsetExternal::Validate(LaTeXFeatures & features) const
{
	ExternalTemplate const & et = getTemplate(templatename);
	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find("LaTeX");
	if (cit == et.formats.end()) {
		return;
	}
	
	if (!(*cit).second.requirement.empty()) {
		features.require((*cit).second.requirement);
	}
	if (!(*cit).second.preamble.empty()) {
		features.externalPreambles += (*cit).second.preamble + "\n";
	}
}


Inset * InsetExternal::Clone(Buffer const &) const
{
	InsetExternal * inset = new InsetExternal();
	inset->templatename = templatename;
	inset->filename = filename;
	inset->parameters = parameters;
	return inset;
}


string const InsetExternal::getScreenLabel() const
{
	if (templatename.empty()) {
		return _("External");
	} else {
		ExternalTemplate const & et = getTemplate(templatename);
		return doSubstitution(0, et.guiName);
	}
}


void InsetExternal::doUpdate(BufferView const * bv) const
{
	ExternalTemplate const & et = getTemplate(getCurrentTemplate());
	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find("LaTeX");
	if (cit == et.formats.end())
		return;
	
	executeCommand(doSubstitution(bv->buffer(),
				      (*cit).second.updateCommand),
		       bv->buffer());
}


void InsetExternal::doView(BufferView const * bv) const
{
	automaticUpdate(bv);
	ExternalTemplate const & et = getTemplate(getCurrentTemplate());
	executeCommand(doSubstitution(bv->buffer(), et.viewCommand),
		       bv->buffer());
}


void InsetExternal::doEdit(BufferView const * bv) const
{
	automaticUpdate(bv);
	ExternalTemplate const & et = getTemplate(getCurrentTemplate());
	executeCommand(doSubstitution(bv->buffer(), et.editCommand),
		       bv->buffer());
}


void InsetExternal::executeCommand(string const & s,
				   Buffer const * buffer) const
{
	string buf = MakeAbsPath(buffer->fileName());
	string path = OnlyPath(buf);
	Path p(path);
	Systemcalls one;
	if (lyxerr.debugging()) {
		lyxerr << "Executing '" << s << "' in '"
		       << path << "'" << endl;
	}
	one.startscript(Systemcalls::Wait, s);
}


void InsetExternal::automaticUpdate(BufferView const * bv) const
{
	ExternalTemplate const & et = getTemplate(templatename);
	if (et.automaticProduction) {
		doUpdate(bv);
	}
}


string const InsetExternal::doSubstitution(Buffer const * buffer,
				     string const & s) const
{
	string result;
	string const basename = ChangeExtension(filename, string());
	result = subst(s, "$$FName", filename);
	result = subst(result, "$$Basename", basename);
	result = subst(result, "$$Parameters", parameters);
	result = ReplaceEnvironmentPath(result);
	result = subst(result, "$$Tempname", tempname);
	result = subst(result, "$$Sysdir", system_lyxdir);
	
	// Handle the $$Contents(filename) syntax
	if (contains(result, "$$Contents(\"")) {

		string::size_type const pos = result.find("$$Contents(\"");
		string::size_type const end = result.find("\")", pos);
		string const file = result.substr(pos + 12, end - (pos + 12));
		string contents;
		if (buffer) {
			// Make sure we are in the directory of the buffer
			string const buf = MakeAbsPath(buffer->fileName());
			string const path = OnlyPath(buf);
			Path p(path);
			contents = GetFileContents(file);
		} else {
			contents = GetFileContents(file);
		}
		result = subst(result,
			       ("$$Contents(\"" + file + "\")").c_str(),
			       contents);
	}

	return result;
}


string const InsetExternal::getCurrentTemplate() const
{
	return getTemplateName(fl_get_choice(form_external->templatechoice));
}


ExternalTemplate const InsetExternal::getTemplate(string const & name) const
{
	ExternalTemplateManager::Templates::iterator i = 
		ExternalTemplateManager::get().getTemplates().find(name);
	// Make sure that the template exists in the map
	if (i == ExternalTemplateManager::get().getTemplates().end()) {
		lyxerr << "Unknown external material template: "
		       << name << endl;
		return ExternalTemplate();
	}
	return (*i).second;
}


int InsetExternal::getTemplateNumber(string const & name) const
{
	int i = 1;
	ExternalTemplateManager::Templates::const_iterator i1, i2;
	i1 = ExternalTemplateManager::get().getTemplates().begin();
	i2 = ExternalTemplateManager::get().getTemplates().end();
	for (; i1 != i2; ++i1) {
		if ((*i1).second.lyxName == name)
			return i;
		++i;
	}
	// This should never happen
	Assert(false);
	return 0;
}


string const InsetExternal::getTemplateName(int i) const
{
	ExternalTemplateManager::Templates::const_iterator i1;
	i1 = ExternalTemplateManager::get().getTemplates().begin();
	for (int n = 1; n < i; ++n) {
		++i1;
	}
	return (*i1).second.lyxName;
}


string const InsetExternal::getTemplateString() const
{
	string result;
	bool first = true;
	ExternalTemplateManager::Templates::const_iterator i1, i2;
	i1 = ExternalTemplateManager::get().getTemplates().begin();
	i2 = ExternalTemplateManager::get().getTemplates().end();
	for (; i1 != i2; ++i1) {
		if (!first) {
			result += "|";
		} else {
			first = false;
		}
		result += (*i1).second.lyxName;
	}
	return result;
}
