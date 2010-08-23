#ifndef HOLON
#define HOLON

#include "state.h"
#include "event.h"
#include "transition.h"
#include "myiostream.h"
#include <set>
#include <map>
#include <iterator>
#include <iomanip>

using namespace std;

typedef pair<set<state>, set<state> > statepair; // first=source, second=target

class holon {
	set<event> _Sigma;
	set<event> _Sigma_c;
	set<event> _Sigma_uc;

	set<transition> _internal_trans;
	map<event, set<statepair> > _event_info;

	void _init_event_info(set<transition> const&);
public:
	holon() {}
	holon(istream&);
	holon(holon const&);
	holon& operator =(holon const&);
	void read(istream&);

	// information
	set<event> const& Sigma(void) const{ return _Sigma; }
	set<event> const& Sigma_c(void) const{ return _Sigma_c; }
	set<event> const& Sigma_uc(void) const{ return _Sigma_uc; }
	bool find(event const& e) const;
	void find_statepairs(event const&, set<statepair> * const) const; 
			// find (source,target) pairs
	void find_sources(event const&, set<set<state> >* const) const;
	void find_targets(event const&, set<set<state> >* const) const;
	void find_trans_node_for_TCT(state const&, set<pair<event,state> > * const) const; // only valid for simple state --> simple state transitions.

	// add/delete a transition in this holon
	bool	insert(transition const&);
	bool	erase(transition const&);
	// enlarge the event set of this holon, including _Sigma, _Sigma_c, _sigma_uc
	void	enlarge_event_sets(set<event> const& S); 

	friend ostream& operator <<(ostream&, holon const&);
	friend istream& operator >>(istream& in, holon & h)
		{ h.read(in); return in; }

	void clear(void);
	~holon() {}
};


#endif /* HOLON */
