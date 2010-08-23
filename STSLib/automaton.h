#ifndef AUTOMATON
#define AUTOMATON
/*  This class automaton is for the interface with CTCT.
 *  File i/o with .des file in ctct 
 *  A method to convert automaton model to sts.
 *  To build automaton model from sts, try to use sts::sync 
*/

#include "myiostream.h"
#include "sts.h"
#include <set>
#include <vector>

bool const MEMORY = true;
bool const PLANT  = false;

/* In an automaton, the member class "event_type" is an integer according
   to TCT. That is, if you want to write to .des file generated from a 
   .sts file, make sure the event labels are all integer strings. Otherwise, 
   the program will ASSIGN an arbitrary num
   for each event(according to the TCT event coding rule). 
   Related: bdd_synthesis::sync
*/
//	typedef map<event, automaton::event_type> 	event_coding;
//	typedef map<bdd, set<pair<event,bdd> >		trans_map;
//	typedef map<bdd, automaton::state_type> 	state_coding;
class automaton {
public:
	typedef int state_type;
	typedef int event_type;
	typedef struct transition {
		event_type	e;
		state_type	t;
		friend bool operator <( struct transition const& tn1, struct transition const& tn2)
		{ 
			if( tn1.e < tn2.e )	return true;
			else if( tn1.e == tn2.e )
			{
				if( tn1.t < tn2.t )	return true;
			}
			return false;
		}
	} trans_node; // indexed by the source state_type.

	// constructors 
	automaton() {}	
	automaton(ifstream&, string const&); 	// read a CTCT .des file
	automaton(istream& in);		// from a singleton .sts file
	automaton(string const& name, set<event> const& allEvents); // ALLEVENTS in TCT

	// information
	//state_type qo(void) const { return 0; }
	//set<state_type>const& Qm(void) const { return _Qm; }
	int 	get_state_size() const { return _t.size(); }
	bool	controllable(event_type e) const { return e%2 == 1; }
	bool	uncontrollable(event_type e) const { return e%2 == 0; }

	// operation
	void	set_name(string const& name) { _name = name; }
	void	set_state_size(int size)    { _t.resize(size); }
	void	selfloop(set<event> const& events);
	bool	insert_transition(state_type s, trans_node const&);
	bool	erase_transition(state_type s, trans_node const&);
	bool	insert_marker_state(state_type s);
	bool	erase_marker_state(state_type s);

	// output
	friend ostream& operator<<(ostream&, automaton const&);
	string write_des(string const& fn = string() ) const; // CTCT .des file. Filename decided by _name if the argument is emtpy string.
	string write_sts(bool f, string const& fn = string() ) const; // STS .sts file. Filename decided by _name if "fn" is empty string
	ostream& write_sts(bool f, ostream& out)  const;
	void to_sts(sts* const g, bool f = PLANT); // convert the automaton to a STS with singleton holon. Set the only holon to be memory if the bool arg. is "MEMORY"

private:
	string			_name;
	vector<set<trans_node> > _t; // the state_type set Q is {0,1,..._t.size()-1}	
	set<state_type>		 _Qm; // set of marker state_types

	int _num_of_transition(void) const;
};

#endif /* AUTOMATON */
