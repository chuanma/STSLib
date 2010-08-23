#include "simsup.h"

/* read the command line and get the list of
	plant DES components
	spec DES components,
	name of the output_file
*/
bool arg_handler(	int argc, char* argv[], 
			vector<string>* const plant,
			vector<string>* const spec,
			string* const output_file )
{
	plant->clear();
	spec->clear();
	output_file->clear();

	// Step 1. initialize the vectors: plant and spec, and the root string
	if( argc == 1 ) // need to input plant DES and spec. DES one by one
	{
		cout << "Please enter name of plant DES (without .des). To end, enter -1." << endl;
		for(;;)
		{
			string fn;
			cout << "\n> ";
			cin >> fn;
			if( fn == "-1" ) break;
			if( add_des_postfix_check(&fn) )
				plant->push_back(fn);
			else
				cout << "This file does not exist! Input again.\n";
		}

		cout << "Please enter names of spec DES (without .des). To end, enter -1." << endl;
		for(;;)
		{
			string fn;
			cout << "\n> ";
			cin >> fn;
			if( fn == "-1" ) break;
			if( add_des_postfix_check(&fn) )
				spec->push_back(fn);
			else
				cout << "This file does not exist! Input again.\n";
		}
		cout << "Please enter name of output DES (without .des)." << endl;
		for(;;)
		{
			cout << "\n> ";
			cin >> *output_file;
			if( !output_file->empty() )
			{
				*output_file += ".des";
				break;
			}
		}
		
	}
	else // simsup -p P*.des -s E*.des -o output_file
	{
		int flag; // 1 for plant and 0 for spec and 2 for output_file
		string s(argv[1]);
		if( s == "-p" || s == "-P" )
			flag = 1;
		else if( s == "-s" || s == "-S" )
			flag = 0;
		else if( s == "-o" || s == "-O" )
			flag = 2;
		else // wrong format
		{
			cerr << "Usage: simsup -o output_DES_file -p plant_DES_files -s spec_DES_files" << endl;
			cerr << "Wildcards like * and ? can be used. e.g., m* includes all DES files starting with m." << endl;
			exit(1);
		}
		for(int i=2; i<argc; i++)
		{
			s = argv[i];
			if( s == "-p" || s == "-P" )
				flag = 1;
			else if( s == "-s" || s == "-S" )
				flag = 0;
			else if( s == "-o" || s == "-O" )
				flag = 2;
			else // file names
			{
				if( flag == 2 ) // no need to check existance
					*output_file = s;
				else if( file_existed(s) )
				{
					if( flag == 1 ) // plant
						plant->push_back(s);
					else if( flag == 0 )// spec
						spec->push_back(s);
				}
				else // Wrong
					open_error(s);
			}
		}
		if(  output_file->empty() || plant->empty() || spec->empty() )
		{
			cerr << "Usage: simsup -o output_DES_file -p plant_DES_files -s spec_DES_files" << endl;
			cerr << "Wildcards like * and ? can be used. e.g., m* includes all DES files starting with m." << endl;
			exit(1);
		}
	}

	//Step 2: check if plant(or spec) has same components appearing more than once 
	// 	also check if plant and spec share common components (not allowed)
	set<string> set_p(plant->begin(), plant->end());
	set<string> set_s(spec->begin(), spec->end());
	if( set_p.size() != plant->size() || set_s.size() != spec->size() )
	{
		cerr << "Entered the same component twice!" << endl;
		exit(1);
	}
	set<string> tmp;
	set_intersection(set_p.begin(), set_p.end(),
			 set_s.begin(), set_s.end(),
			 inserter(tmp, tmp.begin()));
	if( !tmp.empty() ) // wrong
	{
		cerr << "It's not allowed to share components between plant DES and spec DES!" << endl;
		cerr << "The shared DES components are: " << tmp << endl;
		exit(1);
	}
cout << set_p << endl;
cout << set_s << endl;

	return true;
}

control_relation::control_relation(bdd_synthesis const& syn, bdd const& C)
{
	// It requires that marking should be consistent in each cell.
	bdd b = C & ( syn.Pm() | !bdd_exist(syn.Pm(), syn.fdd_varset_of(syn.memories())));
	if( b != bddfalse )
		_marker.first = b;
	else // no marker states at all!
	{
		cerr << " No marker states in the final controlled system!" << endl;
		exit(1);
	}
	bdd a = C & !syn.Pm();
	if( (a & b) != a ) // a isn't a subset of b
		_marker.second = a;
	else
		_marker.second = bddfalse; // all states in C have their marking decided by plant model alone

	typedef map<event, pair<bdd,bdd> > CONDAT;
	CONDAT condat;
	for(set<event>::const_iterator e = syn.Sigma_c().begin();
			e != syn.Sigma_c().end(); e++)
	{ // for each controllable event
		bdd E_e = syn.Gamma(syn.Next(*e)&(C), *e) & C; // MUST enable *e
		bdd D_e = syn.Gamma(syn.Next(*e)&(!C), *e) & C; // MUST disable *e
		if( E_e ==  bddfalse || D_e == bddfalse ) // same condat for *e
			continue; // no need to consider *e because of same control act
		else
			condat[*e]= make_pair(C-D_e, C-E_e);
	}
	// As some controllable events may be pre-disabled in type 2 spec,
	// we need to add those disablement information too.
	for(spec2::const_iterator i = syn.E2().begin(); i != syn.E2().end(); i++)
	{
		if( i->second.controllable() ) // the event is controllable
		{// P is the state subset of C where i->second is pre-disabled
			bdd P = syn.Theta(i->first) & C & 
			bdd_exist(syn.Elig(i->second), syn.fdd_varset_of(syn.memories()));
			if( P == bddfalse ) continue;

			CONDAT::iterator k;
			k = condat.find(i->second);
			if( k == condat.end() ) // not find
			{
				bdd E_e = syn.Gamma(syn.Next(i->second)&(C), i->second) & C; // MUST enable 
				bdd D_e = syn.Gamma(syn.Next(i->second)&(!C), i->second) & C; // MUST disable *e
				D_e |= P;
				assert( (E_e & P ) == bddfalse );
				if( E_e == bddfalse || D_e == bddfalse ) 
					continue;
				else
					condat[i->second]=make_pair(C-D_e, C-E_e);
			}
			else // already in 
			{
				k->second.second |= P; // k->second: should be disabled
				k->second.first -= P; // important!
				if( k->second.second == C )
				{
				//	cout << "cell for event: " << i->second << " is erase." << endl;
					condat.erase(k);
				}
			}
		}
	}
	_control.clear();
	for(CONDAT::iterator i=condat.begin(); i!=condat.end(); i++)
		_control.push_back(i->second);
}

bool
simsup::_procedure(ostream& out) const
{
	if( _C == bddfalse ) // empty reachable legal set
	{
//		cout << "The reachable set of legal basic sub-state-tree is empty. That is, there is no nonempty solution to this control problem!.\n";
		return false;
	}

	_supreduce(out, _C);
	return true;
}

int
simsup::_supreduce(ostream& out, bdd const& C) const
{

	vector<bdd> cover;
	control_relation R(*this, C);
	_enumerate( &cover, C);
	if( cover.empty() )
	{
		cout << "Trying to run the supreduce with an empty controller!" << endl;
		return 0;
	}
	
cout << "The original controller's state size: " << cover.size() << endl;
	for(vector<bdd>::size_type i=0; i<cover.size()-1;i++)
	{
//cout << "Cover size at this iteration: " << cover.size() << endl;
		_merge_one_cell( i, &cover, R, C);
	}
cout << "The final controller's state size: " << cover.size() << endl;
	_check( cover, C );

	// now cover is a control cover. (may not be the minimal one)
	_induce_flat_STS(out, cover, C);

	return cover.size();
}

void 
simsup::_induce_flat_STS(	ostream& out, 
				vector<bdd> const& cover,
				bdd const& C ) const
{
	map<bdd,vec_trans,less_bdd> bdd_trans;
	int	qo;
	set<int> markers;
	bool find_qo = false;
	for(vector<bdd>::const_iterator i = cover.begin();
			i != cover.end(); i++)
	{ // for each RW state
		if( find_qo == false && (*i & Po()) != bddfalse )
		{
			find_qo = true;
			qo = i->id();
		}
		if( (*i & Pm()) != bddfalse )
			markers.insert(i->id());

		for(set<event>::const_iterator e = Sigma().begin();
				e != Sigma().end(); e++)
		{ // for each event
			bdd tmp = Delta(*i, *e) & C;
			if( tmp == bddfalse ) continue;
			for(vector<bdd>::const_iterator m=cover.begin();
					m!=cover.end(); m++)
				if( (tmp & *m) == tmp )
				{
					bdd_trans[*i].push_back(make_pair(*e,*m));
					break;
				}
		}
	}

	_write_flat_STS(out, ST().root(), bdd_trans, qo, markers, Sigma());
}


// This insertion is stable, meaning that it tries to keep the original order.
bool
simsup::_insert_if(	bdd const& v,
				list<bdd>* const new_cells ) const
{
	list<bdd>::iterator i;
	list<bdd>::iterator tmp;
	bool found_first = false;
	for(i=new_cells->begin(); i!=new_cells->end(); )
	{
		bdd w = v & *i;
		if( w == v ) // v subset of *i
			return false; // do nothing
		else if ( w == *i ) // *i subset of v
		{
			if( !found_first )
			{
				found_first = true;
				*i = v;
				i++;
			}
			else
			{
				tmp = i;
				i++;
				new_cells->erase(tmp);
			}
		}
		else
			i++;
	}

	if( !found_first ) // not inserted in the middle of the list
		new_cells->push_back(v);
	return true;
}
// update the cover from *cover[curr] to the end of the cover.
bool
simsup::_update_cover(	bdd const& v,
				vector<bdd>::size_type const curr,
				vector<bdd>* const cover ) const
{
	bool found_first = false;
	for(vector<bdd>::iterator i = cover->begin()+curr; i!=cover->end(); i++)
	{
		if( *i == bddfalse ) continue;
		bdd w = v & *i;
		if( w == v ) // v subset of *i
			return false; // do nothing
		else if ( w == *i ) // *i subset of v
		{
			if( !found_first )
			{
				found_first = true;
				*i = v;
			}
			else
				*i = bddfalse;
		}
	}
	//assert(found_first);
	if( !found_first )
	{
//cerr << "Redundant cells are generated.\n";
		cover->push_back(v);
	}


	return true;
}

// check if the cover is well-defined.
void 
simsup::_check( vector<bdd> const& cover, bdd const& C ) const
{
	bdd c = bddfalse;
	int is_cover = 0;
	for(vector<bdd>::const_iterator i=cover.begin(); i!=cover.end(); i++)
	{
		c |= *i;
		for(vector<bdd>::const_iterator k=i+1; k!=cover.end(); k++)
		{
			bdd w = *i & *k;
			if( w == *i || w == *k ) // exist partial order
			{
				cerr << "The cover should NOT have one set to be a subset of another!" << endl;
				return;
			}
			if( w != bddfalse )
				is_cover++;
		}
	}

	if ( c == C )
		;//cout << "The cover is well-defined." << endl;
	else
		cout << "The cover is ill-defined." << endl;

	if( is_cover != 0 )
		cout << "This is an arbitrary cover with # of sharing: " << is_cover << endl;
	else
		;//cout << "This is a congruence. " << endl;
}

bool
simsup::_legal_cell(	bdd const& ab,
				list<bdd>* const waitlist,
				vector<bdd>::size_type curr,
				vector<bdd> const& cover,
				control_relation const& R,
				bdd const& C ) const
{
	if( R(ab) ) // ab satisfies the R relation
	{
		bdd v;
		if( _insert_if(ab, waitlist) == false )
			return true; // must add it here for termination reason
		for(set<event>::const_iterator e=Sigma().begin();
				e!=Sigma().end(); e++)
		{
			if( e->controllable() )
				v = Delta(ab,*e) & C;
			else
				v = Delta(ab,*e); 
			if( v != bddfalse ) // defined 
			{
				bdd shared = bddfalse;
				int c = 0;
				for(vector<bdd>::size_type i=0; i<cover.size(); i++)
				{
					if( cover[i] == bddfalse ) continue;
					bdd t = v & cover[i];
					if( t == v ) // v is alredy in the congruence
					{
						c = 1;
						break;
					}
					if( t != bddfalse && curr <= i )
					       shared |= cover[i];	
				}
				if( c == 1 )
					continue;

				if( (v & shared) == v )
					v = shared; // now v consists of parts from the previous congruence 
				else
					return false; // v includes states from those "done" cells, impossible to merge

				// try to find if v can be merged with some cells in waitlist
				shared = bddfalse;
				for(list<bdd>::const_iterator i=waitlist->begin();
							i!=waitlist->end(); i++)
				{
					bdd t = *i & v;
					if( t == v ) // form a loop
					{
						c = 1;
						break;
					}
					if( t != bddfalse )
						shared |= *i;
				}

				if( c == 1 )
					continue;
				v |= shared;
				if( ! _legal_cell(v, waitlist, curr, cover, R, C) ) 
					return false;
			}
		}
		// now all future cells must be legal if reach here, notice that new_allowed is already updated 
		return true;
	}
	else // lucky that ab doesn't satisfy R
		return false;
}

int 
simsup::_merge_one_cell(	vector<bdd>::size_type curr,
					vector<bdd>* const cover,
					control_relation const& R,
					bdd const& C) const
{ 
	bdd&  the_one = (*cover)[curr];

	// Step 1: get a fixpoint for "the_one" such that it is a largest possible legal cell
	for(vector<bdd>::iterator i=cover->begin()+curr+1; i!=cover->end(); i++)
	{
		if( *i == bddfalse ) continue;
		bdd v = the_one | *i;
		if( v != the_one ) // v is a larger set
		{
			list<bdd> waitlist;
			if( _legal_cell(v, &waitlist, curr, *cover, R, C) ) 
			{ // v is also legal cell

				for(list<bdd>::iterator m=waitlist.begin(); m!=waitlist.end(); m++)
					_update_cover(*m, curr, cover);
				//assert( (v & (*cover)[curr]) == v );
			}
		}
		else // the_one includes *i
			*i=bddfalse;	
	}
	vector<bdd>::iterator new_end = remove(cover->begin(), cover->end(), bddfalse);
	cover->erase(new_end, cover->end());

	return (int)cover->size();
}

// The lower bound estimation based on R ONLY
void
simsup::_Ramsey(	vector<bdd>* const C,
			vector<bdd>* const I,	
			list<bdd>* const V,
			control_relation const& R ) const
{
	if( V->empty() )
	{
		C->clear();
		I->clear();
		return;
	}

	// find the pivot node v
	bdd v = V->front();
	V->pop_front();

	// find the neighborhoods of v
	list<bdd> N;
	list<bdd>::iterator tmp;
	for(list<bdd>::iterator i=V->begin(); i!=V->end();)
	{
		if( !R(v|*i) ) // *i a neighbor of v (notice: (*i, v) belongs to not R)
		{
			N.push_back(*i);
			tmp = i;
			i++;
			V->erase(tmp);
		}
		else
			i++;
	}
	// now *V has the non-neighborhood of v

	vector<bdd> C1, I1, C2, I2;
	_Ramsey(&C1,&I1,&N, R);
	_Ramsey(&C2,&I2, V, R);

	C1.push_back(v);
	I2.push_back(v);
	if( C1.size() >= C2.size() )
		C->swap(C1);
	else
		C->swap(C2);
	if( I1.size() >= I2.size() )
		I->swap(I1);
	else
		I->swap(I2);
}

int 
simsup::_list_cliques(control_relation const& R, vector<bdd>* const V) const
{
	for(vector<bdd>::iterator m=V->begin(); m!=V->end(); m++)
	{
		for(vector<bdd>::iterator i=m+1; i!=V->end(); i++)
		{
			bdd t = *m | *i;
			if( R(t) )
			{
				*m = t;
				*i = bddfalse;
			}
		}
		vector<bdd>::iterator new_end = remove(V->begin(), V->end(), bddfalse);
		V->erase(new_end, V->end());
	}

/*MA*/
	char f[10];
	int k=0;
	for(vector<bdd>::iterator m=V->begin(); m!=V->end(); m++, k++)
	{
		ostringstream os;
		os << k;
		strcpy(f, os.str().c_str());
		bdd_fnprintdot(f, *m);			
	}
/**/

	return V->size();
}

int 
simsup::_clique_removal(	control_relation const& R,
			list<bdd>* const V) const
{
	vector<bdd> C, I;
	int max_value = 0;
	while( !V->empty() )
	{
		for(vector<bdd>::iterator i=I.begin(); i!=I.end(); i++) 
		{
			list<bdd>::iterator li = ::find(V->begin(), V->end(), *i);
			V->erase(li);
		}

		list<bdd> W(*V);
		_Ramsey(&C,&I,&W, R);
		if( max_value < (int)C.size() )
			max_value = C.size();
	}

	return max_value;
}

/* We try to find the maximum clique of the graph featuring the relation !R (instead of R).
   The reason is that the graph of !R is MUCH much smaller than the graph of R.
	RETURN: an interval [a,b] that the REAL _lbe locates
   */
pair<int, int>
simsup::_lbe(bdd const& C) const
{
	control_relation R(*this, C);

	// list nodes, and draw the graph
	vector<bdd> s;
	_enumerate( &s, C );

	list<bdd> V(s.begin(), s.end());
	vector<bdd> clique, I;
	_Ramsey(&clique, &I, &V, R);
	//return make_pair(_clique_removal(R, &V), _list_cliques(R, &s));
	return make_pair(clique.size(), _list_cliques(R, &s));
}

///////////////////////////////////////////////////////////
// This _R3 will test all 3 conditions of the control cover. It's similar to the _legal_cell routine
// it will return true only if a and b satisfy R and all their downstream states also satisfy R
bool
simsup::_R3(	bdd const& a, bdd const& b,
		set<bdd, less_bdd>* const waitlist,
		control_relation const& R) const
{
	bdd ab = a | b;

	if( R(ab) ) 
	{
		if( waitlist->insert(ab).second == false ) // already in the waitlist
			return true; 

		bdd va, vb;
		for(set<event>::const_iterator e=Sigma().begin();
				e!=Sigma().end(); e++)
		{
			if( e->controllable() )
			{
				va = Delta(a,*e) & _C;
				vb = Delta(b,*e) & _C;
			}
			else
			{
				va = Delta(a,*e);
				vb = Delta(b,*e);
			}
			if( va != bddfalse && vb != bddfalse && va != vb ) // defined 
			{
				if( ! _R3(va, vb, waitlist, R) ) 
					return false;
			}
		}
		return true;
	}
	else 
		return false;
}

// call this with waitlist to be empty
// Here we construct a graph with the edge defined when two states are NOT mergeable.
// We test 3 conditions instead of two conditions given in surong's paper.
void 
simsup::_Ramsey3(	vector<bdd>* const C,
			vector<bdd>* const I,	
			list<bdd>* const V,
			set<bdd, less_bdd>* const waitlist,
			control_relation const& R ) const 
{
	if( V->empty() )
	{
		C->clear();
		I->clear();
		return;
	}

	// find the pivot node v
	bdd v = V->front();
	V->pop_front();

	// find the neighborhoods of v
	list<bdd> N;
	list<bdd>::iterator tmp;
	for(list<bdd>::iterator i=V->begin(); i!=V->end();)
	{
		if( _R3(v,*i, waitlist, R) ) 
		{
			N.push_back(*i);
			tmp = i;
			i++;
			V->erase(tmp);
		}
		else
			i++;
	}
	// now *V has the non-neighborhood of v

	vector<bdd> C1, I1, C2, I2;
	_Ramsey3(&C1,&I1,&N, waitlist, R);
	_Ramsey3(&C2,&I2, V, waitlist, R);

	C1.push_back(v);
	I2.push_back(v);
	if( C1.size() >= C2.size() )
		C->swap(C1);
	else
		C->swap(C2);
	if( I1.size() >= I2.size() )
		I->swap(I1);
	else
		I->swap(I2);
}

int 
simsup::_list_cliques3(control_relation const& R, vector<bdd>* const V, set<bdd, less_bdd>* const waitlist) const
{
	for(vector<bdd>::iterator m=V->begin(); m!=V->end(); m++)
	{
		for(vector<bdd>::iterator i=m+1; i!=V->end(); i++)
		{
			bdd t = *m | *i;
			if( _R3(*m, *i, waitlist, R) )
			{
				*m = t;
				*i = bddfalse;
			}
		}
		vector<bdd>::iterator new_end = remove(V->begin(), V->end(), bddfalse);
		V->erase(new_end, V->end());
	}
	return V->size();
}

int 
simsup::_clique_removal3(	control_relation const& R,
				list<bdd>* const V,
				set<bdd, less_bdd>* const waitlist) const
{
	vector<bdd> C, I;
	int max_value = 0;
	while( !V->empty() )
	{
		for(vector<bdd>::iterator i=C.begin(); i!=C.end(); i++) 
		{
			list<bdd>::iterator li = ::find(V->begin(), V->end(), *i);
			V->erase(li);
		}

		list<bdd> W(*V);
		_Ramsey3(&C,&I,&W, waitlist, R);
		if( max_value < (int)I.size() )
			max_value = I.size();
	}

	return max_value;
}

pair<int, int>
simsup::_lbe3(bdd const& C) const
{
	control_relation R(*this, C);
	set<bdd, less_bdd> waitlist;

	// list nodes, and draw the graph
	vector<bdd> s;
	_enumerate( &s, C );

	list<bdd> V(s.begin(), s.end());
	vector<bdd> clique, I;
	_Ramsey3(&clique, &I, &V, &waitlist, R);
	return make_pair(I.size(), _list_cliques3(R, &s, &waitlist));
	//return make_pair(_clique_removal3(R, &V, &waitlist), _list_cliques3(R, &s, &waitlist));
}

int 
simsup::_greedy_algo( void ) const
{
	control_relation R(*this, _C);
	set<bdd, less_bdd> waitlist;

	// list nodes, and draw the graph
	vector<bdd> s;
	_enumerate( &s, _C );
	
	typedef map<bdd, set<bdd,less_bdd>, less_bdd> mapbs;
	mapbs edge;
	for(vector<bdd>::iterator i=s.begin(); i!=s.end(); i++)
		for(vector<bdd>::iterator k=i+1; k!=s.end(); k++)
			if( _R3(*i,*k, &waitlist, R) )
			{
				edge[*i].insert(*k);
				edge[*k].insert(*i);
			}
cout << "Start searching..\n";
	mapbs::iterator pivotal;
	for(;;)
	{
		int max_num = 0;
		int tmp;
		// find a pivotal state that has the largest number of neigbors
		for(mapbs::iterator i=edge.begin(); i!=edge.end(); i++)
		{ // for each state
			tmp = i->second.size();
			if( max_num < tmp )
			{
				max_num = tmp;
				pivotal = i;
			}
		}

		if( max_num == 0 ) break;

		for(set<bdd,less_bdd>::iterator m=pivotal->second.begin(); m!=pivotal->second.end(); m++)
			edge[*m].erase(pivotal->first);
		edge.erase(pivotal);
	}

	return edge.size();
}
