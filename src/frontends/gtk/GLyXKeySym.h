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

#include "frontends/LyXKeySym.h"

class GLyXKeySym : public LyXKeySym
{
public:
	GLyXKeySym();
	GLyXKeySym(unsigned int keyval);
	void setKeyval(unsigned int keyval);
	unsigned int getKeyval() const { return keyval_; }
	virtual void init(std::string const & symbolname);
	virtual ~GLyXKeySym() {}
	virtual bool isOK() const;
	virtual bool isModifier() const;
	virtual std::string getSymbolName() const;
	virtual char getISOEncoded(std::string const & encoding) const;
	virtual std::string const print(key_modifier::state mod) const;
private:
	unsigned int keyval_;
};

#endif
