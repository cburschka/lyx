// -*- C++ -*-
/**
 * \file Thesaurus.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef THESAURUS_H
#define THESAURUS_H
 
#include <vector>
 
#include "LString.h"
#ifdef HAVE_LIBAIKSAURUS
#include "AikSaurus.h"
#endif
 
/**
 * This class provides an interface to whatever thesauri we might support.
 */

class Thesaurus {
public:
	///
	Thesaurus();
	///
	~Thesaurus();

	/**
	 * enum of possible part of speech types
	 */
	enum POS {
		NONE = 0x0,
		OTHER = 0x01,
		NOUN = 0x02,
		VERB = 0x04,
		ADJECTIVE = 0x08,
		ADVERB = 0x10
	};

	/**
	 * an individual entry from the thesaurus 
	 */
	struct ThesaurusEntry {
		/// 
		ThesaurusEntry(const string & ent, char pos); 
		/// the actual entry 
		string entry;
		/// entry's part of speech
		int pos;
	};

	/**
	 * look up some text in the thesaurus
	 */
	std::vector<ThesaurusEntry> lookup(string const & text);

private:
#ifdef HAVE_LIBAIKSAURUS
	AikSaurus * aik_;
#endif
};
 
extern Thesaurus thesaurus;
 
#endif
