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
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
*/

#ifndef FORMCITATION_H
#define FORMCITATION_H

#ifdef __GNUG__
#pragma interface
#endif

class QListBox;

#include "Qt2Base.h"
#undef emit

/** This class provides a Qt2 implementation of the Citation Dialog.
    @author Kalle Dalheimer
*/
class ControlCitation;
class FormCitationDialogImpl;

class FormCitation : public Qt2CB<ControlCitation, Qt2DB<FormCitationDialogImpl> > {
    friend class FormCitationDialogImpl;
    
public:
    ///
    FormCitation(ControlCitation &);

private:
    ///
    enum State {
	///
	ON,
	///
	OFF
    };

    /// Set the Params variable for the Controller.
    virtual void apply();
    /// Build the dialog.
    virtual void build();
    /// Hide the dialog.
    virtual void hide();
    /// Update dialog before/whilst showing it.
    virtual void update();
    // 	/// Filter the inputs on callback from xforms
    // 	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

    void updateBrowser(QListBox*, std::vector<string> const &) const;
    ///
    void setBibButtons(State) const;
    ///
    void setCiteButtons(State) const;

    ///
    std::vector<string> citekeys;
    ///
    std::vector<string> bibkeys;
};

#endif // FORMCITATION_H
