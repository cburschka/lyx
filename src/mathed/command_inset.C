
#include "command_inset.h"
#include "math_mathmlstream.h"
#include "funcrequest.h"


CommandInset::CommandInset(string const & data)
{
	lock_ = true;

	string::size_type idx0 = data.find("|++|");
	name_ = data.substr(0, idx0);
	if (idx0 == string::npos)
		return;
	idx0 += 4;
	string::size_type idx1 = data.find("|++|", idx0);
	cell(0) = asArray(data.substr(idx0, idx1 - idx0));
	if (idx1 == string::npos)
		return;
	cell(1) = asArray(data.substr(idx1 + 4));
}


MathInset * CommandInset::clone() const
{
	return new CommandInset(*this);
}


MathInset::result_type
CommandInset::dispatch(FuncRequest const & cmd, idx_type & idx, pos_type & pos)
{
	switch (cmd.action) {
		default:
			return ButtonInset::dispatch(cmd, idx, pos);
	}
	return UNDISPATCHED;
}


void CommandInset::write(WriteStream & os) const
{
	os << "\\" << name_.c_str();
	if (cell(1).size())
		os << "[" << cell(1) << "]";
	os << "{" << cell(0) << "}";
}


string CommandInset::screenLabel() const
{
	return name_;
}
