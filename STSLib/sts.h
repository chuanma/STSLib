#ifndef STS
#define STS
/* This is just a STS container which takes care of input/output.
   The acutual computation will be implemented by another class
   called sybmolic_sts.
*/

#include "state_tree.h"
#include "holon.h"
#include "myiostream.h"

/*log: consider using "memories" as spec too */

/* There are two types of specifications(see ch4 of my thesis.
   We call them spec1 and spce2 in this program 
*/
typedef set<subST>			spec1; // ILLEGAL set
typedef set<pair<subST, event> >	spec2; // ILLEGAL set

class sts {
	state_tree 		_ST;
	map<state, holon> 	_H;
	set<event>		_Sigma;	
	set<event>		_Sigma_c;	
	set<event>		_Sigma_uc;	
	subST			_STo;
	set<subST>		_STm;

	/* For the Control Problem in STS Framework */
	set<state>	_memories; // memories are a set of OR states
	bool		_valid_memory(state const& x) const; // valid means all children of x must be simple state.
	void		_memories2_E2(void);
	spec1		_E1; 
	spec2		_E2; 
public:
	sts() {}
	sts(istream& inp, istream& ins) { read(inp,ins); }
	void read(istream& inp, istream& ins);
					// read from plant and spec files
	sts(vector<string> const& plant, vector<string> const& spec, string const& root); // read from ctct .des files
	sts(sts const&);
	sts& operator =(sts const&);

	/* Operations */

	/* Information of a sts */
	state_tree const& ST(void) const { return _ST; }
	map<state, holon> const& H(void) const { return _H; }
	holon const& H(state const& s) const;
	holon & H(state const& s);
	set<event> const& Sigma(void) const { return _Sigma; }
	set<event> const& Sigma_c(void) const { return _Sigma_c; }
	set<event> const& Sigma_uc(void) const { return _Sigma_uc; }
	subST const& STo(void) const { return _STo; }
	set<subST> const& STm(void) const { return _STm; }
	set<state> const& memories(void) const {return _memories; }
	spec1 const& E1(void) const { return _E1; }
	spec2 const& E2(void) const { return _E2; }
	bool  flat(void) const 
		{ return _ST.size(OR) == 1 && _ST.size(AND) == 0; }

	/* Information of one special event */
	void find(event const&, set<state>*) const; // find the set of OR states where e belongs

	/* Output (Later may add DOTTY output) */
	friend ostream& operator <<(ostream&, sts const&);
	
	void clear(void);
	~sts() {}
};

#endif /* STS */
