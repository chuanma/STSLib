#include "bdd_synthesis.h"
#include "automaton.h"
#include <fstream>
#include <sstream>
#include "timer.h"
#include <bdd.h>

int main(int argc, const char* argv[])
{
  if( argc != 3 ) {
    cout <<" Usage: nbc plant.sts specification\n";
    exit(1);
  }
	ifstream ifile(argv[1]);
	if( !ifile )
		open_error("plant.sts");

	//istringstream ins("{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}");
        ifstream ins(argv[2]);
	if( !ins )
		open_error("spec");
        

	sts agv(ifile, ins);
	cout << agv << endl;

	bdd_synthesis syn(agv);
	timer t;
	syn.compute();
	cout << "The time spent in synthesis is " << t.diff_time() << " seconds." << endl;
	cout << syn << endl;	
	
	syn.print_control_functions();
	for(set<event>::const_iterator et=syn.Sigma_c().begin();
				et!=syn.Sigma_c().end(); et++)
  {
    string fn(et->label());
    fn += ".elig";
		char* f = new char[fn.size()+1];
		strcpy(f, fn.c_str());
		bdd_fnprintdot(f, syn.Elig(*et));			
  }

	return 0;
}
