/**
 * \file FormPrint.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMPRINT_H
#define FORMPRINT_H

#include "KFormBase.h"

class ControlPrint; 
class PrintDialog;

///
class FormPrint : public KFormBase<ControlPrint, PrintDialog> {
public:
	///
	FormPrint(ControlPrint & c);

private: 
	/// apply dialog 
	virtual void apply(); 
	/// build dialog
	virtual void build();
	/// update dialog
	virtual void update();
};

#endif // FORMPRINT_H
