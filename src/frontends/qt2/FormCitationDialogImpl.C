/**
 * $Id: FormCitationDialogImpl.C,v 1.3 2001/03/29 18:58:47 kalle Exp $
 */

#include "FormCitationDialogImpl.h"
#include "Dialogs.h"
#include "FormCitation.h"

#include <qlistbox.h>
#include <qmultilineedit.h>
#undef emit

#include <algorithm>
#include "LyXView.h"
#include "buffer.h"

using std::vector;
using std::find;

// PENDING(kalle) Wire text before and citation style

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
}

/*
 *  Destroys the object and frees any allocated resources
 */
FormCitationDialogImpl::~FormCitationDialogImpl()
{
    // no need to delete child widgets, Qt does it all for us
}


#if 0
// These slots correspond to the XForms input() method.
void FormCitationDialogImpl::slotBibSelected( int sel )
{
    insetKeysLB->clearSelection();

    if( sel < 0 || sel > form_->bibkeys.size()-1)
	return;

    // Put into browser_info the additional info associated with
    // the selected browser_bib key
    infoML->clear();
    infoML->append( form_->bibkeysInfo[sel].c_str() );

    // Highlight the selected browser_bib key in browser_cite if present
    vector<string>::iterator it =
	::find(form_->citekeys.begin(), form_->citekeys.end(), form_->bibkeys[sel]);

    if (it != form_->citekeys.end()) {
	int n = static_cast<int>(it - form_->citekeys.begin());
	insetKeysLB->setSelected( n, true );
    }

    if (!form_->lv_->buffer()->isReadonly()) {
	if (it != form_->citekeys.end()) {
	    form_->setBibButtons(FormCitation::OFF);
	    form_->setCiteButtons(FormCitation::ON);
	} else {
	    form_->setBibButtons(FormCitation::ON);
	    form_->setCiteButtons(FormCitation::OFF);
	}
    }

}


void FormCitationDialogImpl::slotInsetSelected( int sel )
{
    if (sel < 0 || sel > form_->citekeys.size() -1 )
	return;

    if (!form_->lv_->buffer()->isReadonly()) {
	form_->setBibButtons(FormCitation::OFF);
	form_->setCiteButtons(FormCitation::ON);
    }

    // Highlight the selected browser_cite key in browser_bib
    vector<string>::iterator it =
	::find(form_->bibkeys.begin(), form_->bibkeys.end(), form_->citekeys[sel]);

    if (it != form_->bibkeys.end()) {
	int n = static_cast<int>(it - form_->bibkeys.begin());
	bibliographyKeysLB->setSelected( n, true );

	// Put into browser_info the additional info associated with
	// the selected browser_cite key
	infoML->clear();
	infoML->append( form_->bibkeysInfo[n].c_str() );
    }
}


void FormCitationDialogImpl::slotAddClicked()
{
    qDebug( "FormCitationDialogImpl::slotAddClicked()" );
    int sel = bibliographyKeysLB->currentItem();
    if (sel < 0 || sel > form_->bibkeys.size() -1 )
	return;

    qDebug( "sel = %d" );
    qDebug( "bibkeys.size() = %d", form_->bibkeys.size() );

    // Add the selected browser_bib key to browser_cite
    insetKeysLB->insertItem( form_->bibkeys[sel].c_str());
    form_->citekeys.push_back(form_->bibkeys[sel]);

    int n = static_cast<int>(form_->citekeys.size());
    insetKeysLB->setSelected( n, true );

    form_->setBibButtons(FormCitation::OFF);
    form_->setCiteButtons(FormCitation::ON);
}


void FormCitationDialogImpl::slotDelClicked()
{
    int sel = insetKeysLB->currentItem();
    if (sel < 0 || sel > form_->citekeys.size()-1)
	return;

    // Remove the selected key from browser_cite
    insetKeysLB->removeItem( sel );
    form_->citekeys.erase(form_->citekeys.begin() + sel);

    form_->setBibButtons(FormCitation::ON);
    form_->setCiteButtons(FormCitation::OFF);
}


void FormCitationDialogImpl::slotUpClicked()
{
    int sel = insetKeysLB->currentItem();
    if (sel < 1 || sel > form_->citekeys.size()-1)
	return;

    // Move the selected key up one line
    vector<string>::iterator it = form_->citekeys.begin() + sel;
    string tmp = *it;

    insetKeysLB->removeItem( sel );
    form_->citekeys.erase(it);

    insetKeysLB->insertItem( tmp.c_str(), sel-1 );
    insetKeysLB->setSelected( sel-1, true );
    form_->citekeys.insert(it-1, tmp);
    form_->setCiteButtons(FormCitation::ON);
}


void FormCitationDialogImpl::slotDownClicked()
{
    int sel = insetKeysLB->currentItem();
    if (sel < 0 || sel > form_->citekeys.size()-2)
	return;

    // Move the selected key down one line
    vector<string>::iterator it = form_->citekeys.begin() + sel;
    string tmp = *it;

    insetKeysLB->removeItem( sel );
    form_->citekeys.erase(it);

    insetKeysLB->insertItem( tmp.c_str(), sel+1 );
    insetKeysLB->setSelected( sel+1, true );
    form_->citekeys.insert(it+1, tmp);
    form_->setCiteButtons(FormCitation::ON);
}


void FormCitationDialogImpl::apply_adaptor()
{
    form_->apply();
}


void FormCitationDialogImpl::close_adaptor()
{
    form_->close();
    hide();
}
#endif
