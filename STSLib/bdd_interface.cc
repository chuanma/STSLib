#include "bdd_interface.h"
#include <vector>


int 	bdd_interface::_num_of_interfaces = 0; // # of interfaces in use

vector<state> forprint; // same as bdd_interface::_var2s
/* Due to the fixed interface required by BuDDy, I have to make
   such copy "forprint" */

// p points to a permutation of OR states. The perumtation given in this
// function is just one choice.	Further research is required to try others.
void	
bdd_interface::_gen_var_orders_R(state_tree const& st, state const& x, vector<state>* p) const
{
	if( st.type_of(x) == OR )
		p->push_back(x);

	if( st.type_of(x) == OR || st.type_of(x) == AND ) // not simple state
	{
		set<state> children;
		st.find_children_of(x, &children);
		for(set<state>::const_iterator it=children.begin();
				it!=children.end(); it++)
			_gen_var_orders_R(st,*it,p);
	}
}

variable 
bdd_interface::_get_normal_var(state s) const
{
	map<state, variable>::const_iterator it=_s2var.find(s);	
	if( it == _s2var.end() ) // s isn't an OR state
	{
		cerr << s << " is not an OR state.\n";
		exit(1);
	}
	return (_base + it->second); // normal variable
}

variable 
bdd_interface::_get_prime_var(state s) const
{
	map<state, variable>::const_iterator it=_s2var.find(s);	
	if( it == _s2var.end() ) // s isn't an OR state
	{
		cerr << s << " is not an OR state.\n";
		exit(1);
	}
	return (_base + it->second + 1); // prime variable
}

/* Require that _ST is valid */
void	
bdd_interface::_init_s2val(void)
{ /* _s2val: OR component -> its encoded value. */

	_s2val.clear();
	set<state> const& Xr = _ST.X();
	for(set<state>::const_iterator x=Xr.begin(); x!=Xr.end(); x++)
	{
		if( _ST.type_of(*x) == OR ) // only for OR components
		{
			set<state> const& ex = _ST.expansion(*x);
			value v = 0;
			for(set<state>::const_iterator it=ex.begin();
					it!=ex.end(); it++, v++)
				_s2val[*it] = v;
		}
	}
}

value
bdd_interface::_get_value(state const& s) const
{
	map<state,value>::const_iterator it=_s2val.find(s);
	if( it == _s2val.end() ) // s isn't an OR component
	{
		cerr << s << " is not an OR component.\n";
		exit(1);
	}
	return it->second;
}

/*Step 1: initialize variables *
 *Step 2: initialize encoded transitions for all events 
*/
bdd_interface::bdd_interface(sts const& g) : _ST(g.ST())
{
	_num_of_interfaces++; 

	if( _num_of_interfaces == 1 ) // first time
		bdd_init( NODESIZE, CACHESIZE ); // BuDDy init

	_init_s2val();

	/*Step 1: initialize variables */
	_s2var.clear();	
	_var2s.clear(); 
	vector<state> ordered_var;
	_gen_var_orders_R(g.ST(), g.ST().root(), &ordered_var);
	for(vector<state>::size_type i=0; i<ordered_var.size(); i++)
	{
		_var2s.push_back(ordered_var[i]); // normal var.
		_var2s.push_back(ordered_var[i]); // followed by prime var.
		_s2var[ordered_var[i]] = 2*i; // only for normal var.
	}
	forprint = _var2s;	// for FDD handlers

	for(vector<state>::size_type i=0; i<_var2s.size(); i++)
	{
		int fdd_size = g.ST().count_children_of(_var2s[i]);
		if( i == 0 ) // record _base
			_base = fdd_extdomain(&fdd_size, 1);
		else
			fdd_extdomain(&fdd_size, 1);
	}

//cout << "The index of the first fdd domain for " 
//	<< _ST.root() << " is: " << _base << endl;

	fdd_strm_hook(fdd_streamhandler);
	fdd_file_hook(fdd_filehandler);
	bdd_strm_hook(bdd_streamhandler);
	bdd_file_hook(bdd_filehandler);

	// set bddPairs for all varialbes	
	_var_n2p_pairs = bdd_newpair(); 
	_var_p2n_pairs = bdd_newpair(); 
	for(map<state,variable>::const_iterator it=_s2var.begin();
				it!=_s2var.end(); it++)
	{ // for each OR state
		fdd_setpair(_var_p2n_pairs, _get_prime_var(it->first),
					_get_normal_var(it->first));
		fdd_setpair(_var_n2p_pairs, _get_normal_var(it->first),
					_get_prime_var(it->first));
	}

	/*Step 2: initialize encoded transitions for all events */
	for(set<event>::const_iterator it= g.Sigma().begin();
				it!=g.Sigma().end(); it++)
	{ // for each event *it, initialize _encoded_info "a"
		_ei	a;	

		set<state> D; // set of OR states where *it belongs
		g.find(*it, &D);	

		/*Step 2a: initialize  Elig, Next for *it */
		/* the following codes valid only for sound STS */
		subST elig_subST; // Eligible subST, given by active states 
		subST next_subST; // Next subST, given by active states 
		for(set<state>::const_iterator h=D.begin();
					h!=D.end(); h++)
		{ // for each holon
			holon const& hr = g.H(*h);
			set<set<state> > t1;

			hr.find_sources(*it, &t1);
			for(set<set<state> >::const_iterator k=t1.begin();
						k!=t1.end(); k++)
				elig_subST.insert(k->begin(), k->end());

			hr.find_targets(*it, &t1);
			for(set<set<state> >::const_iterator k=t1.begin();
						k!=t1.end(); k++)
				next_subST.insert(k->begin(), k->end());
		}
		a.Elig = Theta(elig_subST);
		a.Next = Theta(next_subST);

		/*Step 2b: initialize  for func. Delta,Gamma(bdd, *it) */
		a.N	= bddtrue;
		a.p	= bdd_newpair();	
		a.v	= bddtrue;
		a.NR	= bddtrue;
		a.pR	= bdd_newpair();	
		a.vR	= bddtrue;
		set<state>	source;
		set<state>	target;
		for(set<state>::const_iterator h=D.begin();
					h!=D.end(); h++)
		{ // for each holon

			set<statepair> sp;
			holon const& hr = g.H(*h);
			hr.find_statepairs(*it, &sp);
			bdd N1 = bddfalse;
			bdd NR1= bddfalse;
			for(set<statepair>::const_iterator k=sp.begin();
						k!=sp.end(); k++)
			{ // for each statepair
				N1 |= Theta_prime(k->first, *h) & Theta(k->second);
				NR1 |= Theta(k->first) & Theta_prime(k->second,*h);
				set<state> tmp;
				g.ST().find_ORancestors_of(k->first,&tmp,*h);
				source.insert(tmp.begin(), tmp.end());

				g.ST().find_ORancestors_of(k->second,&tmp,*h);
				target.insert(tmp.begin(), tmp.end());
			}

			a.N  &= N1;
			a.NR &= NR1;
		}// finish a.N and a.NR. 

		
		vector<variable> vs_prime, vs, vt_prime, vt;
		transform(source.begin(), source.end(), 
			back_insert_iterator<vector<variable> >(vs_prime),
			bind1st(mem_fun(&bdd_interface::_get_prime_var), this));
		transform(source.begin(), source.end(), 
			back_insert_iterator<vector<variable> >(vs),
			bind1st(mem_fun(&bdd_interface::_get_normal_var), this));
		transform(target.begin(), target.end(), 
			back_insert_iterator<vector<variable> >(vt_prime),
			bind1st(mem_fun(&bdd_interface::_get_prime_var), this));
		transform(target.begin(), target.end(), 
			back_insert_iterator<vector<variable> >(vt),
			bind1st(mem_fun(&bdd_interface::_get_normal_var), this));
		// initialize a.p and a.v;
		fdd_setpairs(a.p, &vs_prime[0], &vs[0], vs.size());
		a.v = fdd_makeset(&vt[0], vt.size()); 
		// initialize a.pR and a.vR;
		fdd_setpairs(a.pR, &vt_prime[0], &vt[0], vt.size());
		a.vR = fdd_makeset(&vs[0], vs.size()); 
	
/*MA*
char* f = new char[it->label().size()+1];
strcpy(f, it->label().c_str());
bdd_fnprintdot(f, a.N);			
delete f;
*/
		_trans[*it] = a; 
	}

	// initialze Po and Pm
	_Po = Theta(g.STo());
	_Pm = bddfalse;
	for(set<subST>::const_iterator i=g.STm().begin(); 
				i!=g.STm().end(); i++)
		_Pm |= Theta(*i);
}

/* return the domain(variable range) of state c */
bdd 
bdd_interface::fdd_domain_of(state const& c) const
{ 
	assert(_ST.type_of(c) == OR );
	return ::fdd_domain(_get_normal_var(c)); 
}

/* return the variable set of state c */
bdd 
bdd_interface::fdd_varset_of(state const& c) const
{
	assert(_ST.type_of(c) == OR );
	return fdd_ithset(_get_normal_var(c));
}

/* return the variable set of a set of states */
bdd 
bdd_interface::fdd_varset_of(set<state> const& m) const
{
	for(set<state>::const_iterator i = m.begin();
			i != m.end(); i++ )
		assert( _ST.type_of(*i) == OR );

	vector<variable> vm;
	transform(m.begin(), m.end(), 
		back_insert_iterator<vector<variable> >(vm),
		bind1st(mem_fun(&bdd_interface::_get_normal_var), this));
	return fdd_makeset(&(vm[0]), vm.size());
}

/* return the variable set of all OR states */
bdd
bdd_interface::normal_variable_set() const
{
	vector<variable> vm;
	for(map<state, variable>::const_iterator i=_s2var.begin(); i!=_s2var.end(); i++) {
		vm.push_back(i->second);
	}
	return fdd_makeset(&(vm[0]), vm.size());
}

/* encode an OR component c to bdd: v_p = c */
bdd 
bdd_interface::fdd_encode(state const& c) const
{
	state f = _ST.parent_of(c);
	if( _ST.type_of(f) != OR ) // something's wrong
	{
		cerr << "Try to encode AND component: " << c << endl;
		exit(1);
	}
	variable var = _get_normal_var(f);
	value v      = _get_value(c);

	return fdd_ithvar(var, v);
}

bdd 
bdd_interface::fdd_encode_prime(state const& c) const
{
	bdd tmp = fdd_encode(c);
	return bdd_replace(tmp, _var_n2p_pairs);
}

/* Theta func. from subST to predicate . See my thesis: ch4 */
bdd 
bdd_interface::Theta(state const& s) const
{
	subST st;
	st.insert(s);
	return Theta(st);
}

/* Encode a child sub-state-tree of state x */
bdd 
bdd_interface::Theta(subST const& st, state const& x) const // encode child subST
{
	assert( _ST.type_of(x) != SIM );

	if( st.size() == 1 && *st.begin() == _ST.root() )
		return bddtrue;
	if( st.empty() )
		return bddfalse;

	bdd ret;
	if( _ST.type_of(x) == OR )
	{
		ret = bddfalse;
		set<state> const& ex = _ST.expansion(x);
		set<state> shared, diff;
		set_intersection(st.begin(), st.end(), ex.begin(), ex.end(),
			insert_iterator<set<state> >(shared, shared.begin()));
		if( !shared.empty() ) // at least one child of x is active
		{
			for(set<state>::const_iterator i=shared.begin();
					i!=shared.end(); i++)
				ret |= fdd_encode(*i);

			set_difference(	ex.begin(), ex.end(),
					st.begin(), st.end(),		      
			     insert_iterator<set<state> >(diff, diff.begin()));
			for(set<state>::const_iterator i=diff.begin();
					i!=diff.end(); i++)
				if( _ST.type_of(*i) != SIM )
					ret |= fdd_encode(*i) & Theta(st, *i);
		}
		else // no children of x is active
		{
			for(set<state>::const_iterator i=ex.begin();
					i!=ex.end(); i++)
				if( _ST.type_of(*i) != SIM )
					ret |= fdd_encode(*i) & Theta(st, *i);
		}
	}
	else // x is AND state
	{
		ret = bddtrue;
		vector<bdd> child_ret;
		set<state> const& ex = _ST.expansion(x);
		for(set<state>::const_iterator i=ex.begin(); i!=ex.end(); i++)
			child_ret.push_back(Theta(st,*i)); // no simple children
		vector<bdd>::const_iterator it = 
			find_if( child_ret.begin(), child_ret.end(), 
				bind2nd(not_equal_to<bdd>(), bddfalse) );
		if( it == child_ret.end() ) // all children return bddfalse
			ret = bddfalse;
		else // at least on child doesn't return bddfalse
		{
			for(vector<bdd>::size_type k=0; k<child_ret.size(); k++)
				if( child_ret[k] != bddfalse )
					ret &= child_ret[k];
		}
	}
		
	return ret;
}

inline bdd 
bdd_interface::Theta_prime(state const& s) const
{
	return bdd_replace(Theta(s), _var_n2p_pairs);
}

inline bdd 
bdd_interface::Theta_prime(subST const& st) const
{
	return bdd_replace(Theta(st), _var_n2p_pairs);	
}

/* Encode a child sub-state-tree of state x, using prime variables */
inline bdd 
bdd_interface::Theta_prime(subST const& st, state const& x) const // encode child subST
{
	return bdd_replace(Theta(st,x), _var_n2p_pairs);
}

/* Information */
bdd 
bdd_interface::Elig(event const& e) const // return Elig(e)
{
	map<event, _ei>::const_iterator i=_trans.find(e); 
	return i->second.Elig;
}

bdd 
bdd_interface::Next(event const& e) const // return Next(e)
{
	map<event, _ei>::const_iterator i=_trans.find(e); 
	return i->second.Next;
}

/* one-step post-image computation */
bdd 
bdd_interface::Delta(bdd const& P, event const& e) const
{
	map<event, _ei>::const_iterator i=_trans.find(e); 

	return bdd_replace( 	bdd_relprod(P, i->second.NR, i->second.vR),
				i->second.pR 
			  );
}

bdd 
bdd_interface::Delta(bdd const& P, set<event> const& Sigma) const 
{
	bdd ret = bddfalse;
	for(set<event>::const_iterator i=Sigma.begin(); 
				i!=Sigma.end(); i++)
		ret |= Delta(P, *i);

	return ret;
}

/* one-step pre-image computation */
bdd 
bdd_interface::Gamma(bdd const& P, event const& e) const
{
	map<event, _ei>::const_iterator i=_trans.find(e); 

	return bdd_replace( 	bdd_relprod(P, i->second.N, i->second.v),
				i->second.p 
			  );
}

bdd 
bdd_interface::Gamma(bdd const& P, set<event> const& Sigma) const 
{
	bdd ret = bddfalse;
	for(set<event>::const_iterator i=Sigma.begin(); 
				i!=Sigma.end(); i++)
		ret |= Gamma(P, *i);

	return ret;
}

/* Other operations */
// This func. finds ALL of the subSTs in P with childST _ST^x

// Input: bdd P and the OR state "x" to be quantified out
//        by forall operator. All descendant OR state of "x"
//        will also be quantified out.      
// Output: the resulting bdd that is independent of any variables under x
// It's always the case that P >= forall_under(P,x)
// Notice that for a variable var,
//              2^(m-1) <= ||fdd_domain(var)|| <= 2^m
// for some m. So you can't use the buddy2.0 function
//              bdd_forall
// directly. What we do here is to let
//              P' = bdd_not( fdd_domain(var) );
// and then apply
//              bdd_forall(P|P', bdd_var);
// You can easily check that it performs exactly what we want it to.
// notice that forall(P, x) doesn't make sense, see ch4
bdd 
bdd_interface::forall_under(bdd const& P, state const& x) const
{
	bdd ret = P;
	if( _ST.type_of(x) == SIM )
		return ret;

	// check all descendants first
	set<state> const& ex=_ST.expansion(x);
	for(set<state>::const_iterator i=ex.begin(); i!=ex.end(); i++)
		ret = forall_under(ret, *i);

	// now ret is independent of any variable of x's descendants
	if( _ST.type_of(x) == OR ) // no need to handle other states
	{
		variable var = _get_normal_var(x);
		ret         |= !fdd_domain(var);
		return bdd_forall(ret, fdd_ithset(var) );
	}
	else // x is AND 
		return ret;
}

// Input: bdd P and the OR state set "a" to be quantified out
//        by forall operator. All descendant OR state of "a"
//        will also be quantified out.      
// Output: the resulting bdd that is independent of any variables under x
// It's always the case that P >= forall_under(P,x)
bdd 
bdd_interface::forall_under(bdd const& P, set<state>const& a) const
{
	bdd ret = P;
	for(set<state>::const_iterator i=a.begin(); i!=a.end(); i++)
		ret = forall_under(ret, *i);

	return ret;
}

// It extends ALL of the subSTs in P to childST _ST^x
// Input: bdd P and the OR state "x" to be quantified out
//        by bdd_exist operator. All descendant OR state of "x"
//        will also be quantified out.      
// Output: the resulting bdd that is independent of any variables under x
// It's always the case that P <= exist_under(P,x)
bdd 
bdd_interface::exist_under(bdd const& P, state const& x) const
{
	bdd ret = P;
	if( _ST.type_of(x) == SIM )
		return ret;

	// check all descendants first
	set<state> const& ex=_ST.expansion(x);
	for(set<state>::const_iterator i=ex.begin(); i!=ex.end(); i++)
		ret = exist_under(ret, *i);

	// now ret is independent of any variable of x's descendants
	if( _ST.type_of(x) == OR ) // no need to handle other states
	{
		variable var = _get_normal_var(x);
		return bdd_exist(ret, fdd_ithset(var) );
	}
	else // x is AND 
		return ret;
}

bdd 
bdd_interface::exist_under(bdd const& P, set<state>const& a) const
{
	bdd ret = P;
	for(set<state>::const_iterator i=a.begin(); i!=a.end(); i++)
		ret = exist_under(ret, *i);

	return ret;
}

bdd 
bdd_interface::simplify(bdd const& P) const // simplify by bdd_simplify
{
	return simplify_under(P, _ST.root());
}

bdd 
bdd_interface::simplify_under(bdd const& P, state const& x) const 
{
	bdd ret = P;
	if( _ST.type_of(x) == SIM )
		return ret;

	// check all descendants first
	set<state> const& ex=_ST.expansion(x);
	for(set<state>::const_iterator i=ex.begin(); i!=ex.end(); i++)
		ret = simplify_under(ret, *i);

	
	if( _ST.type_of(x) == OR ) // no need to handle other states
	{
		variable var = _get_normal_var(x);
		return bdd_simplify(ret, fdd_domain(var) );
	}
	else // x is AND 
		return ret;
}

/* BDD output handlers */
inline bool prime(variable const& var) 
{
	return var%2 == 1;
}
void fdd_streamhandler(ostream& o, int var)
{
	assert (var >=0 );
	o << forprint[var];
	if( prime(var) ) // prime variable 
		o << "'";
}

void fdd_filehandler(FILE *o, int var)
{
	assert (var >=0 );
	fprintf(o, "%s",forprint[var].label().c_str());
	if( prime(var) ) // prime variable 
		fprintf(o, "'");
}

void bdd_streamhandler(ostream& o, int var)
{
       int *bddvars;
        int block_size;
        int offset;
        // the size of forprint is equal to 2 times of _H.size()
        for(int fdd_var=0; fdd_var<(int)forprint.size(); fdd_var++)
        {
                bddvars         = fdd_vars(fdd_var);
                block_size      = fdd_varnum(fdd_var);
                for(offset=0; offset<block_size; offset++)
                        if( var == bddvars[offset] )
                                break;

                if( offset != block_size ) // find var in bddvars
                {
                        if( block_size != 1 )
                        {
                                o << forprint[fdd_var] << "_";
                                o << offset;
                        }
                        else
                                o << forprint[fdd_var];

			if( prime(fdd_var) ) // prime variable 
				o << "'";
                        break;
                }
        }
}

void bdd_filehandler(FILE *o, int var)
{
       int *bddvars;
        int block_size;
        int offset;
        // the size of forprint is equal to 2 times of _H.size()
        for(int fdd_var=0; fdd_var<(int)forprint.size(); fdd_var++)
        {
                bddvars         = fdd_vars(fdd_var);
                block_size      = fdd_varnum(fdd_var);
                for(offset=0; offset<block_size; offset++)
                        if( var == bddvars[offset] )
                                break;

                if( offset != block_size ) // find var in bddvars
                {
                        if( block_size != 1 )
                        {
                                fprintf(o, "%s_", forprint[fdd_var].label().c_str());
                                fprintf(o, "%d", offset);
                        }
                        else
                                fprintf(o, "%s", forprint[fdd_var].label().c_str());

			if( prime(fdd_var) ) // prime variable 
				fprintf(o, "'");
                        break;
                }
        }
}
bdd_interface::~bdd_interface()
{
	_num_of_interfaces--;
	if( _num_of_interfaces == 0 )
		bdd_done(); 
}
