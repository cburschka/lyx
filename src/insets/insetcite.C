#include <config.h>

#include <fstream>
#include <cstdlib>
#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION  
#include "insetcite.h"
#include "buffer.h"
#include "debug.h"
#include "lyx_gui_misc.h"
#include "BufferView.h"
#include "gettext.h"
#include "lyxtext.h"
#include "support/filetools.h"

using std::getline;
using std::vector;
using std::pair;
using std::max;
using std::min;
using std::find;

FD_citation_form * citation_form = 0;
FD_citation_form * create_form_citation_form(void);
void set_size_citation_form(FD_citation_form *, int, bool);

static vector<pair<string,string> > bibkeys_info;
static vector<string> bibkeys;
static vector<string> insetkeys;

extern "C" void citation_cb( FL_OBJECT *, long data )
{
	InsetCitation::Holder * holder =
		static_cast<InsetCitation::Holder*>(citation_form->form->u_vdata);

	holder->inset->callback( citation_form,
				 static_cast<InsetCitation::State>(data) );
}


FD_citation_form *create_form_citation_form(void)
{
  FL_OBJECT *obj;
  FD_citation_form *fdui = (FD_citation_form *) fl_calloc(1, sizeof(*fdui));

  fdui->form = fl_bgn_form(FL_NO_BOX, 440, 790);
  fdui->box = obj = fl_add_box(FL_UP_BOX,0,0,440,790,"");
    fl_set_object_resize(obj, FL_RESIZE_X);

  fdui->citeBrsr = obj =
    fl_add_browser(FL_HOLD_BROWSER,20,40,170,370,_("Inset keys"));
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,citation_cb,InsetCitation::CITEBRSR);

  fdui->bibBrsr = obj =
    fl_add_browser(FL_HOLD_BROWSER,250,40,170,370,_("Bibliography keys"));
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,citation_cb,InsetCitation::BIBBRSR);

  fdui->addBtn = obj =
    fl_add_button(FL_NORMAL_BUTTON,200,40,40,40,"@4->");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,citation_cb,InsetCitation::ADD);

  fdui->delBtn = obj =
    fl_add_button(FL_NORMAL_BUTTON,200,90,40,40,"@9+");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,citation_cb,InsetCitation::DELETE);

  fdui->upBtn = obj =
    fl_add_button(FL_NORMAL_BUTTON,200,140,40,40,"@8->");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,citation_cb,InsetCitation::UP);

  fdui->downBtn = obj =
    fl_add_button(FL_NORMAL_BUTTON,200,190,40,40,"@2->");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,citation_cb,InsetCitation::DOWN);

  fdui->infoBrsr = obj =
    fl_add_browser(FL_NORMAL_BROWSER,20,440,400,110,_("Info"));
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_resize(obj, FL_RESIZE_X);

  /*
  fdui->style = obj =
    fl_add_choice(FL_NORMAL_CHOICE,160,570,130,30,_("Citation style"));
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_resize(obj, FL_RESIZE_X);

  fdui->textBefore = obj =
    fl_add_input(FL_NORMAL_INPUT,100,620,250,30,_("Text before"));
    fl_set_object_resize(obj, FL_RESIZE_X);
  */

  fdui->textAftr = obj =
    fl_add_input(FL_NORMAL_INPUT,100,660,250,30,_("Text after"));
    fl_set_object_resize(obj, FL_RESIZE_X);

  fdui->ok = obj =
    fl_add_button(FL_RETURN_BUTTON,190,730,110,40,_("OK"));
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,citation_cb,InsetCitation::OK);

  fdui->cancel = obj =
    fl_add_button(FL_NORMAL_BUTTON,310,730,110,40,idex(_("Cancel|^[")));
    fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,citation_cb,InsetCitation::CANCEL);

  fl_end_form();

  return fdui;
}


InsetCitation::InsetCitation(string const & key, string const & note)
	: InsetCommand("cite", key, note)
{
}


InsetCitation::~InsetCitation()
{
	if(citation_form && citation_form->form
	   && citation_form->form->visible
	   && citation_form->form->u_vdata == &holder)
		fl_hide_form(citation_form->form);
}


void InsetCitation::Edit( BufferView * bv, int, int, unsigned int )
{
	if ( !citation_form ) {
		citation_form = create_form_citation_form();
		fl_set_form_atclose( citation_form->form, 
				     CancelCloseBoxCB, 0 );
	}

	holder.inset = this;
	holder.view = bv;
		
	citation_form->form->u_vdata = &holder;

	// update the browsers, noting the number of keys.
	bibkeys_info = bv->buffer()->getBibkeyList();
	bibkeys.clear();
	insetkeys.clear();
	for( unsigned int i = 0; i < bibkeys_info.size(); ++i )
		bibkeys.push_back(bibkeys_info[i].first);

	string tmp;
	string keys = getContents();
	keys = frontStrip( split(keys, tmp, ',') );
	while( !tmp.empty() ) {
		insetkeys.push_back( tmp );
		keys = frontStrip( split(keys, tmp, ',') );
	}

	updateBrowser( citation_form->bibBrsr, bibkeys );
	updateBrowser( citation_form->citeBrsr, insetkeys );
	fl_clear_browser( citation_form->infoBrsr );

	// No keys have been selected yet, so...
	setBibButtons(  citation_form, OFF );
	setCiteButtons( citation_form, OFF );

	int noKeys = max( bibkeys.size(), insetkeys.size() );

	// Place bounds, so that 4 <= noKeys <= 15
	noKeys = max( 4, min(15, noKeys) );

	// Re-size the form to accommodate the new browser size
	int size = 20 * noKeys;
	bool bibPresent = ( bibkeys.size() > 0 );
	setSize(citation_form, size, bibPresent);

	fl_set_input( citation_form->textAftr, getOptions().c_str() );
	if( holder.view->buffer()->isReadonly() )
		fl_deactivate_object( citation_form->textAftr );

	if( citation_form->form->visible ) {
		fl_raise_form( citation_form->form );
	} else {
		fl_show_form(citation_form->form,
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     FL_FULLBORDER,
			     _("Citation") );
	}
}


void InsetCitation::updateBrowser( FL_OBJECT * browser,
				   vector<string> const & inkeys ) const
{
	fl_clear_browser( browser );

	fl_freeze_form( browser->form );
	for( unsigned int i = 0; i < inkeys.size(); ++i )
		fl_add_browser_line( browser, inkeys[i].c_str() );
	fl_unfreeze_form( browser->form );
}

void InsetCitation::callback( FD_citation_form * form, State cb )
{
	switch( cb ) {
	case BIBBRSR: {
		fl_deselect_browser( form->citeBrsr );
		
		unsigned int sel = fl_get_browser( form->bibBrsr );
		if( sel < 1 || sel > bibkeys.size() ) break;

		// Put into infoBrsr the additional info associated with
		// the selected bibBrsr key
		fl_clear_browser( form->infoBrsr );
		fl_add_browser_line( form->infoBrsr,
				     bibkeys_info[sel-1].second.c_str() );

		// Highlight the selected bibBrsr key in citeBrsr if present
		vector<string>::iterator it =
			find( insetkeys.begin(), insetkeys.end(), bibkeys[sel-1] );

		if( it != insetkeys.end() ) {
			int n = it - insetkeys.begin();
			fl_select_browser_line( form->citeBrsr, n+1 );
			fl_set_browser_topline( form->citeBrsr, n+1 );
		}

		if( !holder.view->buffer()->isReadonly() ) {
			if( it != insetkeys.end() ) {
				setBibButtons(  form, OFF );
				setCiteButtons( form, ON );
			} else {
				setBibButtons(  form, ON );
				setCiteButtons( form, OFF );
			}
		}
		break;

	} case CITEBRSR: {
		unsigned int sel = fl_get_browser( form->citeBrsr );
		if( sel < 1 || sel > insetkeys.size() ) break;

		if( !holder.view->buffer()->isReadonly() ) {
			setBibButtons(  form, OFF );
			setCiteButtons( form, ON );
		}

		// Highlight the selected citeBrsr key in bibBrsr
		vector<string>::iterator it =
			find( bibkeys.begin(), bibkeys.end(), insetkeys[sel-1] );

		if (it != bibkeys.end()) {
			int n = it - bibkeys.begin();
			fl_select_browser_line( form->bibBrsr, n+1 );
			fl_set_browser_topline( form->bibBrsr, n+1 );

			// Put into infoBrsr the additional info associated with
			// the selected citeBrsr key
			fl_clear_browser( form->infoBrsr );
			fl_add_browser_line( form->infoBrsr,
					     bibkeys_info[n].second.c_str() );
		}
		break;

	} case ADD: {
		if( holder.view->buffer()->isReadonly() ) break;

		unsigned int sel = fl_get_browser( form->bibBrsr );
		if( sel < 1 || sel > bibkeys.size() ) break;

		// Add the selected bibBrsr key to citeBrsr
		fl_addto_browser( form->citeBrsr,
				  bibkeys[sel-1].c_str() );
		insetkeys.push_back( bibkeys[sel-1] );

		int n = insetkeys.size();
		fl_select_browser_line( form->citeBrsr, n );

		setBibButtons(  form, OFF );
		setCiteButtons( form, ON );

		break;

	} case DELETE: {
		if( holder.view->buffer()->isReadonly() ) break;

		unsigned int sel = fl_get_browser( form->citeBrsr );
		if( sel < 1 || sel > insetkeys.size() ) break;

		// Remove the selected key from citeBrsr
		fl_delete_browser_line( form->citeBrsr, sel ) ;
		insetkeys.erase( insetkeys.begin() + sel-1 );

		setBibButtons(  form, ON );
		setCiteButtons( form, OFF );
		break;

	} case UP: {
		if( holder.view->buffer()->isReadonly() ) break;

		unsigned int sel = fl_get_browser( form->citeBrsr );
		if( sel < 2 || sel > insetkeys.size() ) break;

		// Move the selected key up one line
		vector<string>::iterator it = insetkeys.begin() + sel-1;
		string tmp = *it;

		fl_delete_browser_line( form->citeBrsr, sel );
		insetkeys.erase( it );

		fl_insert_browser_line( form->citeBrsr, sel-1, tmp.c_str() );
		fl_select_browser_line( form->citeBrsr, sel-1 );
		insetkeys.insert( it-1, tmp );
		setCiteButtons( form, ON );

		break;

	} case DOWN: {
		if( holder.view->buffer()->isReadonly() ) break;

		unsigned int sel = fl_get_browser( form->citeBrsr );
		if( sel < 1 || sel > insetkeys.size()-1 ) break;

		// Move the selected key down one line
		vector<string>::iterator it = insetkeys.begin() + sel-1;
		string tmp = *it;

		fl_delete_browser_line( form->citeBrsr, sel );
		insetkeys.erase( it );

		fl_insert_browser_line( form->citeBrsr, sel+1, tmp.c_str() );
		fl_select_browser_line( form->citeBrsr, sel+1 );
		insetkeys.insert( it+1, tmp );
		setCiteButtons( form, ON );

		break;

	} case OK: {
		// The inset contains a comma separated list of the keys
		// in citeBrsr
		if( !holder.view->buffer()->isReadonly() )
		{
			string tmp;
			for( unsigned int i = 0; i < insetkeys.size(); ++i ) {
				if (i > 0)
					tmp += ", ";
				tmp += insetkeys[i];
			}
			setContents( tmp );
			setOptions( fl_get_input(form->textAftr) );
			// shouldn't mark the buffer dirty unless something
			// was actually altered
			holder.view->updateInset( this, true );
		}
		// fall through to Cancel

	} case CANCEL: {
		fl_hide_form( form->form );
		break;

        } default:
		break;
	}
}


void InsetCitation::setSize( FD_citation_form * form,
			     int brsrHeight, bool bibPresent ) const
{
	int const infoHeight  = 110;
	int const otherHeight = 140;
	brsrHeight = max( brsrHeight, 175 );
	int formHeight = brsrHeight + otherHeight;

	if( bibPresent ) formHeight += infoHeight + 30;
	fl_set_form_size( form->form, 430, formHeight );

	// No resizing is alowed in the y-direction
	fl_set_form_minsize( form->form, 300, formHeight );
	fl_set_form_maxsize( form->form, 1000, formHeight );

	int ypos = 0;
	fl_set_object_geometry( form->box,      0,   ypos, 430, formHeight );
	ypos += 30;
	fl_set_object_geometry( form->citeBrsr, 10,  ypos, 180, brsrHeight );
	fl_set_object_geometry( form->bibBrsr,  240, ypos, 180, brsrHeight );
	fl_set_object_geometry( form->addBtn,   200, ypos,  30, 30 );
	ypos += 35;
	fl_set_object_geometry( form->delBtn,   200, ypos,  30, 30 );
	ypos += 35;
	fl_set_object_geometry( form->upBtn,    200, ypos,  30, 30 );
	ypos += 35;
	fl_set_object_geometry( form->downBtn,  200, ypos,  30, 30 );

	ypos = brsrHeight+30; // base of Citation/Bibliography browsers

	if( bibPresent ) {
		ypos += 30;
		fl_set_object_geometry( form->infoBrsr, 10, ypos, 410, infoHeight );
		fl_show_object( form->infoBrsr );
		ypos += infoHeight;
	}
	else
		fl_hide_object( form->infoBrsr );

	ypos += 20;
	fl_set_object_geometry( form->textAftr, 100, ypos,   250, 30 );
	fl_set_object_geometry( form->ok,       230, ypos+50, 90, 30 );
	fl_set_object_geometry( form->cancel,   330, ypos+50, 90, 30 );
}


void InsetCitation::setBibButtons( FD_citation_form * form, State status ) const
{
	switch (status) {
	case ON:
        {
		fl_activate_object( form->addBtn );
		fl_set_object_lcol( form->addBtn, FL_BLACK );

		break;
	}
	case OFF:
	{
		fl_deactivate_object( form->addBtn );
		fl_set_object_lcol( form->addBtn, FL_INACTIVE );
	}
	default:
		break;
	}
}


void InsetCitation::setCiteButtons( FD_citation_form * form, State status ) const
{
	switch( status ) {
	case ON:
        {
		fl_activate_object( form->delBtn );
		fl_set_object_lcol( form->delBtn, FL_BLACK );

		int sel = fl_get_browser( form->citeBrsr );

		if( sel != 1 ) {
			fl_activate_object( form->upBtn );
			fl_set_object_lcol( form->upBtn, FL_BLACK );
		} else {
			fl_deactivate_object( form->upBtn );
			fl_set_object_lcol( form->upBtn, FL_INACTIVE );
		}

		if( sel != fl_get_browser_maxline(form->citeBrsr)) {
			fl_activate_object( form->downBtn );
			fl_set_object_lcol( form->downBtn, FL_BLACK );
		} else {
			fl_deactivate_object( form->downBtn );
			fl_set_object_lcol( form->downBtn, FL_INACTIVE );
		}

		break;
	}
	case OFF:
	{
		fl_deactivate_object( form->delBtn );
		fl_set_object_lcol( form->delBtn, FL_INACTIVE );

		fl_deactivate_object( form->upBtn );
		fl_set_object_lcol( form->upBtn, FL_INACTIVE );

		fl_deactivate_object( form->downBtn );
		fl_set_object_lcol( form->downBtn, FL_INACTIVE );
	}
	default:
		break;
	}
}


string InsetCitation::getScreenLabel() const
{
	string temp("[");

	temp += getContents();

	if( !getOptions().empty() ) {
		temp += ", " + getOptions();
	}

	return temp + ']';
}
