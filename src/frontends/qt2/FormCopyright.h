/***************************************************************************
                          formcopyright.h  -  description
                             -------------------
    begin                : Thu Feb 3 2000
    copyright            : (C) 2000 by Jürgen Vigna, 2001 by Kalle Dalheimer
    email                : kalle@klaralvdalens-datakonsult.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FORMCOPYRIGHT_H
#define FORMCOPYRIGHT_H

#include "Qt2Base.h"
#include "boost/utility.hpp"

class Dialogs;
class LyXView;
class FormCopyrightDialogImpl;
class ControlCopyright;

/**
  @author Kalle Dalheimer
  */
class FormCopyright 
  : public Qt2CB<ControlCopyright, Qt2DB<FormCopyrightDialogImpl> >
{
public: 
	FormCopyright( ControlCopyright& );

private: 
	/// not needed
	virtual void apply() {}
	/// not needed
	virtual void update() {}
	// build the dialog
	virtual void build();
};

#endif
