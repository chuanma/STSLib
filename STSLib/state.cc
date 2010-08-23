#include "state.h"
map<string, state_id> 	state::_li; 
vector<string> 		state::_lt; 

state::state(string const& s) 	// initiated by a state label
{
	map<string, state_id>::const_iterator it = _li.find(s);
	if( it == _li.end() ) // string s is a new state
	{
		_lt.push_back(s);
		_id = _lt.size()-1;
		_li[_lt[_id]] = _id;
	}
	else // string s is already in the state set
	{
		_id = it->second;
	}
}

state& 
state::operator =(state const& s)	// assignment constructor
{
	_id = s._id;
	return *this;
}

ostream& 
operator <<(ostream& out, state const& s)
{
	out << s.label();
	return out;
}

istream& 
operator >>(istream& in, state & s)
{
	string a;
	get_fstring(in, a, state_delimiter);
	if( a.empty() ) // not valid if the string is emtpy
	{
		cerr << "The label of a state must be nonempty. Abort.\n";
		exit(1);
	}

	state tmp(a);
	s = tmp;

	return in;	
}
