#include <functional>
#include <string>
#include <sstream>
#include <fdd.h>

using namespace std;

class cat : public binary_function<string, string, string> {
public:
	cat(string const& d) { _delimiter = d; }
	string operator()(string s1, string s2) const
	{
		return (s1 + _delimiter + s2);
	}
private:
	string _delimiter;
};

/* string --> long integer 
   The return pair: first is the integer and second is to see if 
   the translation is right by comparing it with s.end(). e.g.,
   	if (the_iterator == s.end() ) then the translation is successful.
*/
class str2l {
public:
	pair<long, string::iterator> operator()(string s) const
	{
		char * end;
		int l = std::strtol(s.c_str(), &end, 10);
		string::iterator i = s.begin() + (end - &s[0]);
		return make_pair(l,i);
	}
};
class a2l {
public:
	long operator()(string s) const
	{
		return std::atol(s.c_str());
	}
};

/* long ---> string */
class l2str {
public:
	string operator()(long l)
	{
		ostringstream o;
		o << l;
		return o.str();
	}
};

class less_bdd {
public:
	bool operator()( bdd a, bdd b) const
	{
		return a.id() < b.id(); 
	}
};
