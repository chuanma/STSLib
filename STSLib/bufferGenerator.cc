#include "bufferGenerator.h"

bufferGenerator::bufferGenerator(string const& name, int bufferSize, 
				vector<event_type>const& in, 
				vector<event_type>const& out,
				vector<event_type>const& self)
{
	set_name(name);  // name
	
	int stateSize = bufferSize+1;
	set_state_size(stateSize);  // state size
	
	automaton::state_type s=0;
	insert_marker_state(s); // marker state: state 0 is the marker state as well.
	
	// handle the events that incease buffer count. note that the last state has no events from 'in'
	trans_node tn;
	for(int i=0; i<stateSize-1; i++) {
		tn.t= i+1;
		for(vector<event_type>::const_iterator ei=in.begin(); ei!=in.end(); ei++) {
			tn.e=*ei;
			insert_transition(i,tn);
		}
	}
	
	// handle the events that decrease buffer count. note that state 0 has no events from 'out'
	for(int i=1; i<stateSize; i++) {
		tn.t= i-1;
		for(vector<event_type>::const_iterator ei=out.begin(); ei!=out.end(); ei++) {
			tn.e=*ei;
			insert_transition(i,tn);
		}
	}
	//handle selfloop events
	for(int i=0; i<stateSize; i++) {
		tn.t= i;
		for(vector<event_type>::const_iterator ei=self.begin(); ei!=self.end(); ei++) {
			tn.e=*ei;
			insert_transition(i,tn);
		}
	}
}

bufferGenerator::~bufferGenerator()
{
}
