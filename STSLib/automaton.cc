#include "automaton.h"
#include <math.h>
#include <fstream>
#include <sstream>

int
automaton::_num_of_transition(void) const
{
	int sz=0;
	for(int i=0; i<_t.size(); i++)
		sz += _t[i].size();
	return sz;
}
	
automaton::automaton(ifstream& in, string const& name) : _name(name)
{
	assert( sizeof(int) == 4 );

	/* skip the text header */
	while( !in.eof() )
	{
		char dummy;
		in.read(&dummy, sizeof(char));
		if( dummy == (char)0x1a )
			break;
	}

	/* read the signature, 7 bytes */
	char c[8];
	in.read(c,7);
	c[7] = '\0';
	if( strcmp("Z8^0L;1", c) != 0 )
	{
		cerr << "DES(" << _name << ")";
		cerr << " file reading: signature is different.\n";
		exit(1);
	}

	/* read the endian */
	int endian;
	in.read((char*)&endian, sizeof(int));
	if( endian != 0xff00aa55 )
	{
		cerr << "DES(" << _name << ")";
		cerr << " file reading: endian is incorrect.\n";
		exit(1);
	}

	/* find the block with block-type == 0 */
	int block_type, data_size;
	do {
		in.read((char*)&block_type, sizeof(int));
		in.read((char*)&data_size, sizeof(int));
		if( block_type == 0 ) break;
		else in.seekg(data_size, ios::cur);
	}while ( block_type != -1 );
	if( block_type == -1 )
	{
		cerr << "DES(" << _name << ")";
		cerr << " file reading: block-type is incorrect.\n";
		exit(1);
	}

	/* read data */
	/* read the # of states */
	int nstate;
	in.read((char*)&nstate, sizeof(int));
	_t.resize(nstate);

	/* read the initial state */
	int initial_state;
	in.read((char*)&initial_state, sizeof(int));
	if( initial_state == -1 )
	{
		cerr << " This is a DAT file! Expecting a DES file.\n";
		exit(1);
	}
	assert(initial_state == 0);

	/* read the marker states */
	while(1)
	{
		int dummy;
		in.read((char*)&dummy, sizeof(int));
		if( dummy == -1 )
			break;
		_Qm.insert(dummy);
	}

	/* read the transitions */
	while(1)
	{
		int src;
		unsigned short ntrans;
		in.read((char*)&src, sizeof(int));
		if(src == -1) break;
		in.read((char*)&ntrans, sizeof(unsigned short));

		trans_node tn; 
		int data;
		for(int i=0; i<ntrans; i++)
		{
			in.read((char*)&data, sizeof(int));
			tn.t = data & 0x3fffff;
			tn.e = data >> 22;
			_t[src].insert(tn);
		}
	}
}

// from a singleton .sts file
// Usually we don't want to build a memory-inefficient CTCT model
// So this constructor has no use except when the STS file g
// was built from a set of TCT .des files and the user would like
// to compare the results computed from STS with the ones from TCT.

// Assumption: 
//	the event label in STS is a number, which follows the rules
//	of TCT regarding to the (un)controllable events. That is, odd
//	number for controllable event.
automaton::automaton(istream& in)
{
	state_tree st;
	in >> st;
	if( st.size(OR) != 1 || st.size(AND) !=0 )
	{
		cerr << "Conversion from STS to TCT: ";
		cerr << "The STS with root " << st.root();
		cerr << " should have a singleton holon.";
		exit(1);
	}
	// init _name
	_name = st.root().label();

	state s;
	in >> s;
	assert( s == st.root());

	holon h;
	in >> h;

	subST STo;
	set<subST> STm;
	in >> STo;
	in >> STm;


	state qo = *STo.begin();
	map<state, state_type> state_code; // code states to integers [0,n)
	vector<state> state_list; // integers to states
	state_code[qo] = 0;
	state_list.push_back(qo);
	state_type k = 1;
	for(set<state>::const_iterator i=st.X().begin(); 
			i!=st.X().end(); i++) 
		if( *i != st.root() && *i != qo )
		{
			state_list.push_back(*i);
			state_code[*i] = k;
			assert( state_list[k] == *i );
			k++;
		}

	// init _Qm
	for(subST::iterator it=STm.begin()->begin(); 
			it!=STm.begin()->end(); it++)
		_Qm.insert(state_code[*it]); 

	// init _t
	//vector<set<trans_node> > _t; 
	_t.resize(state_list.size()); 
	trans_node tn;
	set<pair<event,state> > p;	
	for(state_type i=0; i< _t.size(); i++)
	{
		h.find_trans_node_for_TCT(state_list[i], &p);
		for(set<pair<event,state> >::iterator k=p.begin();
			k!=p.end(); k++)
		{
			tn.e = atoi(k->first.label().c_str());
			tn.t = state_code[k->second];
			_t[i].insert(tn);
		}
	}
}

automaton::automaton(string const& name, set<event>const& allEvents) 
{
	this->set_name(name);
	this->set_state_size(1);
	this->insert_marker_state(0);
	trans_node tn;
	tn.t=0;
	for(set<event>::const_iterator ei=allEvents.begin();
		ei!=allEvents.end(); ei++) {
		tn.e = atoi(ei->label().c_str());
		insert_transition(0,tn);
	}
}

//add selfloops to all states in this automaton
void	
automaton::selfloop(set<event> const& events)
{
	trans_node tn;
	for(int i=0; i<_t.size(); i++) {
		tn.t=i;
		for(set<event>::const_iterator ei=events.begin();
			ei!=events.end(); ei++) {
				tn.e=atoi(ei->label().c_str());
				insert_transition(i,tn);
			}
	}
}

// operation
bool	
automaton::insert_transition(state_type s, trans_node const& tn)
{
	if( s < 0 || s >= _t.size() ) return false;

	return _t[s].insert(tn).second;
}

bool	
automaton::erase_transition(state_type s, trans_node const& tn)
{
	if( s < 0 || s >= _t.size() ) return false;

	set<trans_node>::iterator i= _t[s].find(tn);
	if( i == _t[s].end() ) 
		return false;
	else
	{
		_t[s].erase(i);
		return true;
	}
}

bool	
automaton::insert_marker_state(state_type s)
{
	assert( s>=0 && s<_t.size());
	return _Qm.insert(s).second;
}

bool	
automaton::erase_marker_state(state_type s)
{
	assert( s>=0 && s<_t.size());
	set<state_type>::iterator i= _Qm.find(s);
	if( i == _Qm.end() ) // not find
		return false;
	else
	{
		_Qm.erase(i);
		return true;
	}
}


// output: simple because TCT has already given a nice one
ostream& operator<<(ostream& out, automaton const& a)
{
	out << "The information for the DES " << a._name << endl;
	out << "-----------------------------------------" << endl;
	out << "The # of states: " << a._t.size() << endl;
	out << "The marker states are: " << a._Qm << endl;
	out << endl;
	out << "The # of transitions: " << a._num_of_transition() << endl;

	for(int i=0; i<a._t.size(); i++)
		for(set<automaton::trans_node>::const_iterator it= a._t[i].begin();
				it != a._t[i].end(); it++)
		{
			out << "[ " << i << " ";
			out << it->e << " ";
			out << it->t << " ]" << endl;
		}

	return out;
}

// write to CTCT .des file
// return the filename
string 
automaton::write_des(string const& fn) const
{ // 
	string filename;
	if( fn == string() )
		filename = _name + ".des";
	else
		filename = fn;
	
	ofstream out(filename.c_str(), ios::binary);
	if( !out )
	{
		cerr << "Can't create file: " << filename << endl;
		exit(1);
	}

	/* write the text header */
	string text = "This DES file is created by MA, Chuan's STS package.";
	out.write((char*)text.c_str(), text.size());
	out.put( (char)0x1a );

	/* write the signature, 7 bytes */
	out.write("Z8^0L;1", 7);

	/* write the endian */
	int endian = 0xff00aa55;
	out.write((char*)&endian, sizeof(int));

	/* write the block-type and data-size*/
	int block_type = 0; 
	int data_size = 0;
	out.write((char*)&block_type, sizeof(int));
	ios::pos_type data_size_pos = out.tellp();
	out.write((char*)&data_size, sizeof(int));

	/* write data */
	/* write the # of states */
	int nstate = _t.size();
	out.write ((char*)&nstate, sizeof(int));

	/* write the initial state */
	int initial_state = 0;
	out.write((char*)&initial_state, sizeof(int));

	/* write the marker states */
	vector<int> q;
	copy(_Qm.begin(), _Qm.end(), back_insert_iterator<vector<int> >(q));
	out.write((char*)&q[0], sizeof(int)*q.size());
	int end_of_marker_states = -1;
	out.write((char*)&end_of_marker_states, sizeof(int));

	/* write the transitions */
	for(int i=0; i<_t.size(); i++)
	{
		int ntrans = _t[i].size();
		if( ntrans == 0 ) continue;
		out.write((char*)&i, sizeof(int)); // write exit state
		//ccformat!! out.write((char*)&ntrans, sizeof(int));// write the number of trans.
		out.write((char*)&ntrans, sizeof(unsigned short));// write the number of trans.

		int data;
		for(set<trans_node>::const_iterator it=_t[i].begin();
					it!=_t[i].end(); it++)
		{
			data = (it->t & 0x003fffff) | (it->e <<22);
			out.write((char*)&data, sizeof(int));
		}
	}
	int end_of_transition_state = -1;
	out.write((char*)&end_of_transition_state, sizeof(int));
	// no vocal states 
	out.write((char*)&end_of_transition_state, sizeof(int));

	// update the data size 
	ios::pos_type cur_pos = out.tellp();
	out.seekp(data_size_pos);
	data_size = cur_pos - data_size_pos;
	out.write((char*)&data_size, sizeof(int));
	out.seekp(0, ios::end);
	
	
	block_type = -1; 
	data_size = 8;
	int zeros[2];
	zeros[0] = zeros[1] = 0;	
	out.write((char*)&block_type, sizeof(int));
	out.write((char*)&data_size, sizeof(int));
	out.write((char*)zeros, sizeof(int)*2);

	out.close();
	
	return filename;
}

// write to STS file. Filename decided by _name
// the boolean value "f" decide if it should be saved as a MEMORY or not
// return the filename
string 
automaton::write_sts(bool f, string const& fn)  const
{
	string filename;
	if( fn == string() )
		filename = _name + ".sts";
	else
		filename = fn;

	ofstream out(filename.c_str());
	if( !out )
	{
		cerr << "Can't create file: " << filename << endl;
		exit(1);
	}

	/* write the state tree */
	// The simple state lable is defined as "_name.id", for example,
	// the state 5 in automaton "M1" is encoded as "M1.5". This is
	// required by STS definition as no two states on the tree are
	// allowed to have the same label.
	// Bug: M1.5 isn't okay if state size is larger than 10. In that case,
	// M1.10 is listed before M1.2 in any state set, because state set is
	// ordered in terms of state labels (strings). So we need to put padding
	// zeros to make sure that states are ordered as we expected.
	int pad_length = (int) log10((double)_t.size()) + 1;
	vector<string> children_string;
	for(int i=0; i<_t.size(); i++)
	{
		ostringstream oss;
		//oss << _name << "." << i;
		oss << _name << "." << setfill('0') << setw(pad_length) << i;
		children_string.push_back(oss.str());
	}
	set<string> 
		expansion(children_string.begin(), children_string.end());
	out << "root = " << _name << endl;
	out << "{" << endl;
	out << _name << " = OR "<< expansion << endl;
	out << "}" << endl;

	/* write the holon */
	out << _name << endl; // the OR state _name;
	// build the controllable set of event, uncontrollable set of event
	set<event> con_events, unc_events;
	for(int i=0; i<_t.size(); i++)
		for(set<trans_node>::const_iterator it=_t[i].begin();
					it!=_t[i].end(); it++)
		{
			ostringstream o;
			o << it->e;
			if( controllable(it->e) )
			{
				event t(o.str(), con);
				con_events.insert(t);
			}
			else
			{
				event t(o.str(), unc);
				unc_events.insert(t);
			}
		}
	out << con_events << endl; // write contr. events
	out << unc_events << endl; // write uncontr. events
	// write transition set
	out << "{" << endl;
	ostringstream o;
	for(int i=0; i<_t.size(); i++)
		for(set<trans_node>::const_iterator it=_t[i].begin();
					it!=_t[i].end(); it++)
		{
			o << "[" << children_string[i] << ",";
			o << it->e << ",";
			o << children_string[it->t] << "]  ";
		}	
	out << o.str() << endl;
	out << "}" << endl;

	out << "{" << children_string[0] << "}" << endl;  // the initial state
	out << "{ { ";
	for(set<state_type>::const_iterator i=_Qm.begin(); i!=_Qm.end(); i++)
		out << children_string[ *i ] << " ";
	out << "} }" << endl;	// the marker states
	if( f == PLANT )
		out << "{}" << endl; // not a memory
	else if ( f == MEMORY )
		out << "{" << _name << "}" << endl;

	out.close();
	
	return filename;
}

ostream& 
automaton::write_sts(bool f, ostream& out)  const
{
	/* write the state tree */
	// The simple state lable is defined as "_name.id", for example,
	// the state 5 in automaton "M1" is encoded as "M1.5". This is
	// required by STS definition as no two states on the tree are
	// allowed to have the same label.
	int pad_length = (int) log10((double)_t.size()) + 1;
	vector<string> children_string;
	for(int i=0; i<_t.size(); i++)
	{
		ostringstream oss;
		//oss << _name << "." << i;
		oss << _name << "." << setfill('0') << setw(pad_length) << i;
		children_string.push_back(oss.str());
	}
	set<string> 
		expansion(children_string.begin(), children_string.end());
	out << "root = " << _name << endl;
	out << "{" << endl;
	out << _name << " = OR "<< expansion << endl;
	out << "}" << endl;

	/* write the holon */
	out << _name << endl; // the OR state _name;
	// build the controllable set of event, uncontrollable set of event
	set<event> con_events, unc_events;
	for(int i=0; i<_t.size(); i++)
		for(set<trans_node>::const_iterator it=_t[i].begin();
					it!=_t[i].end(); it++)
		{
			ostringstream o;
			o << it->e;
			if( controllable(it->e) )
			{
				event t(o.str(), con);
				con_events.insert(t);
			}
			else
			{
				event t(o.str(), unc);
				unc_events.insert(t);
			}
		}
	out << con_events << endl; // write contr. events
	out << unc_events << endl; // write uncontr. events
	// write transition set
	out << "{" << endl;
	ostringstream o;
	for(int i=0; i<_t.size(); i++)
		for(set<trans_node>::const_iterator it=_t[i].begin();
					it!=_t[i].end(); it++)
		{
			o << "[" << children_string[i] << ",";
			o << it->e << ",";
			o << children_string[it->t] << "]  ";
		}	
	out << o.str() << endl;
	out << "}" << endl;

	out << "{" << children_string[0] << "}" << endl;  // the initial state
	out << "{ { ";
	for(set<state_type>::const_iterator i=_Qm.begin(); i!=_Qm.end(); i++)
		out << children_string[ *i ] << " ";
	out << "} }" << endl;	// the marker states
	if( f == PLANT )
		out << "{}" << endl; // not a memory
	else if ( f == MEMORY )
		out << "{" << _name << "}" << endl;

	return out;
}

/* convert an automaton to a STS with singleton holon. 
 * Set the only holon to be memory if the bool arg. is "MEMORY"
 */
void
automaton::to_sts(sts* const g, bool f)
{
	stringstream ss;
	write_sts(f, ss);
	istringstream ins("{ } { }"); // no spec file
	g->read(ss,ins);
}
