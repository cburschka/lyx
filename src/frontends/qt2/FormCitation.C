// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormCitationDialogImpl.h"
#include "FormCitation.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>

#undef emit
#include "qt2BC.h"
#include "ControlCitation.h"
#include "gettext.h"
#include "support/lstrings.h"
#include "biblio.h"
#include "helper_funcs.h"


using std::find;
using std::max;
using std::min;
using std::pair;
using std::sort;
using std::vector;

typedef Qt2CB<ControlCitation, Qt2DB<FormCitationDialogImpl> > base_class;

FormCitation::FormCitation(ControlCitation & c)
    : base_class(c, _("Citation"))
{}


void FormCitation::apply()
{
    controller().params().setCmdName("cite");
    controller().params().setContents(getStringFromVector(citekeys));
    
    string const after  = dialog_->textAfterED->text().latin1();
    controller().params().setOptions(after);
}


void FormCitation::hide()
{
    citekeys.clear();
    bibkeys.clear();
    
    Qt2Base::hide();
}


void FormCitation::build()
{
    // PENDING(kalle) Parent?
    dialog_.reset( new FormCitationDialogImpl( this ));

    dialog_->searchTypePB->setOn( false );
    dialog_->searchTypePB->setText( _( "Simple" ) );
    
    // Manage the ok, apply, restore and cancel/close buttons
    bc().setOK(dialog_->okPB);
    bc().setApply(dialog_->applyPB);
    bc().setCancel(dialog_->cancelPB);
    bc().setUndoAll(dialog_->restorePB);

    bc().addReadOnly(dialog_->addPB);
    bc().addReadOnly(dialog_->delPB);
    bc().addReadOnly(dialog_->upPB);
    bc().addReadOnly(dialog_->downPB);
    bc().addReadOnly(dialog_->citationStyleCO);
    bc().addReadOnly(dialog_->textBeforeED);
    bc().addReadOnly(dialog_->textAfterED);

    bc().refresh();
}	

#if K
ButtonPolicy::SMInput FormCitation::input(FL_OBJECT * ob, long)
{
    ButtonPolicy::SMInput activate = ButtonPolicy::SMI_NOOP;
    
    biblio::InfoMap const & theMap = controller().bibkeysInfo();
    
    if (ob == dialog_->browser_bib) {
	fl_deselect_browser(dialog_->browser_cite);
	
	unsigned int const sel = fl_get_browser(dialog_->browser_bib);
	if (sel < 1 || sel > bibkeys.size())
	    return ButtonPolicy::SMI_NOOP;
	
	// Put into browser_info the additional info associated with
	// the selected browser_bib key
	fl_clear_browser(dialog_->browser_info);
	
	string const tmp = formatted(biblio::getInfo(theMap,
						     bibkeys[sel-1]),
				     dialog_->browser_info->w-10 );
	fl_add_browser_line(dialog_->browser_info, tmp.c_str());

	// Highlight the selected browser_bib key in browser_cite if
	// present
	vector<string>::const_iterator cit =
	    find(citekeys.begin(), citekeys.end(), bibkeys[sel-1]);
	
	if (cit != citekeys.end()) {
	    int const n = int(cit - citekeys.begin());
	    fl_select_browser_line(dialog_->browser_cite, n+1);
	    fl_set_browser_topline(dialog_->browser_cite, n+1);
	}
	
	if (!controller().isReadonly()) {
	    if (cit != citekeys.end()) {
		setBibButtons(OFF);
		setCiteButtons(ON);
	    } else {
		setBibButtons(ON);
		setCiteButtons(OFF);
	    }
	}
	
    } else if (ob == dialog_->browser_cite) {
	unsigned int const sel = fl_get_browser(dialog_->browser_cite);
	if (sel < 1 || sel > citekeys.size())
	    return ButtonPolicy::SMI_NOOP;
	
	if (!controller().isReadonly()) {
	    setBibButtons(OFF);
	    setCiteButtons(ON);
	}
	
	// Highlight the selected browser_cite key in browser_bib
	vector<string>::const_iterator cit =
	    find(bibkeys.begin(), bibkeys.end(), citekeys[sel-1]);
	
	if (cit != bibkeys.end()) {
	    int const n = int(cit - bibkeys.begin());
	    fl_select_browser_line(dialog_->browser_bib, n+1);
	    fl_set_browser_topline(dialog_->browser_bib, n+1);
	    
	    // Put into browser_info the additional info associated
	    // with the selected browser_cite key
	    fl_clear_browser(dialog_->browser_info);
	    string const tmp =
		formatted(biblio::getInfo(theMap,
					  bibkeys[sel-1]),
			  dialog_->browser_info->w-10);
	    fl_add_browser_line(dialog_->browser_info, tmp.c_str());
	}

    } else if (ob == dialog_->button_add) {
	unsigned int const sel = fl_get_browser(dialog_->browser_bib);
	if (sel < 1 || sel > bibkeys.size())
	    return ButtonPolicy::SMI_NOOP;
	
	// Add the selected browser_bib key to browser_cite
	fl_addto_browser(dialog_->browser_cite,
			 bibkeys[sel-1].c_str());
	citekeys.push_back(bibkeys[sel-1]);

	int const n = int(citekeys.size());
	fl_select_browser_line(dialog_->browser_cite, n);
	
	setBibButtons(OFF);
	setCiteButtons(ON);
	activate = ButtonPolicy::SMI_VALID;
	
    } else if (ob == dialog_->button_del) {
	unsigned int const sel = fl_get_browser(dialog_->browser_cite);
	if (sel < 1 || sel > citekeys.size())
	    return ButtonPolicy::SMI_NOOP;
	
	// Remove the selected key from browser_cite
	fl_delete_browser_line(dialog_->browser_cite, sel) ;
	citekeys.erase(citekeys.begin() + sel-1);
	
	setBibButtons(ON);
	setCiteButtons(OFF);
	activate = ButtonPolicy::SMI_VALID;
	
    } else if (ob == dialog_->button_up) {
	unsigned int const sel = fl_get_browser(dialog_->browser_cite);
	if (sel < 2 || sel > citekeys.size())
	    return ButtonPolicy::SMI_NOOP;
	
	// Move the selected key up one line
	vector<string>::iterator it = citekeys.begin() + sel-1;
	string const tmp = *it;
	
	fl_delete_browser_line(dialog_->browser_cite, sel);
	citekeys.erase(it);
	
	fl_insert_browser_line(dialog_->browser_cite, sel-1, tmp.c_str());
	fl_select_browser_line(dialog_->browser_cite, sel-1);
	citekeys.insert(it-1, tmp);
	setCiteButtons(ON);
	activate = ButtonPolicy::SMI_VALID;
	
    } else if (ob == dialog_->button_down) {
	unsigned int const sel = fl_get_browser(dialog_->browser_cite);
	if (sel < 1 || sel > citekeys.size()-1)
	    return ButtonPolicy::SMI_NOOP;
	
	// Move the selected key down one line
	vector<string>::iterator it = citekeys.begin() + sel-1;
	string const tmp = *it;
	
	fl_delete_browser_line(dialog_->browser_cite, sel);
	citekeys.erase(it);
	
	fl_insert_browser_line(dialog_->browser_cite, sel+1, tmp.c_str());
	fl_select_browser_line(dialog_->browser_cite, sel+1);
	citekeys.insert(it+1, tmp);
	setCiteButtons(ON);
	activate = ButtonPolicy::SMI_VALID;
	
    } else if (ob == dialog_->button_search_type) {
	if (fl_get_button(dialog_->button_search_type))
	    fl_set_object_label(dialog_->button_search_type,
				_("Regex"));
	else
	    fl_set_object_label(dialog_->button_search_type,
				_("Simple"));
	return ButtonPolicy::SMI_NOOP;
	
    } else if (ob == dialog_->button_previous ||
	       ob == dialog_->button_next) {
	
	string const str = fl_get_input(dialog_->input_search);
	
	biblio::Direction const dir =
	    (ob == dialog_->button_previous) ?
	    biblio::BACKWARD : biblio::FORWARD;
	
	biblio::Search const type =
	    fl_get_button(dialog_->button_search_type) ?
	    biblio::REGEX : biblio::SIMPLE;
	
	vector<string>::const_iterator start = bibkeys.begin();
	int const sel = fl_get_browser(dialog_->browser_bib);
	if (sel >= 1 && sel <= int(bibkeys.size()))
	    start += sel-1;
	
	// Find the NEXT instance...
	if (dir == biblio::FORWARD)
	    start += 1;
	else
	    start -= 1;
	
	vector<string>::const_iterator const cit =
	    biblio::searchKeys(theMap, bibkeys, str,
			       start, type, dir);
	
	if (cit == bibkeys.end())
	    return ButtonPolicy::SMI_NOOP;
	
	int const found = int(cit - bibkeys.begin()) + 1;
	if (found == sel)
	    return ButtonPolicy::SMI_NOOP;
	
	// Update the display
	int const top = max(found-5, 1);
	fl_set_browser_topline(dialog_->browser_bib, top);
	fl_select_browser_line(dialog_->browser_bib, found);
	input(dialog_->browser_bib, 0);
	
    } else if (ob == dialog_->choice_style ||
	       ob == dialog_->input_before ||
	       ob == dialog_->input_after) {
	activate = ButtonPolicy::SMI_VALID;
    }
    
    return activate;
}
#endif


void FormCitation::update()
{
    // Make the list of all available bibliography keys
    bibkeys = biblio::getKeys(controller().bibkeysInfo());
    updateBrowser(dialog_->bibLB, bibkeys);
    
    // Ditto for the keys cited in this inset
    citekeys = getVectorFromString(controller().params().getContents());
    updateBrowser(dialog_->citeLB, citekeys);

    // No keys have been selected yet, so...
    dialog_->infoML->clear();
    setBibButtons(OFF);
    setCiteButtons(OFF);

    int noKeys = int(max(bibkeys.size(), citekeys.size()));

    // Place bounds, so that 4 <= noKeys <= 10
    noKeys = max(4, min(10, noKeys));

    dialog_->textAfterED->setText( controller().params().getOptions().c_str());
}


void FormCitation::updateBrowser( QListBox* browser,
				  vector<string> const & keys) const
{
    browser->clear();

    for (vector<string>::const_iterator it = keys.begin();
	 it < keys.end(); ++it) {
	string key = frontStrip(strip(*it));
	browser->insertItem( key.c_str() );
    }
}


void FormCitation::setBibButtons(State status) const
{
    dialog_->addPB->setEnabled( (status == ON) );
}


void FormCitation::setCiteButtons(State status) const
{
    int const sel     = dialog_->citeLB->currentItem();
    int const maxline = dialog_->citeLB->count()-1;
    bool const activate      = (status == ON);
    bool const activate_up   = (activate && sel != 0);
    bool const activate_down = (activate && sel != maxline);

    dialog_->delPB->setEnabled( activate );
    dialog_->upPB->setEnabled( activate_up );
    dialog_->downPB->setEnabled( activate_down );
}


