/**
 * \file FormTabularCreate.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMTABULARCREATE_H
#define FORMTABULARCREATE_H

#include "KFormBase.h"

class ControlTabularCreate; 
class TabularCreateDialog;

///
class FormTabularCreate : public KFormBase<ControlTabularCreate, TabularCreateDialog> {
public: 
	///
	FormTabularCreate(ControlTabularCreate & c); 
 
private:
	/// apply dialog 
	virtual void apply();
	/// build dialog
	virtual void build();
	/// not used
	virtual void update() {};
};

#endif // FORMTABULARCREATE_H
