/**
 * \file FormVCLog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMVCLOG_H
#define FORMVCLOG_H

#include "KFormBase.h"

class ControlVCLog;
class VCLogDialog;

class FormVCLog : public KFormBase<ControlVCLog, VCLogDialog> {
public: 
	FormVCLog(ControlVCLog & c); 

	/// update the dialog
	virtual void update();

	/// apply
	virtual void apply() {};

	/// build dialog
	virtual void build(); 
};

#endif // FORMVCLOG_H
