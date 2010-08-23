#ifndef BDD_INTERFACE
#define BDD_INTERFACE

using namespace std;

#include "sts.h"
#include <fdd.h>
#include <functional>
#include <algorithm>


//#define NODESIZE	4000000
//#define CACHESIZE	400000
#define NODESIZE	8000000
#define CACHESIZE	800000


typedef int 	variable;
typedef int	value;

/* For all recursive functions ended with "_R" and the returned value pointed
   by a pointer, the pointed object MUST be cleared first, due to
   the restriction of recursiveness 
   usually MUST be initialized */
/* BDD encoded event information for computation */
class bdd_interface {
	static int 	_num_of_interfaces; // # of interfaces in use
		
	state_tree		_ST; // store the structural info.
	bdd			_Po;
	bdd			_Pm;

	int 		_base; // the first domain created by fdd_extdomain
			       // This is useful when 2 more interfaces coexist.
	/* (normal or prime) var. -> OR state for FDD
	   In this class, assume normal var. and prime var. alternating */
	// The size of _var2s is 2 times of _H.size();
	vector<state>		_var2s; 

	/* OR state -> its normal variable.
	   In this class, the prime variable = its normal variable + 1 */
	// The size of _s2var is equal to _H.size()
	map<state, variable>	_s2var;	

	/* OR component -> its encoded value. */
	map<state, value>	_s2val;
	void			_init_s2val(void);
	value			_get_value(state const&) const;
			
	void	_gen_var_orders_R(state_tree const& st, state const& x, vector<state>*p) const; // p points to a permutation of all OR states under x
	bool _normal(variable const& v) const { return v%2 == 0; }
	bool _prime(variable const& v) const { return v%2 == 1; }
	variable _get_normal_var(state) const;
	variable _get_prime_var(state) const;
	state 	 _get_state(variable const& v) const { return _var2s[v]; }



	/* For each event sigma, there is an associated _encoded_info */
	typedef struct _encoded_info {
		bdd		Elig;
		bdd		Next;

		bdd		N; // transition relation labeled by sigma
		bddPair*	p; // pairs of variables for N
		bdd		v; // variables in N for taget, used for CR()

		bdd		NR; // trans. rel. of sigma to compute R()
		bddPair*	pR; // pairs of variables for NR
		bdd		vR; // variables in N representing source, used
				    	// for R(G,.) in bdd_exist()
	} _ei;
	map<event, _ei>		_trans; // a table encoding transitions
	bddPair*		_var_n2p_pairs; // normal -> prime pairs
	bddPair*		_var_p2n_pairs; // prime  -> normal pairs
public:
	bdd_interface(sts const&);

	/* information */
	bdd Pm(void) const { return _Pm; }
	bdd Po(void) const { return _Po; } 
	bdd Elig(event const& e) const; // return Elig(e), set of eligible STS basic-state-trees
	bdd Next(event const& e) const; // return Next(e)

	/* Conversion */
	bdd n2p(bdd const& P) const { return bdd_replace(P, _var_n2p_pairs); }
	bdd p2n(bdd const& P) const { return bdd_replace(P, _var_p2n_pairs); }
	bdd fdd_domain_of(state const& c) const;
	bdd fdd_varset_of(state const& c) const;
	bdd fdd_varset_of(set<state> const& m) const;
	bdd normal_variable_set() const;
	bdd fdd_encode(state const& c) const;
	bdd fdd_encode_prime(state const& c) const;
	bdd Theta(state const&) const;
	bdd Theta(subST const& st) const { return Theta(st, _ST.root()); }
	bdd Theta(subST const&, state const&) const; // encode child subST
	bdd Theta_prime(state const&) const;
	bdd Theta_prime(subST const&) const;
	bdd Theta_prime(subST const&, state const&) const; // encode child subST

	/* one-step post-image pre-image computation */
	bdd Delta(bdd const&, event const&) const;
	bdd Delta(bdd const&, set<event> const& Sigma) const; 
	bdd Gamma(bdd const&, event const&) const;
	bdd Gamma(bdd const&, set<event> const& Sigma) const; 

	/* Other operations */
	// notice that forall(P, a_state_var) doesn't make sense, see ch4
	// because the variables of descendants are all related to its
	// ancestors' varialbes.
	bdd forall_under(bdd const& P, state const&) const;
	bdd forall_under(bdd const& P, set<state>const&) const;
	bdd exist_under(bdd const& P, state const&) const;
	bdd exist_under(bdd const& P, set<state>const&) const;
	bdd simplify(bdd const& P) const; // simplify by bdd_simplify
	bdd simplify_under(bdd const& P, state const&) const; 


	~bdd_interface();
};

	void fdd_streamhandler(ostream& o, int var);
	void fdd_filehandler(FILE *o, int var);
	void bdd_streamhandler(ostream& o, int var);
	void bdd_filehandler(FILE *o, int var);
#endif /* BDD_INTERFACE */
