// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlCitation.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "ControlCitation.h"
#include "buffer.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "helper_funcs.h"
#include "support/lstrings.h"
#include "support/LRegex.h"

using std::find;
using std::min;
using std::pair;
using std::vector;
using std::sort;
using SigC::slot;

ControlCitation::ControlCitation(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_CITATION_INSERT)
{
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d_.showCitation.connect(slot(this, &ControlCitation::showInset));
	d_.createCitation.connect(slot(this, &ControlCitation::createInset));
}


void ControlCitation::clearDaughterParams()
{
	bibkeysInfo_.clear();
}


// A functor for use with std::sort, leading to case insensitive sorting
struct compareNoCase: public std::binary_function<string, string, bool> 
{
	bool operator()(string const & s1, string const & s2) const {
		return compare_no_case(s1, s2) < 0;
	}
};


vector<string> const ControlCitation::getBibkeys()
{
	vector<string> bibkeys;
	bibkeysInfo_.clear();

	vector<pair<string,string> > blist = lv_.buffer()->getBibkeyList();

	for (vector<pair<string,string> >::size_type i=0; i<blist.size(); ++i) {
		bibkeys.push_back(blist[i].first);
		bibkeysInfo_.insert(InfoMapValue(blist[i].first,
						 blist[i].second));
	}

	sort(bibkeys.begin(), bibkeys.end(), compareNoCase());
	return bibkeys;
}


string const ControlCitation::getBibkeyInfo(string const & key)
{
	Assert(!bibkeysInfo_.empty());

	string result;

       	InfoMap::const_iterator it = bibkeysInfo_.find(key);
	if (it != bibkeysInfo_.end()) {
		// Search for all possible "required" keys
		string author = parseBibTeX(it->second, "author");
		if (author.empty())
			author = parseBibTeX(it->second, "editor");

		string year       = parseBibTeX(it->second, "year");
		string title      = parseBibTeX(it->second, "title");
		string booktitle  = parseBibTeX(it->second, "booktitle");
		string chapter    = parseBibTeX(it->second, "chapter");
		string pages      = parseBibTeX(it->second, "pages");

		string media      = parseBibTeX(it->second, "journal");
		if (media.empty())
			media = parseBibTeX(it->second, "publisher");
		if (media.empty())
			media = parseBibTeX(it->second, "school");
		if (media.empty())
			media = parseBibTeX(it->second, "institution");

		result = author;
		if (!year.empty())
			result += ", " + year;
		if (!title.empty())
			result += ", " + title;
		if (!booktitle.empty())
			result += ", in " + booktitle;
		if (!chapter.empty())
			result += ", Ch. " + chapter;
		if (!media.empty())
			result += ", " + media;
		if (!pages.empty())
			result += ", pp. " + pages;

		if (result.empty()) // not a BibTeX record
			result = it->second;
	}

	return result;
}


vector<string>::const_iterator
searchKeys(ControlCitation const & controller,
	   vector<string> const & keys,
	   string const & expr,
	   vector<string>::const_iterator start,
	   ControlCitation::Search type,
	   ControlCitation::Direction dir,
	   bool caseSensitive)
{
	// Preliminary checks
	if(start < keys.begin() || start >= keys.end())
		return keys.end();
	
	string search_expr = frontStrip(strip(expr));
	if (search_expr.empty())
		return start;

	if (type == ControlCitation::SIMPLE)
		return simpleSearch(controller, keys, search_expr, start, dir,
				    caseSensitive);

	return regexSearch(controller, keys, search_expr, start, dir);
}


vector<string>::const_iterator
simpleSearch(ControlCitation const & controller,
	     vector<string> const & keys,
	     string const & expr,
	     vector<string>::const_iterator start,
	     ControlCitation::Direction dir,
	     bool caseSensitive)
{
	vector<string> searchwords = getVectorFromString(expr, " ");

	// Loop over all keys from start...
	for (vector<string>::const_iterator it = start;
	     // End condition is direction-dependent.
	     (dir == ControlCitation::FORWARD) ?
		     (it<keys.end()) : (it>=keys.begin());
	     // increment is direction-dependent.
	     (dir == ControlCitation::FORWARD) ?
		     (++it) : (--it)) {

		string data = (*it);
		ControlCitation::InfoMap::const_iterator info =
			controller.bibkeysInfo().find(*it);
		if (info != controller.bibkeysInfo().end())
			data += " " + info->second;
		if (!caseSensitive)
			data = lowercase(data);

		bool found = true;

		// Loop over all search words...
		if (caseSensitive) {
			for (vector<string>::const_iterator sit=
				     searchwords.begin();
			     sit<searchwords.end(); ++sit) {
				if (data.find(*sit) == string::npos) {
					found = false;
					break;
				}
			}
		} else {
			for (vector<string>::const_iterator sit=
				     searchwords.begin();
			     sit<searchwords.end(); ++sit) {
				if (data.find(lowercase(*sit)) ==
				    string::npos) {
					found = false;
					break;
				}
			}
		}
		
		if (found) return it;
	}

	return keys.end();
}


vector<string>::const_iterator
regexSearch(ControlCitation const & controller,
	    vector<string> const & keys,
	    string const & expr,
	    vector<string>::const_iterator start,
	    ControlCitation::Direction dir)
{
	LRegex reg(expr);

	for (vector<string>::const_iterator it = start;
	     // End condition is direction-dependent.
	     (dir == ControlCitation::FORWARD) ?
		     (it<keys.end()) : (it>=keys.begin());
	     // increment is direction-dependent.
	     (dir == ControlCitation::FORWARD) ?
		     (++it) : (--it)) {

		string data = (*it);
		ControlCitation::InfoMap::const_iterator info =
			controller.bibkeysInfo().find(*it);
		if (info != controller.bibkeysInfo().end())
			data += " " + info->second;

		if (reg.exec(data).size() > 0)
			return it;
	}

	return keys.end();
}

string const parseBibTeX(string data, string const & findkey)
{
	string keyvalue;
	
	for (string::iterator it=data.begin(); it<data.end(); ++it) {
		if ((*it) == '\n' || (*it) == '\t')
		        (*it)= ' ';
	}
	
	data = frontStrip(data);
	while (!data.empty() && data[0] != '=' && 
	       (data.find(' ') != string::npos || data.find('=') != string::npos)) {

		string::size_type keypos = min(data.find(' '), data.find('='));
		string key = lowercase(data.substr(0, keypos));
      
		data = data.substr(keypos, data.length()-1);
		data = frontStrip(strip(data));
		if (data.length() > 1 && data[0]=='=') {
			data = frontStrip(data.substr(1, data.length()-1));
			if (!data.empty()) {
				keypos = 1;
				string value;
				char enclosing;

				if (data[0]=='{') {
					enclosing = '}';
				} else if (data[0]=='"') {
					enclosing = '"';
				} else {
					keypos=0;
					enclosing=' ';
				}

				if (keypos &&
				    data.find(enclosing)!=string::npos &&
				    data.length()>1) {
					string tmp = data.substr(keypos,
								 data.length()-1);
					while (tmp.find('{') != string::npos &&
					       tmp.find('}') != string::npos &&
					       tmp.find('{') < tmp.find('}') &&
					       tmp.find('{') < tmp.find(enclosing)) {

						keypos += tmp.find('{')+1;
						tmp = data.substr(keypos,
								  data.length()-1);
						keypos += tmp.find('}')+1;
						tmp = data.substr(keypos,
								  data.length()-1);
					}

					if (tmp.find(enclosing)==string::npos)
						return keyvalue;
					else {
						keypos += tmp.find(enclosing);
						tmp = data.substr(keypos,
								  data.length()-1);
					}

					value = data.substr(1, keypos-1);

					if (keypos+1<data.length()-1)
						data = frontStrip(data.substr(keypos+1, data.length()-1));
					else
						data = "";

				} else if (!keypos &&
					   (data.find(' ') ||
					    data.find(','))) {
					keypos = data.length()-1;
					if (data.find(' ') != string::npos)
						keypos = data.find(' ');
					if (data.find(',') != string::npos &&
					    keypos > data.find(','))
						keypos = data.find(',');

					value = data.substr(0, keypos);
		  
					if (keypos+1<data.length()-1)
						data = frontStrip(data.substr(keypos+1, data.length()-1));
					else
						data = "";
				}
				else
					return keyvalue;

				if (findkey == key) {
					keyvalue = value;
					return keyvalue;
				} 

				data = frontStrip(frontStrip(data,','));
			}
		}
		else return keyvalue;
	}
	return keyvalue;
}
