// -*- C++ -*-

#ifndef KERNEL_H
#define KERNEL_H


#include "LString.h"


class Buffer;
class BufferView;
class FuncRequest;
class LyXView;


class Kernel {
public:
	///
	enum DocTypes {
		///
		LATEX,
		///
		LITERATE,
		///
		LINUXDOC,
		///
		DOCBOOK
	};

	///
	Kernel(LyXView &);
	///
	void dispatch(FuncRequest const &, bool verbose = false) const;
	/** The Dialog has received a request from the user to update
	    its contents. It must, therefore, ask the kernel to provide
	    this information to Dialog 'name'.
	 */
	void updateDialog(string const & name) const;
	///
	void disconnect(string const & name) const;
	///
	bool isBufferAvailable() const;
	///
	bool isBufferReadonly() const;
	///
	DocTypes docType() const;
	///
	LyXView & lyxview() { return lyxview_; }
	///
	LyXView const & lyxview() const { return lyxview_; }
	///
	Buffer * buffer();
	///
	Buffer const * buffer() const;
	///
	BufferView * bufferview();
	///
	BufferView const * bufferview() const;

private:
	LyXView & lyxview_;
};


#endif // KERNEL_H
