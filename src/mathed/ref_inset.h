#ifndef REF_INSET_H
#define REF_INSET_H

#include "command_inset.h"

// for \ref 
class RefInset : public CommandInset {
public:
	///
	RefInset();
	///
	explicit RefInset(string const & data);
	///
	MathInset * clone() const;
	///
	//void write(WriteStream & os) const;
	///
	void infoize(std::ostream & os) const;
	///
	int dispatch(string const & cmd, idx_type idx, pos_type pos);
	///
	string screenLabel() const;
	///
	void validate(LaTeXFeatures & features) const;

	/// plain ascii output
	int ascii(std::ostream & os, int) const;
	/// linuxdoc output
	int linuxdoc(std::ostream & os) const;
	/// docbook output
	int docbook(std::ostream & os, bool) const;


	struct type_info {
		///
		string latex_name;
		///
		string gui_name;
		///
		string short_gui_name;
	};
	static type_info types[];
	///
	static int getType(string const & name);
	///
	static string const & getName(int type);
};

#endif
