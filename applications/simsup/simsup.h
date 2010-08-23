#ifndef SIMSUP_H
#define SIMSUP_H

// This class is based on Su Rong's congruence

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
// arg_handler reads the file names of both plant and spec components, also the output file name
bool arg_handler(	int argc, char* argv[], 
			vector<string>* const plant,
			vector<string>* const spec,
			string* const output_file );

class control_relation {
	pair<bdd,bdd> 		_marker; 
	vector<pair<bdd,bdd> >	_control;
public:
	control_relation(bdd_synthesis const& syn, bdd const& C);
	bool operator ()(bdd const& ab) const // the set ab inside a cover of R
	{
		if( !((ab & _marker.first) == ab || (ab & _marker.second) == ab) ) // ab not in a cell
			return false;

		for(vector<pair<bdd,bdd> >::const_iterator i=_control.begin(); i!=_control.end(); i++)
			if( !((ab & i->first) == ab || (ab & i->second) == ab ) ) // ab not in a cell
				return false;
		return true;
	}
};

class simsup : public tct_engine {
public:
	simsup(vector<string> const& plant, vector<string> const& spec) : tct_engine(plant,spec)
	{ // compute the legal reachable states first
		_C = supC2P(Pspec());
		_C = R(_C); // the reachable pred. 
	}
	simsup(bdd_synthesis const& syn) : tct_engine(syn) 
	{
		_C = supC2P(Pspec());
		_C = R(_C); // the reachable pred. 
	}
	
	pair<int,int> lbe( void ) const
	{
		return _lbe(_C);
	}

	pair<int,int> lbe3( void ) const
	{
		return _lbe3(_C);
	}
	
	virtual ~simsup() {}
protected:
	bool _procedure(ostream& out) const;

	// Supreduce and its estimations
	virtual int _supreduce(ostream& out, bdd const& C) const; // return the size of cover
	pair<int, int> _lbe(bdd const& C) const;
	pair<int, int> _lbe3(bdd const& C) const;

	bool _insert_if(	bdd const& v,
				list<bdd>* const new_cells ) const;
	bool _update_cover(	bdd const& v,
				vector<bdd>::size_type const curr,
				vector<bdd>* const cover ) const;
	int _merge_one_cell(	vector<bdd>::size_type curr,
				vector<bdd>* const cover,
				control_relation const& R,
				bdd const& C) const;
	void _induce_flat_STS(	ostream& out, 
				vector<bdd> const& cover,
				bdd const& C ) const;
	void _check( vector<bdd> const& cover, bdd const& C ) const; // check the cover

private:
	bdd _C;
	bool _legal_cell(	bdd const& ab,
				list<bdd>* const waitlist,
				vector<bdd>::size_type curr,
				vector<bdd> const& cover,
				control_relation const& R,
				bdd const& C ) const;

	// The lower bound estimation based on R ONLY
	void _Ramsey(	vector<bdd>* const C,
			vector<bdd>* const I,	
			list<bdd>* const V,
			control_relation const& R ) const;
	int _list_cliques(control_relation const& R, vector<bdd>* const V) const;
	int _clique_removal(	control_relation const& R,
				list<bdd>* const V) const;

	// The lower bound estimation based on both R and the third condition (all downstreams states satisfy R)
	bool _R3(	bdd const& a, bdd const& b,
			set<bdd, less_bdd>* const waitlist,
			control_relation const& R) const;
	void _Ramsey3(	vector<bdd>* const C,
			vector<bdd>* const I,	
			list<bdd>* const V,
			set<bdd, less_bdd>* const waitlist,
			control_relation const& R ) const; 
	int _list_cliques3(control_relation const& R, vector<bdd>* const V, set<bdd, less_bdd>* const waitlist) const;
	int _clique_removal3(	control_relation const& R,
				list<bdd>* const V,
				set<bdd, less_bdd>* const waitlist) const;
	int _greedy_algo( void ) const;
};


#endif /*SIMSUP_H*/
