#include <config.h>

#include <cstdlib>

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

extern BufferView * current_view;
extern void UpdateInset(BufferView *, Inset * inset, bool mark_dirty = true);

InsetUrl::InsetUrl(string const & cmd)
	: fd_form_url(0)
{
	scanCommand(cmd);
	if (getCmdName() == "url")
		flag = InsetUrl::URL;
	else
		flag = InsetUrl::HTML_URL;
}


InsetUrl::InsetUrl(InsetCommand const & inscmd)
	: fd_form_url(0)
{
	setCmdName(inscmd.getCmdName());
	setContents(inscmd.getContents());
	setOptions(inscmd.getOptions());
	if (getCmdName() == "url")
		flag = InsetUrl::URL;
	else
		flag = InsetUrl::HTML_URL;
}


InsetUrl::InsetUrl(string const & ins_name, string const & ins_cont,
		   string const & ins_opt)
	: fd_form_url(0)
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
	if (fd_form_url) {
		fl_hide_form(fd_form_url->form_url);
		fl_free_form(fd_form_url->form_url);
		fd_form_url = 0;
	}
}


void InsetUrl::CloseUrlCB(FL_OBJECT * ob, long)
{
	InsetUrl * inset = static_cast<InsetUrl*>(ob->u_vdata);
	string url = fl_get_input(inset->fd_form_url->url_name);
	string name = fl_get_input(inset->fd_form_url->name_name);
	string cmdname;
	if (fl_get_button(inset->fd_form_url->radio_html))
		cmdname = "htmlurl";
	else
		cmdname = "url";
	
	Buffer * buffer = current_view->buffer();
	
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
		UpdateInset(current_view, inset);
	}
	
	if (inset->fd_form_url) {
		fl_hide_form(inset->fd_form_url->form_url);
		fl_free_form(inset->fd_form_url->form_url);
		inset->fd_form_url = 0;
	}
}


extern "C" void C_InsetUrl_CloseUrlCB(FL_OBJECT * ob, long data)
{
	InsetUrl::CloseUrlCB(ob, data);
}


void InsetUrl::Edit(int, int)
{
	static int ow = -1, oh;

	if(current_view->buffer()->isReadonly())
		WarnReadonly(current_view->buffer()->fileName());

	if (!fd_form_url) {
		fd_form_url = create_form_form_url();
		fd_form_url->button_close->u_vdata = this;
		fl_set_form_atclose(fd_form_url->form_url,
				    CancelCloseBoxCB, 0);
	}
	fl_set_input(fd_form_url->url_name, getContents().c_str());
	fl_set_input(fd_form_url->name_name, getOptions().c_str());
	switch(flag) {
	case InsetUrl::URL:
		fl_set_button(fd_form_url->radio_html, 0);
		break;
	case InsetUrl::HTML_URL:
		fl_set_button(fd_form_url->radio_html, 1);
		break;
	}
	
	if (fd_form_url->form_url->visible) {
		fl_raise_form(fd_form_url->form_url);
	} else {
		fl_show_form(fd_form_url->form_url,
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     FL_FULLBORDER, _("Insert Url"));
		if (ow < 0) {
			ow = fd_form_url->form_url->w;
			oh = fd_form_url->form_url->h;
		}
		fl_set_form_minsize(fd_form_url->form_url, ow, oh);
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


int InsetUrl::Latex(ostream & os, signed char fragile)
{
	string latex_output;
	int res = Latex(latex_output, fragile);
	os << latex_output;

	return res;
}


int InsetUrl::Latex(string & file, signed char fragile)
{
	if (!getOptions().empty())
		file += getOptions() + ' ';
	if (fragile)
		file += "\\protect";

	file += "\\url{" + getContents() + '}';

	return 0;
}


int InsetUrl::Linuxdoc(string & file)
{
	file +=  "<"+ getCmdName() +
		 " url=\""  + getContents()+"\"" +
		 " name=\"" + getOptions() +"\">";

	return 0;
}


int InsetUrl::DocBook(string & file)
{
	file +=  "<ulink url=\""  + getContents() + "\">" +
		 getOptions() +"</ulink>";

	return 0;
}


void InsetUrl::Validate(LaTeXFeatures & features) const
{
	features.url = true;
}
