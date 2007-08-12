#include "TexStream.h"
#include "TexRow.h"

#include <iostream>
#include <streambuf>

namespace lyx {

////////////////////////////////////////////////////////////////
//
// TexStreamBuffer
//
////////////////////////////////////////////////////////////////


class TexStreamBuffer : public TexStreamBase
{
public:
  TexStreamBuffer(TexStreamBase * sbuf, TexRow * texrow);
	int line() const { return line_; }
	int column() const { return column_; }

protected:
  int overflow(int);
  int sync();

private:
  TexStreamBase * sbuf_; 
	TexRow * texrow_;
	int column_;
	int line_;
};


TexStreamBuffer::TexStreamBuffer(TexStreamBase *sb, TexRow * texrow)
  : sbuf_(sb), texrow_(texrow), line_(0)
{
  setp(0, 0);
  setg(0, 0, 0);
}

int TexStreamBuffer::overflow(int c)
{
	if (c == '\n') {
		++line_;
		column_ = 0;
	} else {
		++column_;
	}
	return c;
}


int TexStreamBuffer::sync()
{
  sbuf_->pubsync();
  return 0;
}

  
////////////////////////////////////////////////////////////////
//
// TexStream
//
////////////////////////////////////////////////////////////////

TexStream::TexStream(TexStreamBase * sbuf, TexRow * texrow)
		: std::basic_ostream<char_type>(sbuf_ = new TexStreamBuffer(sbuf, texrow))
{}


TexStream::~TexStream()
{
	delete sbuf_;
}


int TexStream::line() const
{
	return sbuf_->line();
}


////////////////////////////////////////////////////////////////
//
// Test
//
////////////////////////////////////////////////////////////////

#if 0

int main(int argc, char *argv[])
{
	TexStream out(std::cout.rdbuf());
	char c;
	while (std::cin) {
		if (std::cin.get(c))
			out.put(c);
	}
	std::cout << "line count: " << out.line() << std::endl;

  return 0;
}

#endif

}
