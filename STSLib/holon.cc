#include "holon.h"

/*
The entry for a holon is given in the following format:
	{e1, e2,...en} 
	{v1,v2,...vm}
	{[a,e1,b],...}
That is, the set of controllable events, followed by the
set of uncontrollable events and the set of INTERNAL
transitions.
*/
void 
holon::read(istream& in)
{
	clear();

	// using istream for set<state> to read event set
	set<state> s;
	in >> s;
	for(set<state>::iterator i=s.begin(); i!=s.end(); i++)
	{
		event tmp(i->label(), con);
		_Sigma_c.insert(tmp);
	}

	s.clear();
	in >> s;
	for(set<state>::iterator i=s.begin(); i!=s.end(); i++)
	{
		event tmp(i->label(), unc);
		_Sigma_uc.insert(tmp);
	}

	_Sigma.insert(_Sigma_c.begin(), _Sigma_c.end());
	_Sigma.insert(_Sigma_uc.begin(), _Sigma_uc.end());

	in >> _internal_trans;

	_init_event_info(_internal_trans);
}

void 
holon::_init_event_info(set<transition> const& set_trans)
{
	set<transition>::const_iterator it;
	for(it=set_trans.begin(); it!=set_trans.end(); it++)
		_event_info[it->e].insert(make_pair(it->s, it->t));	
}

// add/delete a transition in this holon
// return true if the addition takes place
bool	
holon::insert(transition const& trans)
{
	pair<set<transition>::iterator, bool> r = 
				_internal_trans.insert(trans);
	if( r.second == true )
		_event_info[trans.e].insert(make_pair(trans.s, trans.t));

	return r.second;
}

// true if the deletion takes place
bool	
holon::erase(transition const& trans)
{
	if( _internal_trans.erase(trans) == 1 ) //success
	{
		_event_info[trans.e].erase(make_pair(trans.s, trans.t));
		return true;
	}
	else
		return false;
}

void	
holon::enlarge_event_sets(set<event> const& S) 
{
	_Sigma.insert(S.begin(), S.end());
	for(set<event>::const_iterator i=S.begin(); i!=S.end(); i++)
	{
		if( i->controllable() )
			_Sigma_c.insert(*i);
		else
			_Sigma_uc.insert(*i);
	
		map<event, set<statepair> >::iterator k=_event_info.find(*i);
		if( k == _event_info.end() ) // *i new
			_event_info[*i];
	}
}

holon::holon(istream& in)
{
	read(in);
}
holon::holon(holon const& h)
{
	*this = h;
}

holon& 
holon::operator =(holon const& h)
{
	_Sigma 		= h._Sigma;
	_Sigma_c	= h._Sigma_c;
	_Sigma_uc	= h._Sigma_uc;
	_internal_trans	= h._internal_trans;
	_event_info	= h._event_info;

	return *this;
}

// operations
// check if event e is in this holon's inner transition structure 
bool 
holon::find(event const& e) const
{
	set<event>::const_iterator it=_Sigma.find(e);
	if( it == _Sigma.end() ) // not find
		return false;
	else	return true;
}

// find (source,target) pairs
void 
holon::find_statepairs(event const& e, set<statepair> * const p) const
{
	map<event, set<statepair> >::const_iterator 
				it = _event_info.find(e);
	if( it == _event_info.end() ) // wrong
	{
		cerr << "Event " << e << " does not belong to this holon.\n";
		exit(1);
	}

	p->clear();
	*p = it->second;
}

void 
holon::find_sources(event const& e, set<set<state> >* const p) const
{
	map<event, set<statepair> >::const_iterator
				it = _event_info.find(e);
	if( it == _event_info.end() ) // wrong
	{
		cerr << "Event " << e << " does not belong to this holon.\n";
		exit(1);
	}

	p->clear();
	for(set<statepair>::const_iterator si=it->second.begin();
			si!=it->second.end(); si++)
		p->insert(si->first); // ->first is the source state sets
}

void 
holon::find_targets(event const& e, set<set<state> >* const p) const
{
	map<event, set<statepair> >::const_iterator
				it = _event_info.find(e);
	if( it == _event_info.end() ) // wrong
	{
		cerr << "Event " << e << " does not belong to this holon.\n";
		exit(1);
	}

	p->clear();
	for(set<statepair>::const_iterator si=it->second.begin();
			si!=it->second.end(); si++)
		p->insert(si->second); // ->second is the target state sets
}

// only valid for simple state --> simple state transitions.
// undefined if s and its target states are not simple
// used to output to TCT files
void 
holon::find_trans_node_for_TCT(state const& s, set<pair<event,state> > * const p) const 
{
	p->clear();
	set<transition>::const_iterator it;
	for(it=_internal_trans.begin(); it!=_internal_trans.end(); it++)
	{
		if( *(it->s.begin()) == s )
			p->insert(make_pair(it->e, *it->t.begin()));
	}
}

ostream& operator <<(ostream& out, holon const& h)
{
	out << "The set of controllable events is:" << endl;
	out << h._Sigma_c << endl;
	out << "The set of uncontrollable events is:" << endl;
	out << h._Sigma_uc << endl;
	out << "The # of transitions is: " << h._internal_trans.size() << endl;
	out << "The set of internal transitions is:" << endl;
	out << h._internal_trans << endl;
	
	return out;
}
void
holon::clear(void)
{
	_Sigma.clear();
	_Sigma_c.clear();
	_Sigma_uc.clear();
	_internal_trans.clear();
	_event_info.clear();
}
