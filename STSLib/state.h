#ifndef STATE
#define STATE

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "myiostream.h"

using namespace std;

typedef int state_id;
string const state_delimiter(",;{}[]()=");


// The label of a state id MUST be unique
// The idea is to use _id to operate a state instead of working on a string
class state {
private:
	static map<string, state_id> _li; // label => id
	static vector<string> _lt; // label table, id => label

	state_id _id;	// state id
public:
	/* constructors */
	state() {_id = -1;}	// default constructor
	state(string const&); 	// initiated by a state label
	state(state const& s) {_id = s._id;}	// copy constructor
	state& operator =(state const&);	// assignment constructor

	friend bool operator ==(state const& s1, state const& s2) 
		{ return s1._id == s2._id;}
	friend bool operator !=(state const& s1, state const& s2) 
		{ return !(s1 == s2);}
	friend bool operator < (state const& s1, state const& s2)
	     	{ return s1._lt[s1._id] < s2._lt[s2._id]; }
	friend ostream& operator <<(ostream&, state const&);
	friend istream& operator >>(istream&, state &);

	string const& label(void) const		// get the label of the state
		{assert(valid()); return _lt[_id];}
	bool valid(void) const { return (0 <= _id && _id < (int)_lt.size());}

	/* destructor */
	~state() {}
};

#endif /* STATE */
