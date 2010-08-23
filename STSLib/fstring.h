#ifndef FSTRING
#define FSTRING
#include "myiostream.h"

class fstring : public string {
	static string 	_delimiter;
public:
	fstring() {}
	fstring(string const& s); 			// unsafe constructor
	fstring(string const& s, string const& d); 	// safe constructor

	string const& 	get_delimiter(void) const	{ return _delimiter; }
	void 		set_delimiter(string const& d) 	{ _delimiter = d; }
	string const&   str(void) const { return *(string*)this; }

	string const& operator ()() { return *(string*)this; }
	friend istream& operator >>(istream&, fstring&);
};
#endif
