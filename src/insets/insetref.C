#include <config.h>

#include <stdlib.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION 
#include "insetref.h"
#include "buffer.h"
#include "error.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB
#include "LyXView.h"
#include "lyxfunc.h"
#include "commandtags.h"
#include "gettext.h"

extern BufferView *current_view;

// Temporarily disabled the GUI code. Reasons:
// - Only page-ref button works currently, IMO we should use a LyX action
//   instead, to toggle the kind of refs.
// - To change the label, IMO it's faster to delete the old one and insert
//   a new one.
// - To goto to the label, IMO it's much faster to just click on the
//   inset. That's how I've implemented it now, I hope you'll like it.
// - The more GUI code we can remove, the less work we'll have at
//   the toolkit switch.
//   (ale 970723)

#if 0

/* Header file generated with fdesign. */

/**** Callback routines ****/

static void ref_close_cb(FL_OBJECT *, long);
static void goto_label_cb(FL_OBJECT *, long);
static void label_change_cb(FL_OBJECT *, long);

/**** Forms and Objects ****/

typedef struct {
	FL_FORM *ref;
	void *vdata;
	long ldata;
	FL_OBJECT *pg_grp;
	FL_OBJECT *flag1;
	FL_OBJECT *flag2;
} FD_ref;

/* Form definition file generated with fdesign. */

static
FD_ref *create_form_ref(void)
{
  FL_OBJECT *obj;
  FD_ref *fdui = (FD_ref *) fl_calloc(1, sizeof(*fdui));

  fdui->ref = fl_bgn_form(FL_NO_BOX, 210, 170);
  obj = fl_add_box(FL_UP_BOX,0,0,210,170,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,20,130,60,"");
  obj = fl_add_button(FL_RETURN_BUTTON,120,130,80,30,_("Close"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,ref_close_cb,0);
  obj = fl_add_text(FL_NORMAL_TEXT,20,10,110,20,_("Reference Type"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_button(FL_NORMAL_BUTTON,10,130,100,30,_("Goto Label"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,goto_label_cb,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,10,90,100,30,_("Change Label"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,label_change_cb,0);

  fdui->pg_grp = fl_bgn_group();
  fdui->flag1 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,20,30,20,20,_("Page Number"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fdui->flag2 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,20,50,20,20,_("Reference"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_group();

  fl_end_form();

  fdui->ref->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/


static FD_ref *form = 0;


static
void ref_close_cb(FL_OBJECT *, long)
{
	InsetRef *inset = (InsetRef*)form->vdata;

	if (fl_get_button(form->flag1)) {
		inset->setFlag(InsetRef::PAGE_REF);
		inset->setCmdName("pageref");
	} else {
		inset->setFlag(InsetRef::REF);
		inset->setCmdName("ref");
	}
		
	fl_hide_form(form->ref);
}


static
void goto_label_cb(FL_OBJECT *, long)
{
	// code yet to be written
	InsetRef *inset = (InsetRef*)form->vdata;
        inset->gotoLabel();
#warning MAKEME!
}


static
void label_change_cb(FL_OBJECT *, long)
{
	// code yet to be written
	InsetRef *inset = (InsetRef*)form->vdata;
#warning MAKEME!
}

#endif

InsetRef::InsetRef(string const & cmd, Buffer *bf)
	: master(bf)
{
	scanCommand(cmd);
	if (getCmdName() == "ref")
		flag = InsetRef::REF;
	else
		flag = InsetRef::PAGE_REF;
}


InsetRef::InsetRef(InsetCommand const &inscmd, Buffer *bf)
	: master(bf)
{
	setCmdName(inscmd.getCmdName());
	setContents(inscmd.getContents());
	setOptions(inscmd.getOptions());
	if (getCmdName() == "ref")
		flag = InsetRef::REF;
	else
		flag = InsetRef::PAGE_REF;
}


InsetRef::~InsetRef()
{
}


void InsetRef::Edit(int, int)
{
        current_view->getOwner()->getLyXFunc()->Dispatch(LFUN_REFGOTO
							 , getContents().c_str());
//        gotoLabel();
/*    
        if (!form) { 
                form = create_form_ref();
		fl_set_form_atclose(form->ref, IgnoreCloseBoxCB, 0);
	}
        form->vdata = this; 
    
	fl_set_button(form->flag1, (flag == InsetRef::REF) ? 1 : 0);
	fl_set_button(form->flag2, (flag == InsetRef::PAGE_REF) ? 1 : 0);
	
        if (form->ref->visible) {
		fl_raise_form(form->ref);
	} else {
		fl_show_form(form->ref,FL_PLACE_MOUSE, FL_FULLBORDER,
			     _("Cross-Reference"));
	}
 */
}


string InsetRef::getScreenLabel() const
{
	string temp;
	if (flag == InsetRef::PAGE_REF)
		temp += _("Page: ");
	else 
		temp += _("Ref: ");
	temp += getContents();
	if(!current_view->currentBuffer()->isLatex() && !getOptions().empty()) {
		temp += "||";
		temp += getOptions();
	}
	return temp;
}


int InsetRef::Latex(FILE *file, signed char /*fragile*/)
{
	if(getOptions().empty())
		fprintf(file, "%s", escape(getCommand()).c_str());
	else {
		string ns;
		InsetCommand clone= InsetCommand(getCmdName(),getContents(),ns);
		fprintf(file, "%s", escape(clone.getCommand()).c_str());
	}
	return 0;
}


int InsetRef::Latex(string &file, signed char /*fragile*/)
{
	if(getOptions().empty())
		file += escape(getCommand());
	else {
		string ns;
		InsetCommand clone= InsetCommand(getCmdName(),getContents(),ns);
		file += escape(clone.getCommand());
	}
	return 0;
}


int InsetRef::Linuxdoc(string &file)
{
	file += "<ref id=\"" + getContents() + "\" name=\""+ getOptions() +"\" >" ;

	return 0;
}


int InsetRef::DocBook(string &file)
{
	file += "<link linkend=\"" + getContents() + "\">"+ getOptions() +"</link>" ;

	return 0;
}


// This function escapes 8-bit characters and other problematic characters
// It's exactly the same code as in insetlabel.C.
string InsetRef::escape(string const & lab) const {
	char hexdigit[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
			      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	string enc;
	for (string::size_type i = 0; i < lab.length(); ++i) {
		unsigned char c=lab[i];
		if (c >= 128 || c=='=' || c=='%') {
			enc += '=';
			enc += hexdigit[c>>4];
			enc += hexdigit[c & 15];
		} else {
			enc += (char) c;
		}
	}
	return enc;
}
