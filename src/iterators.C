#include <config.h>

#include "iterators.h"

ParIterator  & ParIterator::operator++()
{
	while (!positions.empty()) {
		ParPosition & p = positions.back();

		// Does the current inset contain more "cells" ?
		if (p.index >= 0) {
			++p.index;
			Paragraph * par = (*p.it)->getFirstParagraph(p.index);
			if (par) {
				positions.push_back(ParPosition(par));
				return *this;
			}
			++p.it;
		}

		// Try to find the next inset that contains paragraphs
		for ( ; p.it != p.par->inset_iterator_end(); ++p.it) {
			Paragraph * par = (*p.it)->getFirstParagraph(0);
			if (par) {
				p.index = 0;
				positions.push_back(ParPosition(par));
				return *this;
			}
		}
		// Try to go to the next paragarph
		if (p.par->next()) {
			p = ParPosition(p.par->next());
			return *this;
		}

		positions.pop_back();
	}
	return *this;
}
