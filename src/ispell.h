// -*- C++ -*-
/**
 * \file sp_ispell.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 */

#ifndef SP_ISPELL_H
#define SP_ISPELL_H

#include <cstdio>

#include "SpellBase.h"

class BufferParams;

/// i/a spell process-based spellchecker
class ISpell : public SpellBase {
public:
	ISpell(BufferParams const & params, string const & lang);

	~ISpell();

	/// return true if the spellchecker instance still exists
	virtual bool alive();

	/// clean up on messy exit
	virtual void cleanUp();

	/// check the given word and return the result
	virtual enum Result check(WordLangTuple const & word);

	/// finish this spellchecker instance
	virtual void close();

	/// insert the given word into the personal dictionary
	virtual void insert(WordLangTuple const & word);

	/// accept the given word temporarily
	virtual void accept(WordLangTuple const & word);

	/// return the next near miss after a MISSED result
	virtual string const nextMiss();

	/// give an error message on messy exit 
	virtual string const error();

private:
	/// instream to communicate with ispell
	FILE * in;
	/// outstream to communicate with ispell
	FILE * out;
	/// spell error
	char const * error_;
	/// the fd of the outgoing pipe
	int isp_fd;

	// vileness below ... please FIXME
	/// str ???
	char * str;
	/// e ???
	char * e;

};

#endif // ISPELL_H
