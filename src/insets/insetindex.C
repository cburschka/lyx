#include <config.h>

#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION  
#include "insetindex.h"
#include "buffer.h"
#include "debug.h"
#include "LaTeXFeatures.h"
#include "gettext.h"
#include "LString.h"
#include "lyx_gui_misc.h" // WarnReadonly
 
FD_index_form * index_form = 0;

extern "C" void index_cb(FL_OBJECT *, long data)
{
	InsetIndex::Holder * holder =
		static_cast<InsetIndex::Holder*>
		(index_form->index_form->u_vdata);

	holder->inset->callback( index_form, data );
}
	

static
FD_index_form * create_form_index_form()
{
	FL_OBJECT * obj;
	FD_index_form *fdui = (FD_index_form *) fl_calloc(1, sizeof(FD_index_form));

	fdui->index_form = fl_bgn_form(FL_NO_BOX, 258, 196);
	obj = fl_add_box(FL_UP_BOX, 0, 0, 258, 196, "");
	fdui->key = obj = fl_add_input(FL_NORMAL_INPUT, 93, 26, 130, 30,
				       idex(_("Keyword:|#K")));
	  fl_set_object_shortcut(obj, scex(_("Keyword:|#K")), 1);
	  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	obj = fl_add_button(FL_RETURN_BUTTON, 50, 140, 80, 30, _("OK"));
	  obj->u_vdata = index_form;
	  fl_set_object_callback(obj, index_cb, 1);
	obj = fl_add_button(FL_NORMAL_BUTTON, 150, 140, 80, 30,
			    idex(_("Cancel|^[")));
	  fl_set_object_shortcut(obj, scex(_("Cancel|^[")), 1);
	  obj->u_vdata = index_form;
	  fl_set_object_callback(obj, index_cb, 0);
	fl_end_form();

	return fdui;
}



/*---------------------------------------*/


InsetIndex::InsetIndex(string const & key)
	: InsetCommand("index", key) 
{
}


InsetIndex::~InsetIndex()
{
	if(index_form && index_form->index_form
	   && index_form->index_form->visible
	   && index_form->index_form->u_vdata == &holder)
		fl_hide_form(index_form->index_form);
}


void InsetIndex::callback( FD_index_form * form, long data )
{
	switch (data)
	{
	case 1: // OK
		if(!holder.view->buffer()->isReadonly())
		{
			string tmp = fl_get_input(form->key);
			if(tmp != getContents())
			{
				setContents(tmp);
				holder.view->updateInset( this, true );
			}
		} // fall through to Cancel
	case 0:
		fl_hide_form(form->index_form);
		break;
	}
}


void InsetIndex::Edit(BufferView * bv, int, int, unsigned int)
{
	if(bv->buffer()->isReadonly())
		WarnReadonly(bv->buffer()->fileName());

	if (!index_form) {
		index_form = create_form_index_form();
		fl_set_form_atclose(index_form->index_form, CancelCloseBoxCB, 0);
	}

	holder.inset = this;
	holder.view = bv;
	
	index_form->index_form->u_vdata =  &holder;
	fl_set_input(index_form->key, getContents().c_str());
	if (index_form->index_form->visible) {
		fl_raise_form(index_form->index_form);
	} else {
		fl_show_form(index_form->index_form,
			     FL_PLACE_MOUSE, FL_FULLBORDER,
			     _("Index"));
	}
}


string InsetIndex::getScreenLabel() const
{
	return _("Idx");
}


//
// InsetPrintIndex
//

InsetPrintIndex::InsetPrintIndex()
	: InsetCommand("printindex")
{
	owner = 0;
}


InsetPrintIndex::InsetPrintIndex(Buffer * o)
	: InsetCommand("printindex"), owner(o)
{
}


InsetPrintIndex::~InsetPrintIndex()
{
}


string InsetPrintIndex::getScreenLabel() const
{
	return _("PrintIndex");
}


void InsetPrintIndex::Validate(LaTeXFeatures & features) const
{
	features.makeidx = true;
}


Inset::Code InsetPrintIndex::LyxCode() const
{
 	return Inset::INDEX_CODE;
}
