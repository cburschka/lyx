// -*- C++ -*-
/**
 * \file QSpellchecker.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#ifndef QSPELLCHECKER_H
#define QSPELLCHECKER_H

#include "Qt2Base.h"

 
class ControlSpellchecker; 
class QSpellcheckerDialog;

class QSpellchecker :
	public Qt2CB<ControlSpellchecker, Qt2DB<QSpellcheckerDialog> > 
{
	friend class QSpellcheckerDialog;
 
public: 
	QSpellchecker(ControlSpellchecker &);

	/// update from controller 
	void partialUpdate(int id);
 
private:
	void stop(); 
	void accept();
	void add();
	void ignore();
	void replace();
	void options();
	void spellcheck();
 
	/// Apply changes
	virtual void apply() {};
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QSPELLCHECKER_H
