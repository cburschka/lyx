// -*- C++ -*-

#ifndef TOOLBARDEFAULTS_H
#define TOOLBARDEFAULTS_H

#include <vector>

#include "LString.h"

using std::vector;

class LyXLex;


///
class ToolbarDefaults {
public:
	///
	typedef vector<int> Defaults;
	///
	typedef Defaults::iterator iterator;
	///
	typedef Defaults::const_iterator const_iterator;
	///
	ToolbarDefaults();
	///
	iterator begin() {
		return defaults.begin();
	}
	///
	const_iterator begin() const {
		return defaults.begin();
	}
	///
	iterator end() {
		return defaults.end();
	}
	///
	const_iterator end() const {
		return defaults.end();
	}
	///
	void read(LyXLex &);
private:
	///
	void init();
	/// This func is just to make it easy for me...
	void add(int);
	///
	void add(string const &);
	///
	Defaults defaults;
};

#endif



