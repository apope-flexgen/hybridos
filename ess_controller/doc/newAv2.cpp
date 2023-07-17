#include<iostream>
#include<string>
#include<map>
#include<vector>

using namespace std;


// setVal will trigger actions
// actAv["shift"] = 2;
// actAv["inValue"] = 0;
// actAv["outValue"] = "This is an out value";
// actAv["uri"] = "/a/b/c:d@foo";
// make up a vector of actAV's
// actVec.push_back (actAv)

// av->addAction("onSet", "enum", actVec)

// after this we will have a multi dimensional vmap with params and actions
// all over the place.  

//g++ -std=c++11 -o av doc/newAv.cpp
int split(vector<string> &output, const string& s, char seperator)
{
    int i = 0;
    string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(seperator, pos)) != string::npos)
    {
        string substring( s.substr(prev_pos, pos-prev_pos) );
		if(substring.size()> 0)
		{
        	output.push_back(substring);
			i++;
		}
        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

    return i;
}

class AssetVar;
class AssetVar {
private:
	int valueint;
    char* valuestring;
    bool valuebool;
    double valuedouble;
    string name;
    string atype;
	AssetVar* parent;
	AssetVar* child;
    vector<AssetVar*>aVec;
	map<string,AssetVar *> Params;
	

public:
	AssetVar(string _name, AssetVar *p = nullptr, AssetVar *c = nullptr);
	
	// This is automatically called when '+' is used with
	// between two Complex objects
	void operator = (int  val) {
	 	valueint = val;
        valuedouble = val;
        atype = "int";
	}
	void operator = (double  val) {
	 	valuedouble = val;
         valueint = val;
        atype = "double";
	}
	AssetVar &operator [] (string  pname) {
	 	if(Params.find(pname)== Params.end())
         {
             Params[pname]= new AssetVar(pname, this);
             Params[pname]->atype = "Avar";
			 aVec.push_back(Params[pname]);
         }
         return *Params[pname];
	}
    string getName(){return name;}
    void show(int level = 0);// { cout << name << " type :" << atype << " int :" <<valueint << " dbl :"<< valuedouble<<endl; }
    void showVec(int level = 0);// { cout << name << " type :" << atype << " int :" <<valueint << " dbl :"<< valuedouble<<endl; }
    void showKids(int level = 0);// { cout << name << " type :" << atype << " int :" <<valueint << " dbl :"<< valuedouble<<endl; }
};

AssetVar::AssetVar(string _name, AssetVar * _p , AssetVar * _c)  
{
	name = _name; 
	parent = _p;
	child = _c;
	valueint = 0;
	valuedouble = 0.0;
	valuebool = 0;
	valuestring = nullptr;
	atype = "none";
	if(parent)
	{
		parent->child = this;
	}
	
}


void AssetVar::show(int level) 
{ 
	if((level >=0) && parent)
	{
		parent->show(level+1);
	}
	if(level >= 0)
		cout << " ["<<level<<"]";
	cout <<name 
	     << " type :" << atype << " int :" <<valueint << " dbl :"<< valuedouble; 
	if ((level <= 0))
	{
		cout << endl;
	}
	
}

void AssetVar::showVec(int level) 
{ 
 for (auto  x: aVec)
 {
	 x->show(-1);
 }
}
void AssetVar::showKids(int level) 
{
  	int i = level;
 	while (i> 0)
 	{
		cout << "   ";
	 	i--;
 	}
 	cout << name << endl; 
 	if (child)
 		child->showKids(level+1);
}


// we loose the varsmap instead root everything from a base av.
int main()
{
	std::vector<std::string> svec; 
	AssetVar avint("intAv");
	AssetVar avdbl("dblAv");
	AssetVar avtest("testAv");
	AssetVar avkid("Avkid", &avtest );
	
	AssetVar*vmap = &avtest;



	avint = 22;
	// adds to vectors (old Params)
    avint["MaxVal"] = 25;
    avint["MinVal"] = 15;

	avdbl = 22.3;
	avint.show();
	avint["MaxVal"].show();
	avdbl.show();
    avtest["one"]["two"]["three"]=  3;   //deep param map
    avtest["one"]["two"]["three"].show();
	avint.showVec();
	// avtest.find("/one/two")  will split string and cycle through vecs
	avtest["one"]["two"].showKids();
	avtest.showKids();
	split(svec, "/one/two/three", '/');
	for (auto s : svec)
	{
		cout << "[" <<s <<"]" << endl;
	}

}
