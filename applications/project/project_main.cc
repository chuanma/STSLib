#include <fstream>
#include <sstream>
#include "timer.h"
#include <bdd.h>
#include "project.h"

int main(int argc, char* argv[] )
{
	vector<string> plant;
	vector<string> spec;
	string output_file;
	bool null_flag;
	set<event> Sigma;


	arg_project(argc, argv, &plant,  &spec, &null_flag, &Sigma, &output_file);

	// initialize sts and call supreduce
	timer t;
	t.set_start_time();
	{
		project a(plant,null_flag, Sigma, spec);
		if( !a.compute(output_file) )
			cout << "The resulting DES is empty! No DES is saved in " << output_file << endl;
		else
			cout <<"The resulting DES is saved in " << output_file << endl;

	}
	cout << "The time spent in project is " << t.diff_time() << " seconds." << endl;

	return 0;
}

