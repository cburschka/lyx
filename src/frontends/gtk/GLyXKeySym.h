// -*- C++ -*-
/**
 * \file GLyXKeySym.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GLYX_KEYSYM_H
#define GLYX_KEYSYM_H

#include "LString.h"
#include "frontends/LyXKeySym.h"

class GLyXKeySym : public LyXKeySym
{
public:
	GLyXKeySym();
	GLyXKeySym(unsigned int keyval);
	void setKeyval(unsigned int keyval);
	unsigned int getKeyval() const { return keyval_; }
	virtual void init(string const & symbolname);
	virtual ~GLyXKeySym() {}
	virtual bool isOK() const;
	virtual bool isModifier() const;
	virtual string getSymbolName() const;
	virtual char getISOEncoded(string const & encoding) const;
private:
	unsigned int keyval_;
};

#endif
