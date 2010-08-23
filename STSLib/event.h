#ifndef EVENT
#define EVENT
/* Usage
 Call the constructor
	event(string const& e,bool c) SAFE (but inconvenient) constructor
 and	event(string const& e)      UNSAFE (but convenient) constructor.
 Call the second constructor ONLY WHEN the programmer is sure the alphabet 
 has already had an event with label e. The safe constructor is usally called
 only when an event lable is first time being put in the alphabet, 
 while the unsafe constructor is called after we KNOW the event label 
 has been created in the alphabet before.
 ...
 event a("abc", con); // event a is a VALID controllable event with label "abc"
 event b("abc");// now event b is the same VALID event as a.
 event c("abc", unc); // conflicting with event a. Trying to set event "abc" to
		// be uncontrollable. But in fact it's set controllable before.
		// the program will print error info. and exit
 event d("aaa");// the program will print error info. and exit
		//because no valid event with label "aaa" is created before.
 ...
*/

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "myiostream.h"

using namespace std;

typedef int event_id;
const bool con = true;
const bool unc = false;
string const event_delimiter(",;{}[]()=");

// The label of an event id MUST be unique
// The idea is to us _id to operate an event instead of working on a string
class event {
	static map<string, event_id> _li;		// (label) => id
	static vector<pair<string,bool> >	_lt;	// label table, id => (label,bool)

	event_id _id;	// event id
public:
	/* constructors */
	event() {_id = -1;}	// default constructor, an invalid event
	event(string const& e); // Unsafe constructor
	event(string const& e, bool c); // Safe constructor
	event(event const& e)	// copy constructor
		{ _id = e._id;}
	event& operator = (event const&);	// assignment constructor


	friend bool operator ==(event const& e1, event const& e2)
		{ return e1._id == e2._id;}
	friend bool operator < (event const& e1, event const& e2)
		{ return e1._id <  e2._id;}
	friend ostream& operator <<(ostream&, event const&);	
	friend istream& operator >>(istream&, event &);	

	event_id id(void) const 	//get the label of the event
		{ assert(valid()); return _id; }
	string const& label(void) const 	//get the label of the event
		{ assert(valid()); return _lt[_id].first;}
	bool controllable(void) const 	// is the event controllable
		{ assert(valid()); return _lt[_id].second;}
	bool valid(void) const
		{ return (0 <= _id && _id < (int)_lt.size());}

	/* destructor */
	~event() {}
};


#endif	/* EVENT */
