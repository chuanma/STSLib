#include "transition.h"

/* The transition struct is given as
  	[s e t], [ s e t ], [s,e,t], [s;e;t], or [s,e;t]
*/
istream& 
operator >>(istream& in, trans& tr)
{
  char c;
  string del = ",;"; // non-whitespace delimiters in the transition struct
  tr.s.clear(); // initialize
  tr.t.clear();

  in >> c; // eat the char '['

  in >> c;
  if ( c == '{' ) // the source node includes  a set of source states
  {
  	in.putback(c);
        in >> tr.s;
  }
  else // a single source state
  {
	in.putback(c);
	state tmp;
	in >> tmp;
	tr.s.insert(tmp);
  }
  in >> c;
  if( string::npos == del.find(c) ) // if c isn't a delimiter
	in.putback(c); // keep it. otherwise eat it
 

  in >> tr.e;
  in >> c;
  if( string::npos == del.find(c) ) // if c isn't a delimiter
	in.putback(c); // keep it. otherwise eat it
 
  in >> c;
  if ( c == '{' ) // the target node includes  a set of target states
  {
  	in.putback(c);
        in >> tr.t;
  }
  else // a single target state
  {
	in.putback(c);
	state tmp;
	in >> tmp;
	tr.t.insert(tmp);
  }

  in >> c; // eat the char ']'

  return in;
}

ostream& 
operator <<(ostream& out, trans const& tr)
{
	out << "[ ";
	if( tr.s.size() == 1 ) // if it's single state set
		out << *tr.s.begin() << " ";
	else
		out << tr.s << " ";

	out << tr.e.label() << " ";

	if( tr.t.size() == 1 ) // if it's single state set
		out << *tr.t.begin() << " ";
	else
		out << tr.t << " ";
	out << "]  ";
	
	return out;
}
