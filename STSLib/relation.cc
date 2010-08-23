#include "relation.h"

// control relation constructor for e
relation::relation(bdd_synthesis const& syn, event const& e)
{
	assert(e.controllable());
	
	bdd C = syn.Pcon();
	bdd elig = C & syn.EligPlant(e); 
	bdd E_e  = syn.control_function(e) & elig; // MUST enable *e
	bdd D_e = (!syn.control_function(e)) & elig; // MUST disable *e
	
	if(D_e==bddfalse) { // enable e everywhere
		_enabled = C;
		_disabled= bddfalse;
	} else if(E_e==bddfalse) { // disable e everywhere
		_enabled = bddfalse;
		_disabled= C;
	} else {
		_enabled = C-D_e; // including dontcare states
		_disabled= C-E_e; // including dontcare states
	}
	_dontCare = _enabled & _disabled;
}

// marking relation constructor
// marking type=SURONG: the computed marking DES has to synchronize with Plant to report marking.
// marking type=INDEPENDENT: the computed marking DES can independently report marking.
relation::relation(bdd_synthesis const& syn, bool type)
{
	assert(type==INDEPENDENT || type==SURONG);
	
	bdd C = syn.Pcon();	
	if( type == INDEPENDENT ) { // this is a partition
		_enabled = syn.Pm() & C;
		_disabled= !syn.Pm() & C;
	} else { //type==SURONG, this is in general a cover.
  	  _enabled = C & ( syn.Pm() | !bdd_exist(syn.Pm(), syn.fdd_varset_of(syn.memories())));
	  bdd a = C & !syn.Pm();
	  if( (a & _enabled) != a ) // a isn't a subset of b
		_disabled = a;
	  else
		_disabled = bddfalse; // all states in C have their marking decided by plant model alone
	}
	_dontCare = _enabled & _disabled;
}

/*
 * *re: remove the states that are in _enabled;
 * *rd: remove the states that are in _disabled;
 */
void 
relation::getRemoved(bdd point, bdd cell, vector<bdd>* const r) const
{
	r->clear();
	
	bdd ce = cell&_enabled;
	if(ce == cell) return;
	bdd cd = cell&_disabled;
	if(cd == cell) return;
	
	//cell isn't consistent. both cd and ce are not bddfalse
	assert(cd != bddfalse && ce != bddfalse);
	
	if( point != bddfalse ) { // have to keep the part that includes point
		if((point&ce) == point) //point is in ce
			r->push_back(cell-ce); // remove the states that are not in ce. 
					//Important: dont use 'cd' because it's not a partition.
					// This way we remove less states.
		if((point&cd) == point) // point is in cd
			r->push_back(cell-cd); 
		// if point is dontcare, then we may remove either re or rd.
		// if point is in ce, we can only remove rd.
		// if point is in cd, we can only remove re.
	} else { // point is bddfalse. So we can remove either re or rd.
		r->push_back(cell-ce);
		r->push_back(cell-cd);
	}
}

/*
 * le: largest possible subset of _enabled that includes cell and excludes excluded.
 * ld: largest possible subset of _disabled that includes cell and excludes excluded.
 * precondition: cell must be control consistent.
 */
void 
relation::getEnlarged(bdd cell, vector<bdd>* const l, bdd excluded) const
{
	l->clear();
	bdd e=bddfalse, d=bddfalse;
	if( (cell&_enabled) == cell )
		e = (_enabled-excluded);
	if( (cell&_disabled) == cell )
		d = (_disabled-excluded);
	bdd t = e & d;
	if( t == e ) { // e subset of d, enlarge it to the larger set d.
		l->push_back(d|cell);
	} else if ( t == d ) { //d subset of e
		l->push_back(e|cell);
	} else { // 2 choices
		l->push_back(e|cell);
		l->push_back(d|cell);
	}
}

void 
relation::split(bdd cell, vector<bdd>* const r) const
{
	r->clear();
	bdd ce = cell&_enabled;
	if(ce == cell) { r->push_back(cell); return;}
	bdd cd = cell&_disabled;
	if(cd == cell) { r->push_back(cell); return;}
	
	r->push_back(ce);
	r->push_back(cd);
}
