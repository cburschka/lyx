/**
 * \file Thesaurus.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "Thesaurus.h"

#include <algorithm>
 
Thesaurus thesaurus; 

#ifdef HAVE_LIBAIKSAURUS
 
Thesaurus::Thesaurus()
{
	aik_ = new Aiksaurus;
}


Thesaurus::~Thesaurus()
{
	delete aik_;
}


Thesaurus::Meanings Thesaurus::lookup(string const & text)
{
	Meanings meanings;

	if (!aik_->find(text.c_str()))
		return meanings;

	// weird api, but ...
 
	int prev_meaning = -1;
	int cur_meaning;
	string meaning;

	// correct, returns "" at the end 
	string ret = aik_->next(cur_meaning);
 
	while (!ret.empty()) {
		if (cur_meaning != prev_meaning) {
			meaning = ret;
			ret = aik_->next(cur_meaning);
			prev_meaning = cur_meaning;
		} else {
			if (ret != text) {
				meanings[meaning].push_back(ret);
			} 
		}
 
		ret = aik_->next(cur_meaning);
	}

	for (Meanings::iterator it = meanings.begin();
		it != meanings.end(); ++it) {
			std::sort(it->second.begin(), it->second.end());
	}
 
	return meanings;
}

#else

Thesaurus::Thesaurus()
{
}
 
 
Thesaurus::~Thesaurus()
{
}

 
Thesaurus::Meanings Thesaurus::lookup(string const &)
{
	return Meanings();
}

#endif // HAVE_LIBAIKSAURUS
