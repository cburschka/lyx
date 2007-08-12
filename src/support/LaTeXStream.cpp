#include "LaTeXStream.h"

#include <iostream>
#include <streambuf>

namespace lyx {

////////////////////////////////////////////////////////////////
//
// LaTeXStreamBuffer
//
////////////////////////////////////////////////////////////////


class LaTeXStreamBuffer : public std::streambuf
{
public:
  explicit LaTeXStreamBuffer(std::streambuf * sbuf);
	int line() const { return line_; }

protected:
  int overflow(int);
  int sync();

private:
  std::streambuf * sbuf_; 
	int line_;
};


LaTeXStreamBuffer::LaTeXStreamBuffer(std::streambuf *sb)
  : sbuf_(sb), line_(0)
{
  setp(0, 0);
  setg(0, 0, 0);
}

int LaTeXStreamBuffer::overflow(int c)
{
	if (c == '\n')
		++line_;
	return c;
}


int LaTeXStreamBuffer::sync()
{
  sbuf_->pubsync();
  return 0;
}

  
////////////////////////////////////////////////////////////////
//
// LaTeXStream
//
////////////////////////////////////////////////////////////////

LaTeXStream::LaTeXStream(std::streambuf * sbuf)
		: std::ostream(sbuf_ = new LaTeXStreamBuffer(sbuf))
{}


LaTeXStream::~LaTeXStream()
{
	delete sbuf_;
}


int LaTeXStream::line() const
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
	LaTeXStream out(std::cout.rdbuf());
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
