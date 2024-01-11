#include<iostream>
#include<string>
#include<map>

//g++ -std=c++11 -o av doc/newAv.cpp

using namespace std;
class AssetVar;
class AssetParam {
private:
	int valueint;
    char* valuestring;
    bool valuebool;
    double valuedouble;
    string name;
    string atype;
    AssetVar* parent;
public:
	AssetParam(AssetVar* par, string _name)  {parent = par,name = _name;}
	
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
    void show();// { cout << parent->getName()<< "["<< name << "] type :" << atype << " int :" <<valueint << " dbl :"<< valuedouble<<endl; }
};

class AssetVar {
private:
	int valueint;
    char* valuestring;
    bool valuebool;
    double valuedouble;
    string name;
    string atype;
    map<string,AssetParam *> Params;

public:
	AssetVar(string _name)  {name = _name;}
	
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
	AssetParam &operator [] (string  pname) {
	 	if(Params.find(pname)== Params.end())
         {
             Params[pname]= new AssetParam(this, pname);
         }
         return *Params[pname];
	}
    string getName(){return name;}
    void show() { cout << name << " type :" << atype << " int :" <<valueint << " dbl :"<< valuedouble<<endl; }
};
void AssetParam::show() { cout << parent->getName()<< "["<< name << "] type :" << atype << " int :" <<valueint << " dbl :"<< valuedouble<<endl; }

int main()
{
	AssetVar avint("intAv");
	AssetVar avdbl("dblAv");
	avint = 22;
    avint["MaxVal"] = 25;

	avdbl = 22.3;
	avint.show();
	avint["MaxVal"].show();
	avdbl.show();

}
