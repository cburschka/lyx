#include <config.h>

#include <stdlib.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION 
#include "inseturl.h"
#include "LString.h"
#include "commandtags.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB

extern BufferView *current_view;
extern void UpdateInset(Inset* inset, bool mark_dirty = true);

InsetUrl::InsetUrl(string const & cmd)
	: form(0)
{
	scanCommand(cmd);
	if (getCmdName() == "url")
		flag = InsetUrl::URL;
	else
		flag = InsetUrl::HTML_URL;
}


InsetUrl::InsetUrl(InsetCommand const &inscmd)
	: form(0)
{
	setCmdName(inscmd.getCmdName());
	setContents(inscmd.getContents());
	setOptions(inscmd.getOptions());
	if (getCmdName() == "url")
		flag = InsetUrl::URL;
	else
		flag = InsetUrl::HTML_URL;
}


InsetUrl::InsetUrl(string const &ins_name,string const &ins_cont,
		   string const &ins_opt)
	: form(0)
{
	setCmdName(ins_name);
	setContents(ins_cont);
	setOptions(ins_opt);
	if (ins_name == "url")
		flag = InsetUrl::URL;
	else
		flag = InsetUrl::HTML_URL;
}


InsetUrl::~InsetUrl()
{
	if (form) {
		fl_hide_form(form);
		fl_free_form(form);
		form = 0;
	}
}


void InsetUrl::CloseUrlCB(FL_OBJECT *ob, long)
{
	InsetUrl *inset = (InsetUrl*) ob->u_vdata;
	string url = fl_get_input(inset->url_name);
	string name = fl_get_input(inset->name_name);
	string cmdname;
	if (fl_get_button(inset->radio_html))
		cmdname = "htmlurl";
	else
		cmdname = "url";
	
	Buffer *buffer = current_view->currentBuffer();
	
	if ((url != inset->getContents() ||
	     name != inset->getOptions() ||
	     cmdname != inset->getCmdName())
	    && !(buffer->isReadonly()) ) {
		buffer->markDirty();
		inset->setContents(url);
		inset->setOptions(name);
		inset->setCmdName(cmdname);
		if (cmdname == "url")
			inset->flag = InsetUrl::URL;
		else
			inset->flag = InsetUrl::HTML_URL;
		UpdateInset(inset);
	}
	
	if (inset->form) {
		fl_hide_form(inset->form);
		inset->form = 0;
	}
}

extern "C" void C_InsetUrl_CloseUrlCB(FL_OBJECT *ob, long)
{
	InsetUrl::CloseUrlCB(ob,0);
}

void InsetUrl::Edit(int, int)
{
	if(current_view->currentBuffer()->isReadonly())
		WarnReadonly();

	if (!form) {
		FL_OBJECT *obj;
		form = fl_bgn_form(FL_NO_BOX, 530, 170);
		obj = fl_add_box(FL_UP_BOX,0,0,530,170,"");
		url_name = obj = fl_add_input(FL_NORMAL_INPUT,50,30,460,30,idex(_("Url|#U")));
		fl_set_button_shortcut(obj,scex(_("Url|#U")),1);
		name_name = obj = fl_add_input(FL_NORMAL_INPUT,50,80,460,30,idex(_("Name|#N")));
		fl_set_button_shortcut(obj,scex(_("Name|#N")),1);
		obj = fl_add_button(FL_RETURN_BUTTON,360,130,100,30,idex(_("Close|#C^[^M")));
		fl_set_button_shortcut(obj,scex(_("Close|#C^[^M")),1);
		obj->u_vdata = this;
		fl_set_object_callback(obj,C_InsetUrl_CloseUrlCB,0);
		radio_html = obj = fl_add_checkbutton(FL_PUSH_BUTTON,50,130,240,30,idex(_("HTML type|#H")));
		fl_set_button_shortcut(obj,scex(_("HTML type|#H")),1);
		fl_set_object_lsize(obj,FL_NORMAL_SIZE);
		fl_end_form();
		fl_set_form_atclose(form, CancelCloseBoxCB, 0);
	}
	fl_set_input(url_name, getContents().c_str());
	fl_set_input(name_name, getOptions().c_str());
	switch(flag) {
	case InsetUrl::URL:
		fl_set_button(radio_html, 0);
		break;
	case InsetUrl::HTML_URL:
		fl_set_button(radio_html, 1);
		break;
	}
	
	if (form->visible) {
		fl_raise_form(form);
	} else {
		fl_show_form(form, FL_PLACE_MOUSE,
			     FL_FULLBORDER, _("Insert Url"));
	}
}


string InsetUrl::getScreenLabel() const
{
	string temp;
	if (flag == InsetUrl::HTML_URL)
		temp += _("HtmlUrl: ");
	else 
		temp += _("Url: ");
	temp += getContents();
	if(!getOptions().empty()) {
		temp += "||";
		temp += getOptions();
	}
	return temp;
}


int InsetUrl::Latex(FILE *file, signed char fragile)
{
	string latex_output;
	int res = Latex(latex_output, fragile);
	fprintf(file, "%s", latex_output.c_str());

	return res;
}


int InsetUrl::Latex(string &file, signed char fragile)
{
	if (!getOptions().empty())
		file += getOptions() + ' ';
	if (fragile)
		file += "\\protect";

	file += "\\url{" + getContents() + '}';

	return 0;
}


int InsetUrl::Linuxdoc(string &file)
{
	file +=  "<"+ getCmdName() +
		 " url=\""  + getContents()+"\"" +
		 " name=\"" + getOptions() +"\">";

	return 0;
}


int InsetUrl::DocBook(string &file)
{
	file +=  "<ulink url=\""  + getContents() + "\">" +
		 getOptions() +"</ulink>";

	return 0;
}


void InsetUrl::Validate(LaTeXFeatures& features) const
{
	features.url = true;
}
