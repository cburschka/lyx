/** Collection of some useful xform helper functions
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG_
#pragma implementation
#endif

#include <vector>
#include "xform_helpers.h"

using std::vector;

// Take a string and add breaks so that it fits into a desired label width, w
string formatted( string const & sin, int w, int size, int style )
{
	string sout;
	if (sin.empty() ) return sout;

	// break sin up into a vector of individual words
	vector<string> sentence;
	string word;
	for (string::const_iterator sit = sin.begin();
	     sit != sin.end(); ++sit) {
		if ((*sit) == ' ' || (*sit) == '\n') {
			sentence.push_back(word);
			word.erase();
		} else {
			word += (*sit);
		}
	}
	// Flush remaining contents of word
	if (!word.empty() ) sentence.push_back(word);

	string line, l1;
	for (vector<string>::const_iterator vit = sentence.begin();
	     vit != sentence.end(); ++vit) {
		if (!l1.empty() ) l1 += ' ';
		l1 += (*vit);
		int length = fl_get_string_width(style, size, l1.c_str(),
						 int(l1.length()));
		if (length >= w) {
			if (!sout.empty() ) sout += '\n';
			sout += line;
			l1 = (*vit);
		}

		line = l1;
	}
	// Flush remaining contents of line
	if (!line.empty()) {
		if (!sout.empty() ) sout += '\n';
		sout += line;
	}
	
	return sout;
}
