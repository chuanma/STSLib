#include "bdd_synthesis.h"
#include <bdd.h>
#include <numeric>

/* Initialize the specification: "_Pspec" and 
	      the control function table: "_cf_table" */
bdd	
bdd_synthesis::_init_Pspec_cf_table(void)
{ 
	bdd bdd_spec1 = bddfalse; // predicate for the E1 in class sts
	bdd bdd_spec2 = bddfalse; // predicate for the E2 in class sts

	// for type 1 specification
	for(spec1::const_iterator it=E1().begin(); 
				it!=E1().end(); it++)
		bdd_spec1 |= Theta(*it);

	// for type 2 specification
	// Let every controllable event be enabled initially
	for(set<event>::const_iterator et=Sigma_c().begin();
				et!=Sigma_c().end(); et++) {
		_cf_table[*et] = _initial_cf[*et] = bddtrue; 
  }

	for(spec2::const_iterator it=E2().begin(); 
				it!=E2().end(); it++)
	{
		if( it->second.controllable() ) { // controllable event
			_cf_table[it->second] &= !Theta(it->first); 
			_initial_cf[it->second] = _cf_table[it->second];
		} else // uncontrollable event
			bdd_spec2 |= Theta(it->first) & Elig(it->second); // see ch4
	}
	
	_Pspec = !(bdd_spec1 | bdd_spec2); // _Pspec is the set of LEGAL subST.

	return _Pspec;
}

bdd_synthesis::bdd_synthesis(sts const& g) : sts(g), bdd_interface(g)
{
	_init_Pspec_cf_table();
}

/* Overide Delta if e is controllable and _initial_cf[e] is not bddtrue */
bdd
bdd_synthesis::Delta(bdd const& P, event const& e) const
{
  if(e.controllable()) {
    // take into account the initial control function 
    map<event,bdd>::const_iterator i = _initial_cf.find(e);
    if(i->second != bddtrue)
      return bdd_interface::Delta(P & i->second, e);
  }

  return bdd_interface::Delta(P, e);
}

/* Overide Gamma if e is controllable and _initial_cf[e] is not bddtrue */
bdd
bdd_synthesis::Gamma(bdd const& P, event const& e) const
{
  if(e.controllable()) {
    // take into account the initial control function 
    map<event,bdd>::const_iterator i = _initial_cf.find(e);
    if(i->second != bddtrue)
      return bdd_interface::Gamma(P, e) & i->second;
  }

  return bdd_interface::Gamma(P, e);
}

bdd 
bdd_synthesis::control_function(event const& e) const
{
	map<event,bdd>::const_iterator i = _cf_table.find(e);
	if( i != _cf_table.end() )
		return i->second;
	else {
		cerr<< "The event "+e.label()+" does not have a control function." << endl;
		exit(1);
	}
}

// Operations

/*
Return:				a set of basicST (given as a bdd) where all
				elements are (co)reachable from "init & P" via
				a transition path on which
			       		(1) all transitions are labelled 
					with events in Sigma_o, and
					(2) every basicST also satisfy P.
				Notice that the return must include "init&P".

bdd init: 			an initial set of basicST
set<event> const& Sigma_o: 	a set of observable events (to this fixpoint
				computation). Only the post/pre image of
				these events are computed.
bdd P:				the predicate that every resulting basicST
				must satisfy. e.g., the pred. P in CR(G,P).
IMAGE_FUNC image:		the one-step pre/post image function, e.g,
				Delta and Gamma (over a set of events)
state const& x:			the computation will only be performed on
				STS^x, the child STS rooted by x. In default,
				it is ST().root().
*/
bdd	
bdd_synthesis::_fixpoint(	bdd const& init,
				set<event> const& Sigma_o,
				bdd const& P,
				IMAGE_FUNC image,
				state const& x ) const
{
	// build Sigma_o_map
	map<state, set<event> > Sigma_o_map;
	set<event> shared;
	for(map<state,holon>::const_iterator i = H().begin();
			i != H().end(); i++ )
	{
		set_intersection( Sigma_o.begin(), Sigma_o.end(),
				i->second.Sigma().begin(), 
				i->second.Sigma().end(), 
				inserter(shared, shared.begin()) );
		Sigma_o_map[i->first].swap(shared);
		assert( shared.empty() );
	}

	return _fixpoint_rec( init & P, Sigma_o_map, P, image, x );
}

bdd	
bdd_synthesis::_fixpoint_rec(	bdd const R,
				map<state, set<event> >& Sigma_o_map,
				bdd const P,
				IMAGE_FUNC image,
				state const& x ) const
{
	bdd ret = R;
	if( ST().type_of(x) == OR ) 
	{
		for(;;)
		{
			bdd K0 = ret;

			ret |= (this->*image)(ret, Sigma_o_map[x]) & P;

			set<state> const& ex=ST().expansion(x);
			for(set<state>::const_iterator i=ex.begin(); 
						i!=ex.end(); i++)
				if( ST().type_of(*i) != SIM )
					ret = _fixpoint_rec(ret, Sigma_o_map, P, image, *i);

			if( K0 == ret ) 
				return ret;
		}
	}
	else // AND state
	{
		for(;;)
		{
			bdd K0 = ret;
			
			set<state> const& ex=ST().expansion(x);
			for(set<state>::const_iterator i=ex.begin(); 
						i!=ex.end(); i++)
				ret = _fixpoint_rec(ret, Sigma_o_map, P, image, *i);

			if( K0 == ret )
				return ret;
		}
	}

	assert(0);
}

/* Predicate Transformers and Synthesis */

/* supCP(P) := not [not P] */
bdd 
bdd_synthesis::supCP(bdd const& P) const // supremal con. subpred. of P
{
	return ! bracket(!P);
}

/* R(G,P) in the thesis */
//  reachable subpred. of P: R^x(G,P,R)
/* 
   Why using init == bddfalse as a flag to set init = Po()?
   A: Because the return must be bddfalse if init == bddfalse, 
   there is no need to call this function in that case. Same reasoning for
   other flags.

   Examples:
	1. call "R(P)" to compute R(G,P) defined in the ch3 of the thesis
	2. call "R(P,I)" to compute all basicSTs that are reachable from I
		by paths on which every basicST satisfy P
	3. call "R(P,I,Sigma_o)". same as 2 except that all transitions on
	 	the path must be labelled with an event in Simga_o
	4. call "R(P,I,Sigma_o, x)". same as 3 except that a path can only
		have the transitions in childSTS G^x
*/

bdd 
bdd_synthesis::R(	bdd const& P, 
			bdd const& init,
			set<event> const& Sigma_o,
			state const& x ) const 
{
	bdd i;
	if( init == bddfalse )// default: init = Po()
		i = Po();
	else
		i = init;

	state r;
	if( x == state() )// default: ST().root()
		r = ST().root();
	else
		r = x;

	if( Sigma_o == set<event>() )// default: Sigma()
	{
		set<event> const& sig = Sigma();
		return _fixpoint(i, sig, P, &bdd_interface::Delta, r);
	}
	else
		return _fixpoint(i, Sigma_o, P, &bdd_interface::Delta, r);
}

bdd
bdd_synthesis::CR(	bdd const& P, 
			bdd const& init,
			set<event> const& Sigma_o,
			state const& x ) const 
{
	bdd i;
	if( init == bddfalse )// default: init = Pm()
		i = Pm();
	else
		i = init;

	state r;
	if( x == state() )// default: ST().root()
		r = ST().root();
	else
		r = x;

	if( Sigma_o == set<event>() )// default: Sigma()
	{
		set<event> const& sig = Sigma();
		return _fixpoint(i, sig, P, &bdd_interface::Gamma, r);
	}
	else
		return _fixpoint(i, Sigma_o, P, &bdd_interface::Gamma, r);
}	


bdd 
bdd_synthesis::bracket(	bdd const& init,
			bdd const& P,
			set<event>const& Sigma_o,
			state const& x ) const 
{
	state r;
	if( x == state() )// default: ST().root()
		r = ST().root();
	else
		r = x;

	if( Sigma_o == set<event>() )// default: Sigma_uc()
	{
		set<event> const& sig = Sigma_uc(); // default: Sigma_uc()
		return _fixpoint(init, sig, P, &bdd_interface::Gamma, r);
	}
	else
		return _fixpoint(init, Sigma_o, P, &bdd_interface::Gamma, r);
}


inline bdd 
bdd_synthesis::Omega_P(bdd const& K) const // a monotone transformer for "supC2P"
{
	//return CR(supCP(K)) & _Pspec;
	return CR(supCP(K));
}

bdd 
bdd_synthesis::supC2P(bdd const& P) const // supremal con. and coreach. subpred. of P
{
	bdd ret = P;
	for(;;)
	{
		bdd K1 = ret;
		
		ret = Omega_P(ret);
		if( K1 == ret )
			return ret;
	}
}

void 
bdd_synthesis::compute(void) // compute the controller in _Pcon
{
	_Pcon = supC2P(_Pspec);
	_Pcon = R(_Pcon); // return the reachable subpred. 

	if( _Pcon == bddfalse ) // empty reachable legal set
	{
		cout << "The reachable set of legal basic sub-state-tree is empty. That is, there is no nonempty solution to this control problem!.\n";
		return;
	}

	// now update the table of control functions
	for(map<event,bdd>::iterator i=_cf_table.begin();
				i!=_cf_table.end(); i++)
	{
		i->second &= f_sigma_simplify(
					f_sigma(i->first), 
					i->first);
	}
}

bdd	
bdd_synthesis::f_sigma_simplify( bdd const& cf, event const& e) const
{
	assert( e.controllable() );
	return bdd_simplify(cf, _Pcon & Elig(e));
}

//Return the set of eligible basic-state-trees, assuming 'e' occurring 
//at all states of each memory module.
bdd 
bdd_synthesis::EligPlant(event const& e) const
{
	return bdd_exist(Elig(e), fdd_varset_of(memories()));
}

// using this class to define function objects to be used in transform()
// Unfortunately, template functions can't deduct "overloaded" functions
// like bdd_nodecount in BuDDy. The compiler doesn't know which one to 
// choose. This is too dumb, but it's true!
class bdd_nc { //bdd_nodecount
public:
	int operator()(bdd const& b)
	{
		return bdd_nodecount(b);
	}
};	
extern int bdd_nodecount(const bdd &r);
/* Input/Output */
ostream& operator <<(ostream& out , bdd_synthesis const& b)
{
	// output the bdd size of _Pspec, _Pcon
	out << "The bdd size of the specification: ";
	out << bdd_nodecount(b._Pspec) << endl;
	out << "The bdd size of the illegal state set: ";
	out << bdd_nodecount(!b._Pspec) << endl;
	out << "The bdd size of the optimal controlled behavior: ";
	out << bdd_nodecount(b._Pcon) << endl;


	// output the bdd size of all control functions
	vector<bdd> vb;
	for(map<event,bdd>::const_iterator i=b._cf_table.begin();
				i!=b._cf_table.end(); i++)
		vb.push_back(i->second);
	vector<int> counts;
	transform(vb.begin(),vb.end(), back_inserter(counts), bdd_nc());

	out << "The bdd size of all control functions: ";
	out << accumulate(counts.begin(), counts.end(), 0) << endl;
	
	// output the maximum(minimum) bdd size of control functions
	out << "The maximum bdd size of control functions: ";
	out << *max_element(counts.begin(), counts.end()) << endl;
	out << "The minimum bdd size of control functions: ";
	out << *min_element(counts.begin(), counts.end()) << endl;

	return out;
}

void 
bdd_synthesis::save_supC2P(char* filename) const
{
	bdd_fnsave(filename, _Pcon);
}

bdd  
bdd_synthesis::load_supC2P(char* filename) 
{
	bdd_fnload(filename, _Pcon);
	return _Pcon;
}

void 
bdd_synthesis::print_supC2P(char* filename) const
{
	bdd_fnprintdot(filename, _Pcon);
}

void
bdd_synthesis::print_spec(char* filename) const
{
	bdd_fnprintdot(filename, this->_Pspec);
}
void 
bdd_synthesis::print_control_functions(void) const	
{
	for(map<event,bdd>::const_iterator i=_cf_table.begin();
				i!=_cf_table.end(); i++)
	{ // for each control function

		string filename(i->first.label());
		if( i->second == bddtrue ) // enable all
			filename += ".en.all";
		else if( i->second == bddfalse ) // disable all
			filename += ".dis.all";
		else // enable(disable) at some but not all states
			filename += ".en";
		
		char* f = new char[filename.size()+1];
		strcpy(f, filename.c_str());
		bdd_fnprintdot(f, i->second);			
		delete f;
	}
}

long double
bdd_synthesis::get_state_size(bdd const P) const
{
	return bdd_satcountset(P, normal_variable_set());
}

double
bdd_synthesis::get_state_size_ln(bdd const P) const
{
	return bdd_satcountlnset(P, normal_variable_set());
}
