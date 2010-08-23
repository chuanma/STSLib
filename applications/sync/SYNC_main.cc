#include <fstream>
#include <sstream>
#include "timer.h"
#include <bdd.h>
#include "SYNC.h"

int main(int argc, char* argv[] )
{
	vector<string> plant;
	string output_file;

	arg_SYNC(argc, argv, &plant, &output_file);
	SYNC a(plant);
	a.compute(output_file);

	return 0;
}

