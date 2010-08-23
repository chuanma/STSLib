#include "event.h"
map<string, event_id> 		event::_li;
vector<pair<string,bool> >	event::_lt;	

/* initiated by a string label and con/uncon. 
   The constructor adds new element to the event alphabet (_lt)
   if event lable "e" dosn't  appear in the alphabet. 
   If "e" is already in the alphabet, the value in "bool c" will be
   compared with the existing event's con/unc property and if it's
   conflicting, print out error info. and exit
*/
event::event(string const& e, bool c) 
{
	map<string, event_id>::const_iterator it = _li.find(e);
	if( it == _li.end() ) // string e is a new event
	{
		_lt.push_back(make_pair(e,c));
		_id = _lt.size()-1;
		_li[_lt[_id].first] = _id;
	}
	else // string e is already in the event alphabet
	{
		_id = it->second;
		if( c != _lt[_id].second ) // conflicting
		{
			cerr << "In " << __FILE__ << "(line " << __LINE__;
			cerr << "): The event " << e << " is set to be"
		     	<< (c==con? " ":" un") 
		 	<< "controllable, which is conflicting with another"
			<< " event in the alphabet. abort." << endl; 
			exit(1);
		}
	}
}

// Unsafe contructor, but more convenient
event::event(string const& e) 
{
	map<string, event_id>::const_iterator it = _li.find(e);
	if( it == _li.end() ) // string e isn't found in the alphabet 
	{
		cerr << "In " << __FILE__ << "(line " << __LINE__;
		cerr << "): The event class has no idea of the controllability"
			<< " of event " << e << ". ";
		cerr << "Remember to use the SAFE event constructor for the "
 			<< "event " << e << " before using UNSAFE constructor."
			<< endl;
		exit(1);
	}
	else // string e is found in the event alphabet
	{
		_id = it->second;
	}
}

event& 
event::operator = (event const& e)	// assignment constructor
{
	_id = e._id;
	return *this;
}

ostream& 
operator <<(ostream& out, event const& e)	
{
//	out << e.label() << (e.controllable()?"(con)": "(unc)");
	out << e.label();
	return out;
}

/* This istream operator assumes the event alphabet is already constructed */
istream& 
operator >>(istream& in, event & e)	
{
	string a;
	get_fstring(in, a, event_delimiter); 
	event tmp(a); // unsafe constructor, assumming event a in the alphabet.
	e = tmp;
	return in;
}
