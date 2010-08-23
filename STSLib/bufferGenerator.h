#ifndef BUFFERGENERATOR_H_
#define BUFFERGENERATOR_H_

/*
 * This class generates a buffer automaton where both of its initial state and marker state are state 0
 */
 
#include "automaton.h"

class bufferGenerator : public automaton
{
public:
	// note that state size is bufferSize+1;
	bufferGenerator(string const& name, int bufferSize, 
			vector<event_type> const& in, 
			vector<event_type>const& out,
			vector<event_type>const& self);
	virtual ~bufferGenerator();

};

#endif /*BUFFERGENERATOR_H_*/
