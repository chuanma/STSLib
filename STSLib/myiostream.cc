#include "myiostream.h"
/* Read from istream 'in' the string into 's' delimited with 
   one of the characters from the delimiter set 'd'.
   It will not eat the delimiter.
   In default, the delimiter is whitespace. */
/* call it "formated string" */
istream& 
get_fstring(istream& in, string& s, string const& d)
{
	char c;
	string whitespace = " \t\r\n";
	string dd = whitespace + d;

	/* Using "in >> c" to eat whitespaces. */
	s.clear();
	for(in >> c; c != EOF && string::npos == dd.find(c); c = in.get())
		s += c;

	if ( c != EOF )
		in.putback(c);	// put back the delimiter

	return in;	
}

/* (!in) doesn't ALWAYS mean the file doesn't exist.
   Say, the file may be being processed by other programs,
   i.e., it's locked. But this type of flag can only be set by
   Operating system based calls. Different OS has different
   implementation. So it can't be in standard C++. Thus, for
   portable reasons, we will live with it. Remember that
	1). returning "true" is credible
	2). returning "false" isn't.

   If you don't want to overwrite the file with name "abc.des",
   the correct way to do this is
	if( file_existed("abc.des") )
	  cerr<< "abc.des existed. Can't overwrite\n";// (trusted answer), 
	else // return false. abc.des may still exist
	{
	  ofstream o("abc.des");
	  file_open_error(o, "abc.des"); // this is still necessary as "abc.des" may still be locked by other programs.
	}

  By this way, we can do something trying to avoid overwriting. But we can't
  guarantee that can be achieved. Say, the other program happens to release
  the file "abc.des" right before ofstream o is constructed.
*/
bool 
file_existed(string const& filename)
{
	ifstream in(filename.c_str());
	if( in ) // existed
		return true; // in.close() by its destructor
	else // (!in) doesn't ALWAYS mean the file doesn't exist.
		return false;
}
