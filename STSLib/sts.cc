#include "sts.h"
#include "automaton.h"
#include <sstream>

// valid means all children of x must be simple state.
bool
sts::_valid_memory(state const& x) const 
{
	set<state> const& ex = _ST.expansion(x);
	for(set<state>::const_iterator i=ex.begin(); i!=ex.end(); i++)
		if( _ST.type_of(*i) != SIM )
			return false;

	return true;	
}

// build type 2 specifications from the set of memories and
// at the same time update the associated holons to make
// sure them to satisfy delta(p,sigma)! for all events in the holon
void		
sts::_memories2_E2(void)
{
	for(set<state>::const_iterator i=_memories.begin(); 
				i!=_memories.end(); i++)
		if( !_valid_memory(*i) ) // not valid
		{
			cerr << *i << ": invalid memory. No superstate child allowed.\n";
			exit(1);
		}

	for(set<state>::const_iterator i=_memories.begin(); 
				i!=_memories.end(); i++)
	{ // for each holon
		// init
		holon& hr = H(*i);
		set<event> const& sgm = hr.Sigma();
		set<state> const& internal_state_set = _ST.expansion(*i);
		set<set<state> > singleton_set;
		for(set<state>::const_iterator j=internal_state_set.begin();
				j!=internal_state_set.end(); j++)
		{
			set<state> t;
			t.insert(*j);
			singleton_set.insert(t);
		}

		// handle each event in each holon
		for(set<event>::const_iterator et=sgm.begin();
					et!=sgm.end(); et++)
		{ // for each event in the holon
			set<set<state> > t1, t2;
			hr.find_sources(*et, &t1);
			set_difference(	singleton_set.begin(), 
					singleton_set.end(),
					t1.begin(), t1.end(),
			insert_iterator<set<set<state> > >(t2, t2.begin()));
			if( t2.empty() ) // event *et enabled everywhere
				continue;

			// now t2 holds the states where *et is not eligible 
			pair<subST, event> p;
			p.second = *et;
			for( set<set<state> >::iterator m=t2.begin();
							m!=t2.end(); m++)
			{
				if( !et->controllable() ) // uncontrollable
				{ // check the def. of "memory"
					transition trans;
					trans.s = *m;
					trans.e = *et;
					trans.t = *m;
					hr.insert(trans); //add a selfloop
				}
				
				assert(m->size() == 1);
				p.first.insert(*(m->begin()));
			}
			_E2.insert(p);
		}
	}
}


/* The plant file is in the format
	state tree	% use class state_tree to read _ST
	a list of holons	%the size of the list decided by |ORstate|
	initial_subST	% use overloaded >> set<state>
	set_of_marker_subST	% use overloaded >> set<<set<state> > 
	memories		% use overloaded >> set<state>

   The spec file is in the format
	type1_spec
	type2_spec
*/

void
sts::read(istream& inp, istream& ins)
{
	clear();

	/* Step 1. Read in plant sts */
	inp >> _ST;
	int sz = _ST.size(OR); // find the # of OR states

//cout << _ST << endl;

	for(int i=0; i<sz; i++) // every OR state has a matched holon
	{
		state s;
		holon h;

		inp >> s;
		inp >> h;
//cout << h << endl;
		_H[s] = h;
	}
		
	inp >> _STo;
//cout << _STo << endl;
	inp >> _STm;
//cout << _STm << endl;
	inp >> _memories; 
//cout << _memories << endl;

	for(map<state,holon>::const_iterator it=_H.begin();
				it!=_H.end(); it++)
	{
		_Sigma.insert(it->second.Sigma().begin(), 
			      it->second.Sigma().end());
		_Sigma_c.insert(it->second.Sigma_c().begin(), 
			        it->second.Sigma_c().end());
		_Sigma_uc.insert(it->second.Sigma_uc().begin(), 
			         it->second.Sigma_uc().end());
	}

	/* Step 2. Read in specs */
	ins >> _E1;
	ins >> _E2;
	_memories2_E2();
}

// read from ctct .des files
// It's impossible to construct a vector<ifstream>. So we have to pass a filename vector
// This function builds a synchronous product model.
// Must test if all files exist before calling this function.
sts::sts(vector<string> const& plant, vector<string> const& spec, string const& root)
{
	vector<sts> des;
	for(vector<string>::const_iterator i=plant.begin(); i!=plant.end(); i++)
	{

		string  n = *i;
		string  s = *i;
		n.resize(n.size()-4); // delete the postfix .des

		ifstream des_file(s.c_str(), ios::binary);
		if( !des_file )
		{
			cerr << "Can't open DES file: " << s << endl;
			exit(1);
		}

		automaton a(des_file, n); 	// read a CTCT .des file
		sts g;
		a.to_sts(&g, PLANT); 

		des.push_back(g);
	}

	for(vector<string>::const_iterator i=spec.begin(); i!=spec.end(); i++)
	{

		string s = *i;
		string n = *i;
		n.resize(n.size()-4);

		ifstream des_file(s.c_str(), ios::binary);
		if( !des_file )
		{
			cerr << "Can't open DES file: " << s << endl;
			exit(1);
		}

		automaton a(des_file, n); 	// read a CTCT .des file
		sts g;
		a.to_sts(&g, MEMORY); 

		des.push_back(g);
	}

	assert( !des.empty() );

	if( des.size() == 1 )
		*this = des[0];
	else
	{
		// build the state tree _ST with root "root"
		state r(root);
		_ST.copy(des[0]._ST);
		_ST.combine(des[1]._ST, r);
		for(vector<sts>::size_type i=2; i<des.size(); i++)
			_ST.plug(des[i]._ST, _ST.root());

		subST marker;
		set<event> Sigma_plant; // ALL Events occurred in the plant components 
		for(vector<sts>::size_type i=0; i<des.size(); i++)
		{
			// build the map<state,holon> _H
			_H.insert(des[i]._H.begin(), des[i]._H.end());

			// built _Sigma, _Sigma_c, _Sigma_uc
			_Sigma.insert(des[i]._Sigma.begin(), des[i]._Sigma.end());
			_Sigma_c.insert(des[i]._Sigma_c.begin(), des[i]._Sigma_c.end());
			_Sigma_uc.insert(des[i]._Sigma_uc.begin(), des[i]._Sigma_uc.end());


			// build _STo, _STm
			_STo.insert(des[i]._STo.begin(), des[i]._STo.end());
			marker.insert(	(des[i]._STm.begin())->begin(), (des[i]._STm.begin())->end() );

			// build _memories
			_memories.insert(des[i]._memories.begin(),
					 des[i]._memories.end() );

			// build _E1, _E2
			_E1.insert(des[i]._E1.begin(), des[i]._E1.end());
			_E2.insert(des[i]._E2.begin(), des[i]._E2.end());

			// add this line to record all events in the plant components
			if( des[i]._memories.empty() ) // this is a plant model
				Sigma_plant.insert(des[i]._Sigma.begin(), des[i]._Sigma.end());
		}
		_STm.insert(marker);

		// enlarge the event sets of every memory holon, becasue in TCT, every spec DES's event set is
		// in default Sigma_p, the event set of the entire plant.
		for(set<state>::const_iterator i=_memories.begin(); i!=_memories.end(); i++)
			_H[*i].enlarge_event_sets(Sigma_plant);
	}
}

 // read from plant and spec files
sts::sts(sts const& g)
{
	*this = g;
}

sts& 
sts::operator =(sts const& g)
{
	_ST		= g._ST;
	_H		= g._H;
	_Sigma		= g._Sigma;	
	_Sigma_c	= g._Sigma_c;	
	_Sigma_uc	= g._Sigma_uc;	
	_STo		= g._STo;
	_STm		= g._STm;

	_memories	= g._memories;
	_E1		= g._E1;
	_E2		= g._E2;

	return *this;
}

holon const& 
sts::H(state const& s) const
{
	assert( _ST.find(s) );
	map<state,holon>::const_iterator it = _H.find(s);
	return it->second;
}

holon & 
sts::H(state const& s)
{
	assert( _ST.find(s) );
	map<state,holon>::iterator it = _H.find(s);
	return it->second;
}
// find the set of OR states where e belongs
void 
sts::find(event const& e, set<state>* p) const 
{
	p->clear();
	for(map<state,holon>::const_iterator it=_H.begin();
			it!=_H.end(); it++)
		if( it->second.find(e) ) // e is in this holon	
			p->insert(it->first);
}

/* Input/Output (Later may add DOTTY output*/
ostream& operator <<(ostream& out, sts const& g)
{
	out << "The state tree is:\n";
	out << "------------------\n";
	out << g._ST << endl;
	out << "The set of holons is:\n";
	out << "---------------------\n";
	for(map<state,holon>::const_iterator it=g._H.begin();
			it!=g._H.end(); it++)
	{
		out << it->first << endl;
		out << it->second << endl;
	}
	out << "\nThe initial sub-state-tree is: " << g._STo << endl;
	out << "The set of marker sub-state-trees is:" << endl;
	out << g._STm << endl;
	out << "-------------------------------------\n";
	out << "The set of memories is: " << g._memories << endl; 

	out << "The set of type 1 specifications is: " << endl;
	out << "-------------------------------------\n";
	out << g._E1; 
	out << "The set of type 2 specifications is: " << endl;
	out << "-------------------------------------\n";
	out << g._E2; 

	return out;
}

void
sts::clear(void)
{
	_ST.clear();		
	_H.clear();			
	_Sigma.clear();		
	_Sigma_c.clear();		
	_Sigma_uc.clear();		
	_STo.clear();			
	_STm.clear();		

	_memories.clear();		
	_E1.clear();		
	_E2.clear();		
}
