// -*- C++ -*-
/**
 * \file ispell.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 */

#ifndef SP_ISPELL_H
#define SP_ISPELL_H

#include "SpellBase.h"

#include <boost/scoped_ptr.hpp>

#include <cstdio>

class BufferParams;
class ForkedProcess;

/// i/a spell process-based spellchecker
class ISpell : public SpellBase {
public:
	ISpell(BufferParams const & params, string const & lang);

	~ISpell();

	/// return true if the spellchecker instance still exists
	virtual bool alive();

	/// check the given word and return the result
	virtual enum Result check(WordLangTuple const & word);

	/// insert the given word into the personal dictionary
	virtual void insert(WordLangTuple const & word);

	/// accept the given word temporarily
	virtual void accept(WordLangTuple const & word);

	/// return the next near miss after a MISSED result
	virtual string const nextMiss();

	/// give an error message on messy exit
	virtual string const error();

private:
	/// read some data. Returns true on an error. Sets err_read
	/// to true if the data was from stderr.
	bool select(bool & err_read);

	/// instream to communicate with ispell
	FILE * in;
	/// outstream to communicate with ispell
	FILE * out;
	/// errstream for ispell
	FILE * inerr;

	/// pipe fds
	int pipein[2];
	int pipeout[2];
	int pipeerr[2];

	/// buffer for reading
	char buf[BUFSIZ];

	/// spell error
	string error_;

	boost::scoped_ptr<ForkedProcess> child_;

	// vileness below ... please FIXME
	/// str ???
	char * str;
	/// e ???
	char * e;

};

#endif // ISPELL_H
