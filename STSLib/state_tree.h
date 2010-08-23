#ifndef STATE_TREE
#define STATE_TREE
/* This state tree class takes care of basic input/output operations.
   Further programming is needed to extend it to answer more questions
   such as checking if two states are parallel or exclusive.

   In this version of my state tree class, it is the programmer's
   responsibility to check if the asked questions are legal, e.g.,
   to ask the type of a state that MUST be on the tree.
*/

#include "state.h"
#include "myiostream.h"
#include <map>
#include <set>
#include <vector>
#include <iterator>

// don't define the following constants to be 0, due to the "map" class in STL
typedef int TYPE;
TYPE const SIM = 1;
TYPE const AND = 2;
TYPE const OR  = 3;


/* A sub-state-tree is represented by a set of ACTIVE STATES (see chapter 2
   of my thesis). A Theta function is given to encode a subST to
   a BDD. Here a subST is a map from a OR superstate to its active children
   It's stored in a text file as a "set of active states":
	{ active_state_1, active_state_2,....}

   A special subST is ST itself, i.e., with active set {_xo}.
   The empty subST is given as {}.
*/
typedef set<state>	subST;

class state_tree {
	set<state>			X_;
	state				_xo;
	/* both of the following maps are total functions defined over _X */
	map<state, set<state> > 	_expansion; // parent -> children 
	map<state, TYPE>		_type;

	/* This map is defined over the set (_X - {_xo}) */
	map<state, state>		_parent; // child _X -{_xo} -> parent;

	istream& _read(istream&);	// read a state tree from a stream(file)
	void _init_X_simple_states(void);

	class _node {
	public:
		state s;
		TYPE  t;
		set<state> exp;
		friend bool operator <(_node const& n1, _node const& n2)
		{ return n1.s < n2.s; }
	};
	friend istream& operator >>(istream&, _node&);

	/* The recursive function called by find_descendants_of */
	void _add_descendants_of(state const&, set<state>* const) const; 

	int	_countR(state const& x) const; 
	void 	_buildR(	subST const& active_set, 
				state_tree const& ref, 
				state const& x ); 
public:
	state_tree();
	state_tree(state const&); // build a singelton state tree
	state_tree(istream&);	

	// operations on state tree
	/* 1. properties of the state tree */
	state	root(void) const { return _xo; }
	bool	empty(void) const { return X_.empty(); }
	int	size(void) const { return X_.size(); }
	set<state> const& X(void) const { return X_; }
	bool	well_formed(void) const;
	int 	size(TYPE t) const; // count # of states with TYPE t
	int	depth(void) const; // the depth of the state tree

	/*1.0 compute the size of the state space: # of basicST */
	int	count(subST const& active_set, state const& x = state() ) const; 
	int	count(state const& x = state() ) const; 

	/*1.1 manipulate (child) (sub-)state-trees */
	state_tree const& copy(	state_tree const& st, 
				state const& x = state() );
	state_tree const& plug(	state_tree const& childst, 
				state const& x = state() );
	state_tree const& replace(	state_tree const& st, 
					state const& x );
	state_tree const& combine(	state_tree const& st,
					state const& r );

	/*1.2 active set <---> state_tree */
	void	find_active_set(subST* const active_set,
				state_tree const& ref ) const;
	state_tree const& build(subST const& active_set, 
				state_tree const& ref, 
				state const& x = state() ); 

	/* 2. properties associated with a special state on the tree */
	bool	find(state const&) const; 
	int	level(state const&) const;
	TYPE 	type_of(state const&) const;
	state 	parent_of(state const &) const;
	int	count_children_of(state const& s) const;
	set<state>const& expansion(state const&) const;
	void 	find_children_of(state const&, set<state>* const) const;
	void 	find_expansion_star(state const&, set<state>* const) const;
	void 	find_ancestors_of(state const& s, vector<state>* const p, state const&) const;
	void 	find_ORancestors_of(state const&, vector<state>* const, state const&) const;
	void 	find_ORancestors_of(set<state> const&, set<state>* const, state const&) const;
	void 	find_descendants_of(state const&, set<state>* const) const;

	/* 3. Relations between 2 states over the state tree */
	state	nca(state const&, state const&) const; // nearest common ancestor
	bool 	less(state const&, state const&) const;
	bool 	less_equal(state const& a, state const& b) const
			{ return a==b || less(a,b); }
	bool	greater(state const& a, state const b) const
			{ return less(b,a); }
	bool	parallel(state const& a, state const& b) const
			{ return type_of(nca(a,b)) == AND; }
	bool	exclusive(state const& a, state const& b) const
			{ return type_of(nca(a,b)) == OR; }

	/* i/o streams */
	friend istream& operator >>(istream& in, state_tree& t)
			{ return t._read(in);}
	friend ostream& operator <<(ostream&, state_tree const&);
	ostream& save(ostream& out) const;


	void clear(void);
	~state_tree();
};

#endif /* STATE_TREE */
