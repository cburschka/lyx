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
	dispatch_result dispatch(FuncRequest const & cmd, idx_type & idx, pos_type & pos);
	///
	string const screenLabel() const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	virtual RefInset * asRefInset() { return this; }

	/// plain ascii output
	int ascii(std::ostream & os, int) const;
	/// linuxdoc output
	int linuxdoc(std::ostream & os) const;
	/// docbook output
	int docbook(std::ostream & os, bool) const;

	/// small wrapper for the time being
	dispatch_result localDispatch(FuncRequest const & cmd);

	struct ref_type_info {
		///
		string latex_name;
		///
		string gui_name;
		///
		string short_gui_name;
	};
	static ref_type_info types[];
	///
	static int getType(string const & name);
	///
	static string const & getName(int type);
};

#endif
