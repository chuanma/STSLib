#include "state_tree.h"
#include <assert.h>

// read a state tree from a stream(file)
/* The data file of state tree is just a set of entries 
   in one of the following formats:
	root = x_o
	{
	x = OR { y1,...yn }
	y = AND { z1, z2, ..., zm }
	...
	}
   namely x is an OR state with expansion { y1, ... yn }.
   And there is no entries for simple states.
*/
istream&  
state_tree::_read(istream& in)
{
	clear(); 

	char c;
	string a;
	get_fstring(in, a, "=");// read in "root"
	in >> c; // read in "="
	in >> _xo; // read in the root state.
	
	set<_node> nd;
	in >> nd;
	set<_node>::const_iterator it;
	for(it=nd.begin(); it!=nd.end(); it++)
	{
		_expansion[it->s] = it->exp;
		_type[it->s] = it->t;
		set<state>::const_iterator si;
		for(si=it->exp.begin(); si!=it->exp.end(); si++)
			_parent[*si] = it->s;
	}
	// _parent is initialized. now need to take care of _X and simple states
	
	_init_X_simple_states(); 

	return in;
}

/* Function: initialize
			_X
			_type for the simple states
			_expansion for the simple states
  Assume: at least the _expansion and _type for the superstates are already initialized.
*/
void
state_tree::_init_X_simple_states(void) 
{
	// first set _X
	map<state, set<state> >::const_iterator mi;
	X_.clear();
	for(mi=_expansion.begin(); mi!=_expansion.end(); mi++)
	{
		X_.insert(mi->first);
		X_.insert(mi->second.begin(), mi->second.end());
	}

	// then according to _X, set _type and _expansion for simple states	
	set<state>::const_iterator si;
	set<state> emptyset;
	emptyset.clear();
	for(si=X_.begin(); si!=X_.end(); si++)
	{
		map<state, TYPE>::const_iterator k = _type.find(*si);
		if( k == _type.end() ) // state *si is a simple state
		{
			_type[*si] = SIM;
			_expansion[*si] = emptyset;
		}
	}
}

istream& operator >>(istream& in, state_tree::_node& n)
{
	n.exp.clear();

	char c;
	string a;

	in >> n.s;	

	in >> c; // read in '='

	get_fstring(in, a, "{");// '{' and whitespaces are delimiters
	if( a == "AND" || a == "and" )
		n.t = AND;
	else if (a == "OR" || a == "or")
		n.t = OR;
	else
	{
		cerr << "The type of ";
		cerr << n.s;
		cerr  << " should be AND/OR. Aborted." << endl;
		exit(1);
	}

	in >> n.exp;
	if( n.exp.empty() ) // something wrong
	{
		cerr << "The children of ";
		cerr << (n.t == AND?"AND":"OR");
		cerr << " state ";
		cerr << n.s;
		cerr << " should have a nonempty set of children." << endl;
		exit(1);
	}
	
	return in;
}

state_tree::state_tree()
{
}

state_tree::state_tree(state const& s) // build a singelton state tree
{
	_xo = s;
	X_.insert(_xo);
	_expansion[_xo];
	_type[_xo] = SIM;
}

state_tree::state_tree(istream& in)	
{
	_read(in);
}

// check if every AND component is either AND or OR state.
bool	
state_tree::well_formed(void) const
{
	set<state>::const_iterator si;
	for(si=X_.begin(); si!=X_.end(); si++)
	{
		map<state, TYPE>::const_iterator ti = _type.find(*si);
		if( ti->second == AND ) // require no children can be simple
		{
			map<state, set<state> >::const_iterator 
					ei = _expansion.find(*si);
			set<state> const& k=ei->second;
			set<state>::const_iterator ki;
			for(ki=k.begin(); ki!=k.end(); ki++)
			{
				ti = _type.find(*ki);
				if( ti->second == SIM ) // wrong
					return false;
			}
		}
	}

	return true;
}

int 	
state_tree::size(TYPE t) const // count # of states with TYPE t
{
	set<state>::const_iterator it;
	int c = 0;
	for(it=X_.begin(); it!=X_.end(); it++)
		if(type_of(*it) == t)
			c++;

	return c;
}

int	
state_tree::depth(void) const // the depth of the state tree
{
	set<int> levs;
	for(set<state>::const_iterator it=X_.begin(); it!=X_.end(); it++)
		levs.insert(level(*it));

	set<int>::const_iterator it = max_element(levs.begin(), levs.end());
	if( it == levs.end() ) 	return 0;
	else			return *it;
}

/* Copy a (child) state tree (rooted by x) of st to *this */
state_tree const& 
state_tree::copy(state_tree const& st, state const& x )
{
	clear();
	
	if( x == state() ) // in default, x = _xo;
	{
		*this = st;
		return *this;
	}

	_xo = x;
	for(map<state, set<state> >::const_iterator i = st._expansion.begin();
			i != st._expansion.end(); i++)
	{
		if( st.less_equal(x, i->first) ) // under x
		{
			_expansion[i->first] = i->second;
			_type[i->first] = st.type_of(i->first);
			for(set<state>::const_iterator k = i->second.begin();
					k != i->second.end(); k++ )
				_parent[*k] = i->first;
		}
	}
	
	_init_X_simple_states(); // take care of _X and simple states

	return *this;
}

// Precondition: no shared states between childst and *this
//		 the state x can't be simple state.
// Postcondition: plug to this state_tree a child state_tree of x, i.e.,
// childst._xo is a child state of x in this state_tree. 
state_tree const& 
state_tree::plug(state_tree const& childst, state const& x )
{
	set<state> shared;
	set_intersection( X_.begin(), X_.end(),
			  childst.X_.begin(), childst.X_.end(),
			  inserter(shared, shared.begin()) );
	if( !shared.empty() )
	{
		cerr << "When plugged child state tree with root "
		     << childst._xo << " to the state tree with root "
		     << _xo << ", the shared states are: "
		     << shared << " (It should be {}.)" << endl;
		exit(1);
	}

	state r;
	if( x == state() ) // in default, x = _xo
		r = _xo;
	else
		r = x;

	if( type_of(r) == SIM ) 
	{
		cerr << " Can't plug a child state tree under a simple state. "
			<< "Try to use the member function replace instead."
			<< endl;
		exit(1);
	}
	
	_expansion[r].insert(childst._xo);
	_parent[childst._xo] = r;
	     	   		     
	X_.insert(childst.X_.begin(), childst.X_.end());
	_type.insert(childst._type.begin(), childst._type.end());
	_expansion.insert(childst._expansion.begin(), childst._expansion.end());
	_parent.insert(childst._parent.begin(), childst._parent.end());

	return *this;
}

// first cut the branch(childST) rooted by x and replace it with
// another childST.
// Postcondition: another state_tree where the .
state_tree const& 
state_tree::replace(state_tree const& st, state const& x )
{
	if( x == state() ) // replace the entire state tree
	{
		*this = st;
		return *this;
	}

	// step 1. delte the child state tree rooted by x
	set<state> X_under_x;
	find_children_of(x, &X_under_x);
	X_under_x.insert(x);
	set<state> c;
	c.swap(X_);
	set_difference(c.begin(),c.end(),
			X_under_x.begin(), X_under_x.end(),
			inserter(X_, X_.begin()) );

	state p = _parent[x];
	for(set<state>::const_iterator i = X_under_x.begin();
			i != X_under_x.end(); i++)
	{
		_expansion.erase(*i);
		_type.erase(*i);
		_parent.erase(*i);
	}

	plug(st, p); // plug st as a childST of p
	return *this;
}

// Combine *this and st into a new state_tree, rooted with
// a new AND state r.
// Notice that either state tree must be NONempty and NONsingleton.
// Otherwise the resulting state tree isn't well-formed.
state_tree const& 
state_tree::combine(state_tree const& st, state const& r )
{
	if( r == state() )
	{
		cerr << "state_tree::combine: must pass a valid state.\n";
		exit(1);
	}
	if( X_.size() < 2 || st.X_.size() < 2 )
	{ //  One of them must be either emptyST or singleton state tree
		cerr << "state_tree::combine: at least one state tree"
			<< " is either empty or singleton.\n";
		exit(1);
	}
	if( X_.find(r) != X_.end() || st.X_.find(r) != st.X_.end() )
	{  
		cerr << "state_tree::combine: " << r
			<< " should not be in either state tree.\n";
		exit(1);
	}

	X_.insert(r);
	X_.insert(st.X_.begin(), st.X_.end());

	_type[r] = AND;
	_type.insert(st._type.begin(), st._type.end());

	pair<state, set<state> > root_exp;
	root_exp.first = r;
	root_exp.second.insert(_xo);
	root_exp.second.insert(st._xo);
	_expansion.insert(root_exp);
	_expansion.insert(st._expansion.begin(), st._expansion.end());

	_parent[_xo] = r;
	_parent[st._xo] = r;
	_parent.insert(st._parent.begin(), st._parent.end());

	_xo = r; 

	return *this;
}

// The referenceST is the childST of ref, rooted by x
// Precondition: *this is a (child) sub-state_tree of ref
void	
state_tree::find_active_set(	subST* const active_set,
				state_tree const& ref ) const
{
	active_set->clear();
	map<state,set<state> >::const_iterator ex;
	for( ex = _expansion.begin(); ex != _expansion.end(); ex++)
		if( type_of(ex->first) == OR ) // only in OR componenents
		{
			set<state> exp_star;
			find_expansion_star(ex->first, &exp_star);
			set<state> ref_exp_star;
			ref.find_expansion_star(ex->first, &ref_exp_star);
			if( exp_star == ref_exp_star ) // no active children
				continue;

			for(set<state>::const_iterator y=ex->second.begin();
					y!=ex->second.end(); y++)
			{
				find_expansion_star(*y, &exp_star);
				ref.find_expansion_star(*y, &ref_exp_star);
				if( exp_star == ref_exp_star )
					active_set->insert(*y); // y active
			}
		}

	if( !empty() && active_set->empty() )
		active_set->insert(_xo);
}

// build a state_tree *ret from the active set st 
// using *this as the reference ST. This is based on Defn. 2.10
state_tree const& 
state_tree::build(	subST const& active_set, 
			state_tree const& ref, 
			state const& x ) 
{
	
	clear();
	if( x == state() ) // in default, x = root state
		_xo = ref._xo;
	else
		_xo = x;

	if( active_set.empty() )
		return *this; // *this an emptyST
	if( active_set.size() == 1 && *active_set.begin() == _xo )
	{
		copy(ref, _xo); // copy the childST of _xo in ref to *this
		return *this;
	}

	_buildR(active_set, ref, _xo);
	return *this;
}

// Check the defn 2.10 at page 37.
void
state_tree::_buildR(	subST const& active_set, 
			state_tree const& ref, 
			state const& x ) 
{ // assume x is already on *this
	X_.insert(x);
	_type[x] = ref.type_of(x);

	if( _type[x] == AND )
	{
		for(set<state>::const_iterator y=ref.expansion(x).begin(); 
				y!=ref.expansion(x).end(); y++)
		{

			subST::const_iterator i;
		       	for(i = active_set.begin(); i!= active_set.end(); i++)
				if( ref.less(*y, *i) ) //AND state can't active
					break; // *y is an ancestor of some *i

			if( i != active_set.end() )
			{ // *y is an ancestor of *i
				_expansion[x].insert(*y);
				_parent[*y] = x;
				_buildR(active_set, ref, *y);
			}
			else // *y isn't an ancestor of *i
			{ // the entire branch under *y should be included
				state_tree childst;
				childst.copy(ref, *y); 
				plug(childst, x);
			}
		}
	}
	else if( _type[x] == OR )
	{
		for(set<state>::const_iterator y=ref.expansion(x).begin(); 
				y!=ref.expansion(x).end(); y++)
		{
			subST::const_iterator i;
		       	for(i = active_set.begin(); i!= active_set.end(); i++)
				if( ref.less_equal(*y, *i) )
					break; 

			if( i != active_set.end() ) // *y  should be included
			{
				_expansion[x].insert(*y);
				_parent[*y] = x;
				_buildR(active_set, ref, *y);
			}
		}
	}
	else // x is simple state. 
		_expansion[x] = set<state>();
}

/* the count of a subST: see ch2 for details of the defn. count 
   It's a recursive function computing the # of basicST of "active_set"
   under the state "x". In default, "x" is the root

   Undefined if st is NOT a valid subST of childST ST^x.
*/
int	
state_tree::count(subST const& active_set, state const& x) const 
{
	state_tree tmp;
	tmp.build(active_set, *this, x); // tmp is a childST rooted by x

	return tmp.count();
}

/* the count of this state_tree: see ch2 for details of the defn. count 
   It's a recursive function computing the # of basicST of *this
   under the state "x". In default, "x" is the root
*/
int	
state_tree::count(state const& x ) const 
{
	state r;
	if( x == state() )
		r = _xo;
	else
		r = x;

	if( empty() )
		return 0;

	return _countR(r);
}

int	
state_tree::_countR(state const& x) const 
{
	int ret;

	if( type_of(x) == AND )
	{
		ret = 1;
		for(set<state>::const_iterator y = expansion(x).begin();
				y != expansion(x).end(); y++)
			ret *= _countR(*y);
	}
	else if ( type_of(x) == OR )
	{
		ret = 0;
		for(set<state>::const_iterator y = expansion(x).begin();
				y != expansion(x).end(); y++)
			ret += _countR(*y);
	}
	else // simple state
		ret = 1;

	return ret;
}

// find if the state 's' is on the state tree
bool	
state_tree::find(state const& s) const
{
	set<state>::const_iterator it = X_.find(s);
	if( it == X_.end() ) // not find
		return false;
	else
		return true;
}

TYPE 
state_tree::type_of(state const& s) const
{
	assert(find(s));
	map<state, TYPE>::const_iterator it = _type.find(s);
	return it->second;
}

/* Return the parent of s. if s is the root state, return an invalid state */
state 
state_tree::parent_of(state const & s) const
{
	assert(find(s));

	if( s == _xo )
	{
		cerr << "No parent for the root state." << endl;
		state k; // k is an invalid state.
		return k;	
	}
	
	map<state,state>::const_iterator pi = _parent.find(s);
	return pi->second;
}

int
state_tree::count_children_of(state const& s) const
{
	map<state, set<state> >::const_iterator it = _expansion.find(s);
	return it->second.size();
}

int	
state_tree::level(state const& s) const
{
	if( s == _xo ) return 0;

	map<state,state>::const_iterator pi = _parent.find(s);
	int lv = 1;
	for( state i = pi->second; i != _xo; 
			pi = _parent.find(i), i=pi->second)
		lv++;

	return lv;
}

set<state>const& 
state_tree::expansion(state const& s) const
{
	assert(find(s));
	map<state, set<state> >::const_iterator si = _expansion.find(s);
	return si->second;
}

void 
state_tree::find_children_of(state const& s, set<state>* const p) const
{
	assert(find(s));

	p->clear();
	map<state, set<state> >::const_iterator si = _expansion.find(s);
	*p = si->second;
}

void 	
state_tree::find_expansion_star(state const& s, set<state>* const p) const
{
	assert(find(s));
	find_descendants_of(s,p);
	p->insert(s);
}

/* The ancestors are totally ordered (see my thesis).  So we return
   a vector p (instead of set) of states satisfying the following order:
	for every i < j, p[i] < p[j]
   So the root state (or "x" if "x" isn't root) is the first element in p. 
   (see the definition of the partial order "<" defined over a state tree.
*/
/* Return a set of ancestors of s under (and including) the superstate x */
void 
state_tree::find_ancestors_of(state const& s, vector<state>* const p, state const& x) const 
{
	assert(find(s) && find(x) );

	p->clear();
	if( !less(x,s) )
		return;	 // return empty set

	map<state,state>::const_iterator pi = _parent.find(s);
	for( state i = pi->second; i != x; )
	{
		p->push_back(i);
	
		pi = _parent.find(i);
		i = pi->second;
	}
	p->push_back(x); // including x

	reverse(p->begin(), p->end()); // required order with x first element
}
void 
state_tree::find_ORancestors_of(state const& s, vector<state>* const p, state const& x) const 
{
	vector<state> tmp;
	find_ancestors_of(s,&tmp, x);
	p->clear();
	for(vector<state>::const_iterator it=tmp.begin();
			it!=tmp.end(); it++)
		if( type_of(*it) == OR ) // or state
			p->push_back(*it);
}

void 	
state_tree::find_ORancestors_of(set<state> const& ss, set<state>* const p, state const& x) const
{
	p->clear();
	for(set<state>::const_iterator it=ss.begin(); it!=ss.end(); it++)
	{
		vector<state> tmp;
		find_ORancestors_of(*it, &tmp, x);
		p->insert(tmp.begin(), tmp.end());
	}
}

void 
state_tree::find_descendants_of(state const& s, set<state>* const p) const
{
	assert(find(s));

	p->clear();
	_add_descendants_of(s, p);
}
// The recursive function called by find_descendants_of
void 
state_tree::_add_descendants_of(state const& s, set<state>* const p) const
{
	map<state, set<state> >::const_iterator it = _expansion.find(s);
	p->insert(it->second.begin(), it->second.end());
	
	set<state>::const_iterator sit;
	for( sit=it->second.begin(); sit != it->second.end(); sit++)
		_add_descendants_of(*sit, p);
}


/* 3. Relations between 2 states over the state tree */
state	
state_tree::nca(state const& a, state const& b) const // nearest common ancestor
{
	vector<state> ancestors_of_a;
	vector<state> ancestors_of_b;

	find_ancestors_of(a, &ancestors_of_a, _xo);
	find_ancestors_of(b, &ancestors_of_b, _xo);

	pair<vector<state>::const_iterator, vector<state>::const_iterator >
		result = mismatch(ancestors_of_a.begin(), 
				  ancestors_of_a.end(), 
				  ancestors_of_b.begin());

	return *(--result.first); // pointing to the nca
}

// same question as: is a an ancestor of b?
bool 	
state_tree::less(state const& a, state const& b) const
{
	set<state> descendants_of_a;	
	find_descendants_of(a, &descendants_of_a);
	set<state>::const_iterator it=descendants_of_a.find(b);
	if( it == descendants_of_a.end() ) // not find
		return false;
	else 
		return true;
}

ostream& operator <<(ostream& out, state_tree const& t)
{
	out << "root = ";
	out << t._xo << endl; 
	
	out << "{" << endl;
	map<state, set<state> >::const_iterator mi=t._expansion.begin(); 
	for(; mi!=t._expansion.end(); mi++)
	{
		TYPE tp = t.type_of(mi->first);
		if( tp != SIM ) 
		{
			out << mi->first << " = ";
			out << (tp==OR? "OR ": "AND ");
			out << mi->second << endl;
		}
	}
	out << "}" << endl;

	return out;
}

ostream&	
state_tree::save(ostream& out) const
{
	out << "root = " << _xo << endl;
	out << "{" << endl;
	for(map<state, set<state> >::const_iterator i=_expansion.begin();
			i!=_expansion.end(); i++)
	{
		out << i->first << " = ";
		out << (type_of(i->first) == AND? "AND ":"OR ");
		out << i->second << endl;
	}
	out << "}" << endl;

	return out;
}

void 
state_tree::clear(void)
{
	X_.clear();
	_expansion.clear();
	_type.clear();
	_parent.clear(); 
}

state_tree::~state_tree()
{
}
