#ifndef SYNC_H
#define SYNC_H

#include "tct_engine.h"
#include <algorithm>

// arg_SYNC reads the file names of both plant components and the event set, also the output file name
bool arg_SYNC(	int argc, char* argv[],
		vector<string>* const plant,
		string* const output_file);

class SYNC: public tct_engine {
public:
	SYNC(	vector<string> const& plant, 
		vector<string> const& spec = vector<string>() ) : tct_engine(plant,spec) {}
	virtual ~SYNC() {}
private:
	bool _procedure(ostream& out) const; // return false if emtpy returned automaton
};

#endif /*SYNC_H*/
