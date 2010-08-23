#include "tct_engine.h"
#include "automaton.h"
#include <fstream>
#include <sstream>


// It's allowed to have a DES file with postfix .des or .DES
bool add_des_postfix_check( string* const fn )
{
	if( file_existed( *fn + ".des" ) )
	{
		*fn += ".des";
		return true;
	}
	else if( file_existed( *fn + ".DES" ) )
	{
		*fn += ".DES";
		return true;
	}
	else
		return false;
}

/* 
	The CLASS: tct_engine
*/
bool 
tct_engine::compute( string const& output_file ) const
{
	stringstream result;
	if( _procedure(result) ) // if succeed
	{
		automaton ret(result);
		ret.write_des(output_file);
		return true;
	}
	else
		return false;

}

ostream&
tct_engine::_write_flat_STS(	ostream& out,
				state const& r, // root state 
				map<bdd,vec_trans,less_bdd> const& basicST_map,
				int	qo,
				set<int>	markers,
				set<event> const& Sigma_o) const
{
	out << "root = " << r << endl;
	out << "{" << endl;
		out << r << " = OR ";
		out << "{ ";
		for(map<bdd,vec_trans, less_bdd>::const_iterator 
				i = basicST_map.begin();
				i != basicST_map.end(); i++)
			out << i->first.id() << ", ";
		out << " }" << endl;
	out << "}" << endl;
	
	out << r << endl; // the only OR state
	set<event> sc;
	set_intersection( Sigma_o.begin(), Sigma_o.end(),
			  Sigma_c().begin(), Sigma_c().end(),
			  inserter(sc, sc.begin()));
	out << sc << endl; 

	set<event> suc;
	set_intersection( Sigma_o.begin(), Sigma_o.end(),
			  Sigma_uc().begin(), Sigma_uc().end(),
			  inserter(suc, suc.begin()));
	out << suc << endl; 

	out << "{" << endl;
	for(map<bdd,vec_trans, less_bdd>::const_iterator 
			i = basicST_map.begin();
			i != basicST_map.end(); i++)
		for(int k=0; k<(int)i->second.size(); k++)
		{
			out << "[" << i->first.id() 	<< ",";
			out << i->second[k].first   	<< ",";
			out << i->second[k].second.id() << "] ";
		}
	out << "}" << endl;

	out << "{ " << qo << " }" << endl; // initial state
	out << "{ " <<	markers<< " }" << endl; // marker states
	out << "{}" << endl; 			// not a memory

	return out;
} 

// Bread-first search
void 
tct_engine::_enumerate(	vector<bdd>* const& A,
				bdd const& C ) const
{
	A->clear();
	
	bdd Qo = Po()&C;
	if( Qo == bddfalse )
		return;

	vector<bdd> frontier, next_frontier;
	frontier.push_back(Qo);
	A->push_back(Qo);
	for(;;)
	{
		next_frontier.clear();

		for(vector<bdd>::const_iterator i=frontier.begin(); i!=frontier.end(); i++)
		{
			for(set<event>::const_iterator e=Sigma().begin(); e!=Sigma().end(); e++)
			{
				bdd v = Delta(*i,*e) & C;
				if( v != bddfalse && ::find(A->begin(), A->end(), v) == A->end() ) // defined and not in *A
				{
					A->push_back(v);
					if( ::find( next_frontier.begin(), next_frontier.end(), v ) == next_frontier.end() )
						next_frontier.push_back(v);
				}
			}
		}

		if( next_frontier.empty() )
			break;
		else
			frontier.swap(next_frontier);
	}
}

// Depth-first search
void 
tct_engine::_enumerate_depth_first(	vector<bdd>* const& A,
					bdd const& C ) const
{
	A->clear();
	
	bdd Qo = Po()&C;
	if( Qo == bddfalse )
		return;

	_enumerate_depth_first_rec(Qo,A,C);
}
void 
tct_engine::_enumerate_depth_first_rec(	bdd const& p,	
						vector<bdd>* const& A,
						bdd const& C ) const
{
	A->push_back(p);
	for(set<event>::const_iterator e=Sigma().begin(); e!=Sigma().end(); e++)
	{
		bdd v = Delta(p,*e) & C;
		if( v != bddfalse && ::find(A->begin(), A->end(), v) == A->end() ) // defined and not in *A
			_enumerate_depth_first_rec(v,A,C);
	}
}
