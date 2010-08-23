#ifndef MYIOSTREAM
#define MYIOSTREAM
#include <set>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <utility>
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
//#include <algorithm>

using namespace std;

istream& get_fstring(istream& in, string& s, string const& d);
string const whitespace(" \t\n\r");

template <class T >
istream& operator >> (istream& in, set<T >& s)
{
  char c;
  s.clear();

  in >> c; // read in '{'
  assert( c == '{');

  while(1)
  {
  	c = in.get();
        switch(c) {
	    case EOF:  cout << "istream error: end of file\n";
		       return in;
            case '}':  return in;
            case ',': 
            case ';': 
            case ' ':
            case '\t':
            case '\n':
            case '\r':break;
            default: in.putback(c);
		     {		
  		      T v;
                      in >> v;
		      if( in.good() )
                      	s.insert(v);
		      else
		      {
			cout << "istream error: wrong element type.\n";
			in.clear();
			in.ignore();
		      }
		     }
                     break;
          }
   }
}

/* a pair is given in this format
	(t1, t2)
or	
	(t1 t2)
Notice that in the overloaded operators >> for T1 and T2, make sure
",()" are delimiters.
*/
template <class T1, class T2>
istream& operator >> (istream& in, pair<T1,T2>& p)
{
  char c;

  in >> c; // read in '('
  assert( c == '(' );

  in >> p.first; 
	// in class T1, make sure the char ',' and whitespaces are delim.
  
  in >> c; // read in ',' or a char for class T2
  if( c != ',' )
	in.putback(c);

  in >> p.second; 
	// in class T2, make sure the char ')' and whitespaces are delim.
  
  in >> c; // read in ')'
  assert( c == ')' );

  return in;
}

namespace std {
template <class T1, class T2>
ostream& operator << (ostream& out, pair<T1,T2> const& p)
{
	out << "(";
	out << p.first << ", ";
	out << p.second;
	out << ")";

	return out;
}
}

namespace std {
template <class T >
ostream& operator<<(ostream& out, set<T> const& m)
{
	ostringstream u;
	u << "{";
	ostream_iterator<T> screen(u, ",");
	copy(m.begin(), m.end(), screen);

	string s = u.str();
	string::size_type len = s.length();
	if( !m.empty() )
		s.replace(len-1, 1, "}");
	else
		s+= "}";

	out << s;
	return out;
}
}

/* Is the file "open error"? */
template <class STREAM>
inline
void file_open_error(STREAM& f, string const& s)
{
	if( !f )
	{
		cerr << "Can't open the file: " << s << endl;
		exit(1);
	}
}

inline
void open_error( string const& code)
{
	cerr << "Can't open file: " 
		<< code << endl;
	exit(1);
}

/* Is the file existed? */
bool file_existed(string const& filename);

/* May use this function later.
void checkstatus(ifstream &in)
{
  ios::iostate i;
   
  i = in.rdstate();
   
  if(i & ios::eofbit)
    cout << "EOF encountered\n";
  else if(i & ios::failbit)
    cout << "Non-Fatal I/O error\n";
  else if(i & ios::badbit)
    cout << "Fatal I/O error\n";
}
*/ 
#endif /*MYIOSTREAM*/
