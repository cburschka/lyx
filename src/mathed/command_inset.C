#include "command_inset.h"
#include "math_mathmlstream.h"
#include "funcrequest.h"
#include "Lsstream.h"

using std::auto_ptr;


CommandInset::CommandInset(string const & name)
	: MathNestInset(2),
	  name_(name),
	  set_label_(false)
{
	lock_ = true;
}


auto_ptr<InsetBase> CommandInset::clone() const
{
	return auto_ptr<InsetBase>(new CommandInset(*this));
}


void CommandInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (!set_label_) {
		set_label_ = true;
		button_.update(screenLabel(), true);
	}
	button_.metrics(mi, dim);
}


void CommandInset::draw(PainterInfo & pi, int x, int y) const
{
	button_.draw(pi, x, y);
}

dispatch_result
CommandInset::dispatch(FuncRequest const & cmd, idx_type & idx, pos_type & pos)
{
	switch (cmd.action) {
		default:
			return MathNestInset::dispatch(cmd, idx, pos);
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


string const CommandInset::screenLabel() const
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
	return STRCONV(data.str());
}
