/**
 * \file Thesaurus.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include "Thesaurus.h"

Thesaurus thesaurus; 

#ifdef HAVE_LIBAIKSAURUS
 
Thesaurus::ThesaurusEntry::ThesaurusEntry(string const & ent, char part)
	: entry(ent), pos(Thesaurus::NONE)
{
	if (part & AikSaurus::Unknown) pos |= OTHER;
	if (part & AikSaurus::Other) pos |= OTHER;
	if (part & AikSaurus::Noun) pos |= NOUN;
	if (part & AikSaurus::Verb) pos |= VERB;
	if (part & AikSaurus::Adjective) pos |= ADJECTIVE;
	if (part & AikSaurus::Adverb) pos |= ADVERB;
}


Thesaurus::Thesaurus()
{
	aik_ = new AikSaurus();
}


Thesaurus::~Thesaurus()
{
	delete aik_;
}


std::vector<Thesaurus::ThesaurusEntry> Thesaurus::lookup(string const & text)
{
	std::vector<ThesaurusEntry> entries;

	if (!aik_->find(text.c_str()))
		return entries;

	char pos;
	string ret;

	ret = aik_->next(pos);
	while (!ret.empty()) {
		entries.push_back(ThesaurusEntry(ret, pos));
		ret = aik_->next(pos);
	}

	return entries;
}

#else

Thesaurus::ThesaurusEntry::ThesaurusEntry(string const &, char)
{
}

 
Thesaurus::Thesaurus()
{
}
 
 
Thesaurus::~Thesaurus()
{
}

 
std::vector<Thesaurus::ThesaurusEntry> Thesaurus::lookup(string const & text)
{
	return std::vector<ThesaurusEntry>();
}

#endif // HAVE_LIBAIKSAURUS
