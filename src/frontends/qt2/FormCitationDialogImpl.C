/**
 * $Id: FormCitationDialogImpl.C,v 1.7 2001/06/05 17:05:51 lasgouttes Exp $
 */

#include <config.h>
 
#include "FormCitationDialogImpl.h"
#include "Dialogs.h"
#include "FormCitation.h"
#include "qt2BC.h"
#include "controllers/biblio.h"
#include "controllers/ControlCitation.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#undef emit

#include <algorithm>
#include "LyXView.h"
#include "buffer.h"

using std::vector;
using std::find;

/*
 *  Constructs a FormCitationDialogImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
FormCitationDialogImpl::FormCitationDialogImpl( FormCitation* form, QWidget* parent,  const char* name, bool modal, WFlags fl )
  : FormCitationDialog( parent, name, modal, fl ),
    form_( form )
{
    connect( okPB, SIGNAL( clicked() ),
	     form, SLOT( slotOK() ) );
    connect( cancelPB, SIGNAL( clicked() ),
	     form, SLOT( slotCancel() ) );
    connect( restorePB, SIGNAL( clicked() ),
	     form, SLOT( slotRestore() ) );
    connect( applyPB, SIGNAL( clicked() ),
	     form, SLOT( slotApply() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
FormCitationDialogImpl::~FormCitationDialogImpl()
{
    // no need to delete child widgets, Qt does it all for us
}


// These slots correspond to the XForms input() method.
void FormCitationDialogImpl::slotBibSelected( int sel )
{
    biblio::InfoMap const & theMap = form_->controller().bibkeysInfo();

    citeLB->clearSelection();

    if (sel < 0 || sel >= (int)form_->bibkeys.size()) {
      //	form_->bc().input( ButtonPolicy::SMI_NOOP );
	return;
    }

    // Put into browser_info the additional info associated with
    // the selected browser_bib key
    infoML->clear();

    infoML->setText( biblio::getInfo( theMap,
				      form_->bibkeys[sel-1] ).c_str() );

    // Highlight the selected browser_bib key in browser_cite if
    // present
    vector<string>::const_iterator cit =
	std::find(form_->citekeys.begin(), form_->citekeys.end(),
		  form_->bibkeys[sel]);

    if (cit != form_->citekeys.end()) {
	int const n = int(cit - form_->citekeys.begin());
	citeLB->setSelected( n, true );
	citeLB->setTopItem( n );
    }

    if (!form_->controller().isReadonly()) {
	if (cit != form_->citekeys.end()) {
	    form_->setBibButtons(FormCitation::OFF);
	    form_->setCiteButtons(FormCitation::ON);
	} else {
	    form_->setBibButtons(FormCitation::ON);
	    form_->setCiteButtons(FormCitation::OFF);
	}
    }

    //    form_->bc().input( ButtonPolicy::SMI_VALID );
}


void FormCitationDialogImpl::slotCiteSelected( int sel )
{
    biblio::InfoMap const & theMap = form_->controller().bibkeysInfo();

    if (sel < 0 || sel >= (int)form_->citekeys.size()) {
      //	form_->bc().input( ButtonPolicy::SMI_NOOP );
	return;
    }

    if (!form_->controller().isReadonly()) {
	form_->setBibButtons(FormCitation::OFF);
	form_->setCiteButtons(FormCitation::ON);
    }

    // Highlight the selected browser_cite key in browser_bib
    vector<string>::const_iterator cit =
	std::find(form_->bibkeys.begin(), form_->bibkeys.end(), form_->citekeys[sel]);

    if (cit != form_->bibkeys.end()) {
	int const n = int(cit - form_->bibkeys.begin());
	bibLB->setSelected( n, true );
	bibLB->setTopItem( n );

	// Put into browser_info the additional info associated
	// with the selected browser_cite key
	infoML->clear();
	infoML->setText( biblio::getInfo( theMap, form_->bibkeys[sel] ).c_str() );
    }

    //    form_->bc().input( ButtonPolicy::SMI_VALID );
}


void FormCitationDialogImpl::slotAddClicked()
{
    int const sel = bibLB->currentItem();
    if (sel < 0 || sel >= (int)form_->bibkeys.size()) {
      //	form_->bc().input( ButtonPolicy::SMI_NOOP );
	return;
    }

    // Add the selected browser_bib key to browser_cite
    citeLB->insertItem( form_->bibkeys[sel].c_str() );
    form_->citekeys.push_back( form_->bibkeys[sel] );

    int const n = int(form_->citekeys.size());
    citeLB->setSelected( n-1, true );

    form_->setBibButtons(FormCitation::OFF);
    form_->setCiteButtons(FormCitation::ON);

    //    form_->bc().input( ButtonPolicy::SMI_VALID );
}


void FormCitationDialogImpl::slotDelClicked()
{
    int const sel = citeLB->currentItem();
    if (sel < 0 || sel >= (int)form_->citekeys.size()) {
      //	form_->bc().input( ButtonPolicy::SMI_NOOP );
	return;
    }

    // Remove the selected key from browser_cite
    citeLB->removeItem( sel );
    form_->citekeys.erase(form_->citekeys.begin() + sel );

    form_->setBibButtons(FormCitation::ON);
    form_->setCiteButtons(FormCitation::OFF);

    //    form_->bc().input( ButtonPolicy::SMI_VALID );
}


void FormCitationDialogImpl::slotUpClicked()
{
    int const sel = citeLB->currentItem();
    if (sel < 1 || sel >= (int)form_->citekeys.size()) {
      //	form_->bc().input( ButtonPolicy::SMI_NOOP );
	return;
    }

    // Move the selected key up one line
    vector<string>::iterator it = form_->citekeys.begin() + sel;
    string const tmp = *it;

    citeLB->removeItem( sel );
    form_->citekeys.erase(it);

    citeLB->insertItem( tmp.c_str(), sel-1 );
    citeLB->setSelected( sel-1, true );
    form_->citekeys.insert(it-1, tmp);
    form_->setCiteButtons(FormCitation::ON);

    //    form_->bc().input( ButtonPolicy::SMI_VALID );
}


void FormCitationDialogImpl::slotDownClicked()
{
    int const sel = citeLB->currentItem();
    if (sel < 0 || sel >= (int)form_->citekeys.size()-1) {
      //	form_->bc().input( ButtonPolicy::SMI_NOOP );
	return;
    }

    // Move the selected key down one line
    vector<string>::iterator it = form_->citekeys.begin() + sel;
    string const tmp = *it;

    citeLB->removeItem( sel );
    form_->citekeys.erase(it);

    citeLB->insertItem( tmp.c_str(), sel+1 );
    citeLB->setSelected( sel+1, true );
    form_->citekeys.insert(it+1, tmp);
    form_->setCiteButtons(FormCitation::ON);

    //    form_->bc().input( ButtonPolicy::SMI_VALID );
}


void FormCitationDialogImpl::slotPreviousClicked()
{
    doPreviousNext( false );
}


void FormCitationDialogImpl::slotNextClicked()
{
    doPreviousNext( true );
}


void FormCitationDialogImpl::doPreviousNext( bool next )
{
    biblio::InfoMap const & theMap = form_->controller().bibkeysInfo();
    string const str = searchED->text().latin1();

    biblio::Direction const dir =
	next ?
	biblio::FORWARD : biblio::BACKWARD;

    biblio::Search const type =
	searchTypeCB->isChecked() ?
	biblio::REGEX : biblio::SIMPLE;

    vector<string>::const_iterator start = form_->bibkeys.begin();
    int const sel = bibLB->currentItem();
    if (sel >= 0 && sel <= int(form_->bibkeys.size()-1))
	start += sel;

    // Find the NEXT instance...
    if (dir == biblio::FORWARD)
	start += 1;
    else
	start -= 1;

    bool const caseSensitive = searchCaseCB->isChecked();
    
    vector<string>::const_iterator const cit =
	biblio::searchKeys(theMap, form_->bibkeys, str,
			   start, type, dir, caseSensitive );

    if (cit == form_->bibkeys.end()) {
      //	form_->bc().input( ButtonPolicy::SMI_NOOP );
	return;
    }

    int const found = int(cit - form_->bibkeys.begin());
    if (found == sel) {
      //	form_->bc().input( ButtonPolicy::SMI_NOOP );
	return;
    }

    // Update the display
    int const top = max(found-5, 1);
    bibLB->setTopItem( top );
    bibLB->setSelected( found, true );
    slotBibSelected( 0 );

    //    form_->bc().input( ButtonPolicy::SMI_VALID );
}


void FormCitationDialogImpl::slotCitationStyleSelected( int )
{
  //    form_->bc().input( ButtonPolicy::SMI_VALID );
}


void FormCitationDialogImpl::slotTextBeforeReturn()
{
  //    form_->bc().input( ButtonPolicy::SMI_VALID );
}


void FormCitationDialogImpl::slotTextAfterReturn()
{
  //    form_->bc().input( ButtonPolicy::SMI_VALID );
}

