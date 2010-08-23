#ifndef TCT_ENGINE_H
#define TCT_ENGINE_H

/* provide interface for all TCT procedures.
 Provide:
	constructor:
	a list of des files as a given control problem => bdd engine for the control problem
		vector P, vector E, output_file => tct_engine object

	adjoin_transitions => .DES file
	enumerate states belongs to a set represented by a bdd

  Notice: all BDD operations are hiden. Users shouldn't know it.

  This is a ADT!
*/
#include "bdd_synthesis.h"


bool add_des_postfix_check( string* const fn );

class tct_engine: public bdd_synthesis {
public:
	tct_engine(	vector<string> const& plant, 
			vector<string> const& spec ) 
				: bdd_synthesis( sts(plant, spec, "______") ) {} // read from ctct .des files. Choose the root state of the state tree to be "______"
	tct_engine(bdd_synthesis const& syn) : bdd_synthesis(syn) {}

	// The "compute" is the standard interface that the users need to call!
	bool compute( string const& output_file ) const;

	virtual ~tct_engine() {}
protected:
	// The procedure is the standard interface that all child class must provide.
	// "out" is a stream that will be filled with a flat STS
	virtual bool _procedure(ostream& out) const = 0;

	typedef pair<event,bdd>  	trans;
	typedef vector<trans> 		vec_trans;
	// The func. _write_flat_STS provides an interface for all child class to convert to a singleton STS
	// and further to the automaton .DES file in function compute.
	ostream& _write_flat_STS(	ostream& out,
				 	state const& r, // root state 
				 	map<bdd,vec_trans,less_bdd> const& basicST_map,
				 	int	qo,
					set<int>	markers,
					set<event> const& Sigma_o) const;

	void _enumerate(		vector<bdd>* const& A,
					bdd const& C ) const;
	void _enumerate_depth_first(	vector<bdd>* const& A,
					bdd const& C ) const;

private:
	void _enumerate_depth_first_rec(	bdd const& p,	
						vector<bdd>* const& A,
						bdd const& C ) const;
};


#endif /*TCT_ENGINE_H*/
