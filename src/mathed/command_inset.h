#ifndef COMMAND_INSET_H
#define COMMAND_INSET_H

#include "button_inset.h"

// for things like \name[options]{contents}
class CommandInset : public ButtonInset {
public:
	/// name, contents, options deliminited by '|++|'
	explicit CommandInset(string const & data);
	///
	MathInset * clone() const;
	///
	void write(WriteStream & os) const;
	///
	//void infoize(std::ostream & os) const;
	///
	//int dispatch(string const & cmd, idx_type idx, pos_type pos);
	///
	string screenLabel() const;
public:
	string name_;
};

#endif
