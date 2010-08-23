#ifndef PROJECT_H
#define PROJECT_H

/* provide interface for all supreduce heuristic methods, respectively.
 and also provide one interface where all heuristic methods and lbes are applied
 and the best result is returned.
 Provide:
	lower_bound_estimation (virtual)
	supreduce (virtual)
	adjoin_transitions => .DES file

  Notice: all BDD operations are hiden. Users shouldn't know it.
*/
#include "tct_engine.h"
#include <algorithm>

// arg_project reads the file names of both plant components and the event set, also the output file name
bool arg_project(	int argc, char* argv[],
			vector<string>* const plant,
			vector<string>* const spec,
			bool* const null_flag, // true if *Sigma is to be erased
			set<event>* const Sigma,
			string* const output_file);

class project: public tct_engine {
public:
	project(	vector<string> const& plant, 
			bool null_flag, 
			set<event> const& Sigma,
			vector<string> const& spec = vector<string>() ) : tct_engine(plant,spec)
	{ // compute the legal reachable states first
		if( null_flag == true ) // Sigam to be erased
			set_difference(sts::Sigma().begin(), sts::Sigma().end(),
					Sigma.begin(), Sigma.end(),
					inserter(_Sigma_o, _Sigma_o.begin()));
		else
			_Sigma_o = Sigma;

		if( !spec.empty() ) // if asked to do supcon first 
			_C = supC2P(Pspec());
		else // if no control is required
			_C = bddtrue;

	}
	
	virtual ~project() {}
private:
	bdd _C;
	set<event> _Sigma_o;
	bool _procedure(ostream& out) const; // return false if emtpy returned automaton
};


#endif /*PROJECT_H*/
