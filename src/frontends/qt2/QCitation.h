// -*- C++ -*-
/**
 * \file QCitation.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#ifndef QCITATION_H
#define QCITATION_H

#ifdef __GNUG__
#pragma interface
#endif

class QListBox;

#include "Qt2Base.h"

class ControlCitation;
class QCitationDialog;

class QCitation : public Qt2CB<ControlCitation, Qt2DB<QCitationDialog> > 
{
	friend class QCitationDialog;
    
public:
	///
	QCitation(ControlCitation &);

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

	void updateBrowser(QListBox *, std::vector<string> const &) const;
	///
	void setBibButtons(State) const;
	///
	void setCiteButtons(State) const;

	///
	std::vector<string> citekeys;
	///
	std::vector<string> bibkeys;
};

#endif // QCITATION_H
