#include "project.h"

/* read the command line and get the list of
	plant DES components
	(un)observable event set,
	name of the output_file
*/
bool arg_project(	int argc, char* argv[],
			vector<string>* const plant,
			vector<string>* const spec,
			bool* const null_flag, // true if *Sigma is to be erased
			set<event>* const Sigma,
			string* const output_file)
{
	plant->clear();
	spec->clear();
	*null_flag = true;
	Sigma->clear();
	output_file->clear();

	if( argc == 1 ) // need to input plant DES and events one by one
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

		cout << "Enter either" << endl
		     << "  NULL (list of event labels erased by projection)"
		     << "or" << endl
		     << "  IMAGE (event labels retained)" << endl
		     << "NULL ? (*y/n)";
		char c;
		cin >> c;
		if( c == 'n' || c == 'N' )
			*null_flag = false;
		else
			*null_flag = true;

		if( *null_flag == true )
			cout << "Enter list of event labels to be erased:" << endl;
		else
			cout << "Enter list of event labels to be retained:" << endl;
		cout << "  terminate list with -1." << endl;
		for(;;)
		{
			string fn;
			int ev;
			bool con;
			cout << "\n> ";
			cin >> fn;
			ev = atoi(fn.c_str());
			if( ev == -1 ) break;
			if( (ev%2) == 0 )
				con = false;
			else
				con = true;
			Sigma->insert(event(fn, con));
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
	else // simsup -p P*.des -s spec*.des -null (or -image) -e events -o output_file
	{
		int flag = 4; // 1 for plant and 0 for specsand 2 for output_file and 3 for events
		string s(argv[1]);
		if( s == "-p" || s == "-P" )
			flag = 1;
		else if( s == "-s" || s == "-S" )
			flag = 0;
		else if( s == "-o" || s == "-O" )
			flag = 2;
		else if( s == "-e" || s == "-E" )
			flag = 3;
		else if( s == "-null")
			*null_flag = true;
		else if( s == "-image")
			*null_flag = false;
		else // wrong format
		{
			cerr << "Usage: project -o output_DES_file -p plant_DES_files -s spec_DES_files -e list_of_events -null(or -image)" << endl;
			cerr << "Wildcards like * and ? can be used for file names, e.g., m* includes all DES files starting with m." << endl;
			cerr << "All items must be seperated by whitespace." << endl;
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
			else if( s == "-e" || s == "-E" )
				flag = 3;
			else if( s == "-null")
				*null_flag = true;
			else if( s == "-image")
				*null_flag = false;
			else // file names
			{
				if( flag == 2 ) // no need to check existance
					*output_file = s;
				else if( flag == 1 && file_existed(s) ) // plant des files
					plant->push_back(s);
				else if( flag == 0 && file_existed(s) ) // plant des files
					spec->push_back(s);
				else if( flag == 3 )// event 
				{
					int ev;
					ev = atoi(s.c_str());
					if( (ev%2) == 0 )
						Sigma->insert(event(s,false));
					else
						Sigma->insert(event(s,true));
				}
				else // Wrong
					open_error(s);
			}
		}
		if(  output_file->empty() || plant->empty() )
		{
			cerr << "Usage: project -o output_DES_file -p plant_DES_files -s spec_DES_files -e list_of_events -null(or -image)" << endl;
			cerr << "Wildcards like * and ? can be used for file names, e.g., m* includes all DES files starting with m." << endl;
			cerr << "All items must be seperated by whitespace." << endl;
			exit(1);
		}
	}

	//Step 2: check if plant has same components appearing more than once 
	set<string> set_p(plant->begin(), plant->end());
	if( set_p.size() != plant->size() )
	{
		plant->clear();
		copy(set_p.begin(), set_p.end(), back_inserter(*plant));
	}
	set<string> set_s(spec->begin(), spec->end());
	if( set_s.size() != spec->size() )
	{
		spec->clear();
		copy(set_s.begin(), set_s.end(), back_inserter(*spec));
	}
cout << "List of plant DES: " << set_p << endl;
cout << "List of spec DES: " << set_s << endl;
cout << "List of events: " << *Sigma << endl;

	return true;
}

bool
project::_procedure( ostream& out ) const
{
	if (_Sigma_o.empty())
		return false;

	set<event> Sigma_uo; // unobservable events
	set_difference(	Sigma().begin(), Sigma().end(),
			_Sigma_o.begin(), _Sigma_o.end(),
			inserter(Sigma_uo, Sigma_uo.begin()) );

	/* First build the basicST_map recording all transition info */

	set<bdd, less_bdd> 		current;
	set<bdd, less_bdd> 		next;
	map<bdd, vec_trans, less_bdd>	basicST_map;

	bdd qo;
	if( Sigma_uo.empty() )
		qo = Po() & _C;
	else
		qo = R(_C, Po(), Sigma_uo); // If Sigma_uo is empty, R(.) will assume it's the default case where Sigma_uo = Sigma()!. So we have to check before calling R(.) and to see if Sgima_uo is emtpy indeed.
	if( qo == bddfalse )
		return false;
	current.insert(qo);

	trans	 t;
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
			for(set<event>::const_iterator e = _Sigma_o.begin();
					e != _Sigma_o.end(); e++)
			{
				t.second = Delta(*i, *e) & _C;
				if( t.second != bddfalse ) // nonempty target
				{
					if( !Sigma_uo.empty() )
						t.second = R(_C, t.second, Sigma_uo); 
					next.insert(t.second);
					t.first = *e;
					basicST_map[*i].push_back(t);
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

	/* write to a .sts file */
	_write_flat_STS(out, ST().root(), basicST_map, qo.id(), markers, _Sigma_o);

	return true;
}
