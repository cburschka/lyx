/**
 * \file FormCopyright.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Jürgen Vigna
 * \author John Levon
 */
#ifndef FORMCOPYRIGHT_H
#define FORMCOPYRIGHT_H

#include "KFormBase.h"

class ControlCopyright;
class CopyrightDialog;

class FormCopyright : public KFormBase<ControlCopyright, CopyrightDialog> {
public: 
	/// 
	FormCopyright(ControlCopyright & c);

private: 
	/// build the dialog
	virtual void build();
	/// not used 
	virtual void apply() {}
	/// not used
	void update() {}
};

#endif // FORMCOPYRIGHT_H
