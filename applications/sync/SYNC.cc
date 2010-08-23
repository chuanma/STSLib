#include "SYNC.h"

/* read the command line and get the list of
	plant DES components
	(un)observable event set,
	name of the output_file
*/
bool arg_SYNC(	int argc, char* argv[],
		vector<string>* const plant,
		string* const output_file)
{
	plant->clear();
	output_file->clear();

	// SYNC -p P*.des -o output_file
  if(argc == 1)
  {
		cerr << "Usage: SYNC -o output_DES_file -p plant_DES_files" << endl;
		cerr << "Wildcards like * and ? can be used for file names, e.g., m* includes all DES files starting with m." << endl;
		cerr << "All items must be seperated by whitespace." << endl;
		exit(1);
	}

	int flag = 4; // 1 for plant and 2 for output_file
	string s(argv[1]);

	if( s == "-p" || s == "-P" )
		flag = 1;
	else if( s == "-o" || s == "-O" )
		flag = 2;
	else // wrong format
	{
		cerr << "Usage: SYNC -o output_DES_file -p plant_DES_files" << endl;
		cerr << "Wildcards like * and ? can be used for file names, e.g., m* includes all DES files starting with m." << endl;
		cerr << "All items must be seperated by whitespace." << endl;
		exit(1);
	}
	for(int i=2; i<argc; i++)
	{
		s = argv[i];
		if( s == "-p" || s == "-P" )
			flag = 1;
		else if( s == "-o" || s == "-O" )
			flag = 2;
		else // file names
		{
			if( flag == 2 ) // no need to check existance
				*output_file = s;
			else if( flag == 1 && file_existed(s) ) // plant des files
				plant->push_back(s);
			else // Wrong
				open_error(s);
		}
	}

	if(  output_file->empty() || plant->empty() )
	{
		cerr << "Usage: SYNC -o output_DES_file -p plant_DES_files" << endl;
		cerr << "Wildcards like * and ? can be used for file names, e.g., m* includes all DES files starting with m." << endl;
		cerr << "All items must be seperated by whitespace." << endl;
		exit(1);
	}

	/* add this line if proper to check the existence of *output_file
	if( file_existed(*output_file) )
	{
		cout << *output_file << " exists. Do you want to overwrite?[y/n]" << endl;
		char c;
		cin >> c;
		if( c == 'n' || c == 'N' )
			exit(1);
	}
	*/

	//Step 2: check if plant has same components appearing more than once 
	set<string> set_p(plant->begin(), plant->end());
	if( set_p.size() != plant->size() )
	{
		plant->clear();
		copy(set_p.begin(), set_p.end(), back_inserter(*plant));
	}
	return true;
}

bool
SYNC::_procedure( ostream& out ) const
{
	/* First build the basicST_map recording all transition info */
	set<bdd, less_bdd> 		current;
	set<bdd, less_bdd> 		next;
	map<bdd, vec_trans, less_bdd>	basicST_map;

	bdd qo = Po();
	if( qo == bddfalse )
		return false;
	current.insert(qo);

	trans	 t;
	set<event> s1; // s1 has the events defined in the transition structure.
	while( !current.empty() )
	{
		next.clear();
		for(set<bdd,less_bdd>::const_iterator i = current.begin();
					i != current.end(); i++)
		{ // for each current basicST
			if( basicST_map.find(*i) != basicST_map.end() )
				continue; // *i has been computed
			basicST_map[*i]; // build an entry saying it's computed
					// has to add this line because this
					// state *i may NOT have exiting trans
			for(set<event>::const_iterator e = Sigma().begin();
					e != Sigma().end(); e++)
			{
				t.second = Delta(*i, *e);
				if( t.second != bddfalse ) // nonempty target
				{
					next.insert(t.second);
					t.first = *e;
					basicST_map[*i].push_back(t);
					s1.insert(*e); // *e appears in trans
				}
			}
		}
		current.swap(next);
	}

	// now find the marker basicST
	bdd m = Pm();
	set<int> markers;
	for(map<bdd,vec_trans, less_bdd>::const_iterator 
			i = basicST_map.begin();
			i != basicST_map.end(); i++)
		if( (i->first & m) != bddfalse ) // *i is a marker basicST
			markers.insert(i->first.id());
	if( markers.empty() )
		return false;

	// now find if s1 == Sigma(), if not, add a state with missing events
	if( s1 != Sigma() )
	{
		set<event> s2;
	  	set_difference(Sigma().begin(), Sigma().end(),
				s1.begin(), s1.end(),
				inserter(s2, s2.begin()) );

		// for each missing event e, add a selfloop of 
		// bddfalse --e--> bddfalse
		basicST_map[bddfalse];// the bddfalse is new in basicST_map
		t.second = bddfalse; 
		for(set<event>::const_iterator e2=s2.begin(); 
				e2!=s2.end(); e2++ )
		{
			t.first = *e2;
			basicST_map[bddfalse].push_back(t);
		}
		// add a transition qo--e-->bddfalse to make bddfalse reachable
		basicST_map[qo].push_back(t);
	}

	/* write to a .sts file */
	_write_flat_STS(out, ST().root(), basicST_map, qo.id(), markers, Sigma());

	return true;
}
