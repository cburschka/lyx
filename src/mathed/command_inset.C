
#include "command_inset.h"
#include "math_mathmlstream.h"
#include "funcrequest.h"
#include "Lsstream.h"


CommandInset::CommandInset(string const & name)
	: name_(name)
{
	lock_ = true;
}


MathInset * CommandInset::clone() const
{
	return new CommandInset(*this);
}


dispatch_result
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
	os << '\\' << name_.c_str();
	if (cell(1).size())
		os << '[' << cell(1) << ']';
	os << '{' << cell(0) << '}';
}


string CommandInset::screenLabel() const
{
	return name_;
}


string const CommandInset::createDialogStr(string const & name) const
{
	ostringstream data;
	data << name << " LatexCommand ";
	WriteStream wsdata(data);
	write(wsdata);
	wsdata << "\n\\end_inset\n\n";

	return data.str();
}


