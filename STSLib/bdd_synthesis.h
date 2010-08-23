#ifndef BDD_SYNTHESIS
#define BDD_SYNTHESIS

using namespace std;

#include "myiostream.h"
#include "myfunctional.h"
#include "sts.h"
#include "bdd_interface.h"
#include <fstream>
#include <list>

class bdd_synthesis : public sts, public bdd_interface {
	bdd		_Pspec; // bdd spec. for the LEGAL subST
	bdd		_Pcon;	// bdd "legal state set"
	map<event,bdd>	_cf_table; 	// control function table

	bdd	_init_Pspec_cf_table(void);

	/* fixpoint post/pre image computation */
	typedef bdd (bdd_interface::*IMAGE_FUNC) 
		(bdd const&, set<event> const&) const; 
	bdd	_fixpoint(	bdd const& init,
				set<event> const& Sigma_o,
				bdd const& P,
				IMAGE_FUNC image,
				state const& x ) const;
	bdd	_fixpoint_rec(	bdd const R,
				map<state, set<event> >& Sigma_o_map,
				bdd const P,
				IMAGE_FUNC image,
				state const& x ) const;
public:
	bdd_synthesis(sts const&);

	// Operations

	/* Information */
	bdd Pcon(void) const  { return _Pcon; }
	bdd Pspec(void) const { return _Pspec;}
	map<event,bdd> const& cf_table(void) const { return _cf_table; }
	bdd control_function(event const&) const;
	bdd EligPlant(event const& e) const; //similar to Elig(e), but assuming e occuring in every state of all memories
	long double get_state_size(bdd const P) const; // count satisfiable assignments for P
	double get_state_size_ln(bdd const P) const; // avoid overflow problem in get_state_size()
	/* Predicate Transformers and Synthesis */

	//  reachable subpred. of P: R^x(G,P,init)
	bdd R(	bdd const& P, 
		bdd const& init = bddfalse,	// default: init = Po() 
		set<event> const& Sigma_o = set<event>(), // default: Sigma()
		state const& x = state() ) const; // default: ST().root()

	// coreachable subpred. of P: CR^x(G,P,init)
	bdd CR(	bdd const& P, 
		bdd const& init = bddfalse,	// default: init = Pm() 
		set<event> const& Sigma_o = set<event>(), // default: Sigma()
		state const& x = state() ) const; // default: ST().root()

	// [init]^x
	bdd bracket(	bdd const& init,
			bdd const& P = bddtrue,
			set<event>const& Sigma_o = set<event>(), //Sigma_uc()
			state const& x = state() ) const; 
	
	bdd supCP(bdd const& P) const; // supremal con. subpred. of P
	bdd Omega_P(bdd const& P) const; // a monotone transformer for "supC2P"
	bdd supC2P(bdd const& P) const; // supremal con. and coreach. subpred. of P
	bdd f_sigma(event const& e) const // f_sigma is the control function of the controllable event sigma
		{ return Gamma( _Pcon & Next(e), e); }
	bdd f_sigma_simplify(bdd const& cf, event const&) const;
	void compute(void); // compute the controller in _Pcon

	/* Input/Output */
	friend ostream& operator <<(ostream&, bdd_synthesis const&);
	void save_supC2P(char* filename) const;
	bdd  load_supC2P(char* filename);
	void print_supC2P(char* filename) const;
	void print_spec(char* filename) const;
	void print_control_functions(void) const; // filenames decided by events' label
	

	~bdd_synthesis() {}
};
#endif /* BDD_SYNTHESIS */
