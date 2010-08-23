#include "fstring.h"

string 	fstring::_delimiter;

// not right to construct the FIRST fstring object using this constructor
// because the delimiter isn't initialized yet.
fstring::fstring(string const& s) : string(s)
{

	if ( _delimiter.empty() )
	{
		cerr << "The first fstring constructor must be passed with a nonempty delimiter string.\n";
		exit(1);
	}
}

// use this contructor to construct the FIRST fstring object 
fstring::fstring(string const& s, string const& d) : string(s)
{
	if( d.empty() ) // in default, using whitespace as delimiter
		_delimiter = whitespace;
	else
		_delimiter = d+whitespace;
}

istream& operator >>(istream& in, fstring& fs)
{

	return get_fstring(in, *(string*)&fs, fs._delimiter);
}

