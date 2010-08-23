#ifndef TRANSITION
#define TRANSITION
#include "myiostream.h"
#include "state.h"
#include "event.h"
#include <set>
#include <iostream>
#include <string>

/* A transition is in this form in a STS file
	[ {set of source states}  event_label  {set of target states} ]
	[ source_state  event_label  {set of target states} ]
	[ {set of source states}  event_label  target_state ]
	[ source_state  event_label  target_state ]
*/
class transition {
public:
	set<state> s;	// source states(in case AND state) of the transition
	event e;	// event of the transition
	set<state> t;	// target states(in case AND state) of the transition
	friend bool operator < (transition const& s1, transition const& s2)
	{ 
		if( s1.s < s2.s ) 	return true;
		else if( s1.s == s2.s )
		{
			if( s1.e < s2.e )	return true;
			else if( s1.e == s2.e ) 
			{
				if( s1.t < s2.t)	return true;
			}
		}
		return false;
	}
};
typedef transition trans;

istream& operator >>(istream&, trans&);
ostream& operator <<(ostream&, trans const&);

#endif /* TRANSITION */
