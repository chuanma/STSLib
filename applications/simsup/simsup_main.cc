#include <fstream>
#include <sstream>
#include "timer.h"
#include <bdd.h>
#include "simsup.h"

int main(int argc, char* argv[] )
{
	vector<string> plant;
	vector<string> spec;
	string output_file;

	arg_handler( argc, argv, &plant, &spec, &output_file );

	// initialize sts and call supreduce
	timer t;
	t.set_start_time();
	{
		// run the supreduce
		simsup a(plant,spec);
		if( !a.compute(output_file) )
		{
			cerr << "The controller is empty! No DES is saved in " << output_file << endl;
			exit(1);
		}
		else
			cout <<"The simplifed controller is saved in " << output_file << endl;

		// find the lower bound estimation
		//pair<int, int> n = a.lbe();
		pair<int, int> n = a.lbe3();
		cout << "The lower bound estimation is in the range: " << "[" << n.first << "," << n.second << "]" << endl;
	}
	cout << "The time spent in simsup is " << t.diff_time() << " seconds." << endl;

	return 0;
}

