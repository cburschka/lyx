// -*- C++ -*-

#ifndef VC_BACKEND_H
#define VC_BACKEND_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "support/syscall.h"

class Buffer;

///
class VCS {
public:
	///
	enum VCStatus {
		///
		UNLOCKED,
		///
		LOCKED
	};
	///
	virtual ~VCS() {}
	///
	virtual void scanMaster() = 0;
	///
	virtual void registrer(string const & msg) = 0;
	///
	virtual void checkIn(string const & msg) = 0;
	///
	virtual void checkOut() = 0;
	///
	virtual void revert() = 0;
	///
	virtual void undoLast() = 0;
	///
	virtual void getLog(string const &) = 0;
	///
	string const & version() const { return version_; }
	///
	string const & locker() const { return locker_; }
	///
	void owner(Buffer * b) { owner_ = b; }
	///
	Buffer * owner() const { return owner_; }
	///
	VCStatus stat() const { return vcstat; }
protected:
	///
	static int doVCCommand(string const &, string const &);

	/** The master VC file. For RCS this is *,v or RCS/ *,v. master should
	    have full path.
	*/
	string master_;
	
	/// The status of the VC controlled file.
	VCStatus vcstat;
	
	/** The version of the VC file. I am not sure if this can be a
	    string of if it must be a
	    float/int. */
	string version_;
	
	/// The user currently keeping the lock on the VC file.
	string locker_;
	/// The buffer using this VC
	Buffer * owner_;
};


///
class RCS : public VCS {
public:
	///
	RCS(string const & m);
	///
	static string find_file(string const & file);
	///
	static void retrive(string const & file);
	///
	virtual void scanMaster();
	///
	virtual void registrer(string const & msg);
	///
	virtual void checkIn(string const & msg);
	///
	virtual void checkOut();
	///
	virtual void revert();
	///
	virtual void undoLast();
	///
	virtual void getLog(string const &);
private:
};


///
class CVS : public VCS {
public:
	///
	CVS(string const & m, string const & f);
	///
	static string find_file(string const & file);
	///
	virtual void scanMaster();
	///
	virtual void registrer(string const & msg);
	///
	virtual void checkIn(string const & msg);
	///
	virtual void checkOut();
	///
	virtual void revert();
	///
	virtual void undoLast();
	///
	virtual void getLog(string const &);
private:
	string file_;
};
#endif
