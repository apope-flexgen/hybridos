#ifndef __NEW_AV2_H
#define __NEW_AV2_H

#include<iostream>
#include<sstream>
#include<string>
#include<map>
#include<vector>
#include <cjson/cJSON.h>
#include <cstring>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#include "newUtils.h"

using namespace std;

#define DBL_EPSILON 2.2204460492503131e-16

// setVal will trigger actions
// actAv["shift"] = 2;
// actAv["inValue"] = 0;
// actAv["outValue"] = "This is an out value";
// actAv["uri"] = "/a/b/c:d@foo";
// make up a vector of actAV's
// actVec.push_back (actAv)

// av->addAction("onSet", "enum", actVec)
// we have an actions dict(av)
// add a name into it
//
// we have a function "enum" .. then  that to the vectors
// as an av
// add the actVec to the enum var 

// after this we will have a multi dimensional vmap with params and actions
// all over the place.  


// Dont have time today to work out the chrono stuff.

//double base_time = 0.0;
long int get_time_us();
double get_time_dbl();

class AssetVar;

// namespace
// {
// //	map<AssetVar *,AssetVar*> AVarMap;
// 	int av_id = 21;
// }
//int av_id = 44;

static map<AssetVar *,AssetVar*> AVarMap;
int split(vector<string> &output, const string& s, char seperator);


// singleton class
// AvarMap::get().Map;
// AvarMap::get().av_id;
class AvarMap 
{
public:
		static AvarMap& get(){
		static AvarMap instance;
		return instance;
	}
	map<AssetVar *,AssetVar*> Map;

private:
	AvarMap(){
	   cout << "getInstance(): First instance\n";
	};
};

class AssetVar;

typedef vector<pair<AssetVar*,char*>> alist;

class AssetVar {
private:
public:

	static int av_id;  // used to clean up


	//int valueint;  // TODO create union 
    char* valuestring;
    bool valuebool;
    double valuedouble;

	void* func;          // HMM
  
    char* cstring;
	char *comp;    // /root/who/am/i
    string name; // loose this
    string atype; // loose this
  
    int type;  // nominally the cjson type
	AssetVar* parent;
	AssetVar* root;	       // we may have to use this 
    //this is a map of AV's
	map<string,AssetVar *> Params;

	// this replaces all the prev / next stuff in the AV's
	// all these are children
	vector<pair<AssetVar*,char*>>aList;

	int id;	
	int alidx;  // index into parent's alist
	
public:
    friend ostringstream &operator << (ostringstream &out, const AssetVar &av);

	AssetVar(string _name, AssetVar *p = nullptr, AssetVar *c = nullptr)
	{
		AvInit();
		name = _name; 
		parent = p;
		cstring = strdup(name.c_str());
		//addMapping(cstring, p);		
	}

	AssetVar(const char* who, AssetVar *p)
	{
		AvInit();
		parent=p; 
		addMapping(who, p);		
	}	


    AssetVar()
	{ 
		AvInit();
		addMapping(" Default", NULL);
	};

	void AvInit()
	{
		id = av_id++;
		root = nullptr;
		parent = nullptr; 

		func = nullptr;

		//valueint = 0;
		valuedouble = 0.0;
		valuebool = 0;
		valuestring = nullptr;
		atype = "none";
		type = cJSON_Object;

		cstring= nullptr;
		name="";
		comp = nullptr;
	
	}
	void addMapping(const char*who, AssetVar *p)
	{
		if (0)//p)
		{
			alidx = p->aList.size();

			//cout << " parent -> id: "<<p->id<<"  alistidx :"<<alidx ;
			// if(p->cstring)
			// 	cout << " name ["<<p->cstring<<"]";
		    p->aList.push_back(make_pair(this, strdup(cstring)));
		}
		//AVarMap[this] = this;
	}

    ~AssetVar()
	{
		if (valuestring) free(valuestring);
	}

	static void showMap()
	{
		cout << "AVarMap at End " << endl;
		for (auto x : AVarMap)
		{
			cout << x.first;
			if(x.second)
			{
				cout << " id : " << x.first->id;
				if(x.first->cstring)
				{
					cout << " cstring [" << x.first->cstring <<"]";
				}
				else
				{
					cout << " name [" << x.first->name <<"]";
				}
			}
			else
			{
				cout << " ... deleted";
			}
			cout << endl;
		}
	}
	// This is automatically called when '+' is used with
	// between two Complex objects
	// TODO need to make sure we dont have a "value" param.
	void operator = (int  val) {
		cout << __func__ << " cstring :["<<cstring<<"] type :"<<type<< endl;
		AssetVar *av = NULL;
		if (type == cJSON_Number)
		{
	 		valuedouble = val;
		}
		else
		{
			if(gotParam("value"))
			{
				av = getParam("value");
				*av = val;
			}
			else
			{			
				av = addCJParam("value", cJSON_Number);
				*av = val;
			}
			//if 
		}
	 	// //valueint = val;
        // valuedouble = val;
        // atype = "int";
		// type = cJSON_Number;
	}

	void operator = (double  val) {
		cout << __func__ << " cstring :["<<cstring<<"] type :"<<type<< endl;
		AssetVar *av = NULL;
		if (type == cJSON_Number)
		{
	 		valuedouble = val;
		}
		else
		{
			if(gotParam("value"))
			{
				av = getParam("value");
				*av = val;
			}
			else
			{			
				av = addCJParam("value", cJSON_Number);
				*av = val;
			}
			//if 
		}
	 	// valuedouble = val;
        // //valueint = val;
        // atype = "double";
		// type = cJSON_Number;
	}
	
	void operator = (const char* val) {
		if(valuestring)
			free(valuestring);
	 	valuestring = strdup(val);
        atype = "string";
		type = cJSON_String;
	}

    // this is the link magic
	// same name different av
	void fixupAlist()
	{
		if(parent)
		{
			int idx = 0;
			for(auto ax : parent->aList)
			{
				cout << "before <<< " << ax.first << " ->" << ax.second << endl;
				if(ax.second)
				{
					free(ax.second);
					ax.second = NULL;
				}
				if (ax.first->cstring)
				{
					ax.second = strdup(ax.first->cstring);
				}
				cout << "        after >>>"<<ax.first << " ->" << ax.second << endl;
				parent->aList.at(idx) = ax;
				idx++;

			}
			for(auto ax : parent->aList)
			{
				cout << "check @@@ " << ax.first << " ->" << ax.second << endl;
			}
		}

	}

	AssetVar &operator = (AssetVar &avlink) {
		cout << __func__ ;
		cout << "replacing [ "<< this->cstring<<"] with ["<< avlink.cstring<<"]"<<endl;
		cout << "replacing idx [ "<< this->alidx<<"] with ["<< avlink.alidx<<"]"<<endl;
		
		// we live in the alidx entry of our parent's aList 
		// we keep the same name but avlink is here to replace us
		if(parent)
		{
			for(auto ax : parent->aList)
			{
				cout << ax.first << " ->" << ax.second << endl;
			}
			for(auto ay : parent->Params)
			{
				cout << ay.first << " ->" <<ay.second<< " -->"<< ay.second->cstring << endl;
			}
			auto al = parent->aList.at(this->alidx);
			cout << "replacing ["<< al.second<<"] with ["<< avlink.cstring<<"]"<<endl;
			al.first = &avlink;
			parent->Params[al.second] = &avlink;
			parent->aList.at(this->alidx) = al;
		}
		return *this;
	}
// this is a bit clumsy but it we allocate a child to a non object then the value should be passed 
//into the value param
	AssetVar &operator [] (string  pname) {
		AssetVar *av = NULL;
		//cout << " Looking for [" << pname << "] in ["<< this->name<< "]"<<endl;
		//cout << " this->name ["<< this->name << "] "<<this<<"  Params size :"<< Params.size()<<endl; 
		// cout << " Current Params for :" << cstring << endl;
		// for ( auto x : Params)
		// {
		// 	cout << x.first << endl; 
		// }

        // set value, move it from local values to a value Param
		if(Params.find(pname) == Params.end())
        {
			//cout << " Creating  [" << pname << "] in ["<< this->name<< "]"<<endl;
			if (type != cJSON_Object)
			{
			    //cout << " Creating  [" << "value" << "] in ["<< this->name<< "]"<<endl;
				AssetVar *av;
				av = addCJParam("value", type);
				
				// we have a parent but we are not its child
    	        //  Params[pname]= new AssetVar(pname, this);
				av->valuebool = valuebool;
				//av->valueint = valueint;
				av->valuedouble = valuedouble;
				av->valuestring = valuestring;
				av->parent = this;
				valuestring = NULL;

			}
			av = addCJParam(pname.c_str(), cJSON_Object);
			 // we have a parent but we are not its child
            //  Params[pname]= new AssetVar(pname, this);
			// aList.push_back
			if (av->parent)
			{
				//cout << __func__<< " parent type: "<<av->parent->type << endl;
				av->parent->type = cJSON_Object;
			}
        }
		// make sure addCJParam increments the parent's aList
        return *Params[pname];
	}

	// 1/parent is an String, Number or an Object
	// turns parent into an array and creates ann array entry Object which it returns.
	// 2/ parent is an array , finds / adds item 
	//  returns entry.
	AssetVar &operator [] (int idx) {
		
		//cout << __func__ << " type :"<< type<<endl;
		if( type != cJSON_Array)
		{
			type = cJSON_Array;
			cout << __func__ << " turning type of ["<< cstring <<"] into an Array :"<< type<<endl;
			
		}
		// if ( (int)aList.size() < (idx+1))
		// {
		// 	cout << __func__ << " old size : "<< aList.size() << " idx : " << idx << " extending"<<endl;
		// }
		int i  = (int)aList.size();
		while (i++ <= idx)
		{
			char *sp = NULL;
			aList.push_back(make_pair(new AssetVar, sp));
		}

		//cout << __func__ << " old size : "<< aList.size() << " idx : " << idx << " extended"<<endl;

		return (*aList.at(idx).first);
	}

	AssetVar &operator << (const char* sp); 

	AssetVar*getChild(int num)
	{
		if(num <= (int)aList.size())
		{
			return aList[num].first;
		}
		return NULL;
	}
	int getListSize(){ return (int)aList.size();}
	int numParams(){ return (int)Params.size();}

	AssetVar* setParam(const char* pname, double dval)
	{
		AssetVar * av = addCJParam(pname,cJSON_Number);
    	av->atype = "Double";
    	av->valuedouble = dval;
    	//av->valueint = (int)dval;
        return av;       
	}

	AssetVar* setParam(const char*  pname, bool dval)
	{
		AssetVar *av;
		if (dval)
			av = addCJParam(pname,cJSON_True);
		else
			av = addCJParam(pname,cJSON_False);
        av->atype = "Bool";
        av->valuebool = dval;
        return av;
        
	}

	AssetVar* setParam(const char* pname, const char* sval)
	{
		AssetVar * av =addCJParam(pname,cJSON_String);
		// we have a parent but we are not its child
        //Params[pname]= new AssetVar(pname, this);
        av->atype = "String";
        if(av->valuestring != NULL)
            free (av->valuestring);
        av->valuestring = strdup(sval);
        return av;        
	}

	AssetVar* gotParam(string pname)
	{
		if(Params.find(pname) == Params.end())
        {
			return NULL;
        }
        return Params[pname];
	}

	AssetVar* getParam(string pname)
	{
		AssetVar *av = gotParam(pname.c_str());
		if (!av)
		{
			av = addCJParam(pname.c_str(),cJSON_Number);
			 // we have a parent but we are not its child
			av->atype = "Double";
        	av->valuedouble = 0;
        	//av->valueint = 0;
        	av->valuebool = false;
		}
        return av;
	}

	AssetVar* getParent()
	{
		return parent;
	}

	void makeLink(const char* var1, const char* var2);
	AssetVar* makeLink(const char* var1, const char* name, AssetVar *av);
	
	AssetVar* getAv(const char* var, int options = 0);
	AssetVar* getAv2(const char* var);

	double getdParam(const char* pname)
	{
		if (Params.find(pname)!= Params.end()) return Params[pname]->valuedouble;
		return 0.0;
	}

	const char* getcParam(const char* pname)
	{
		if (Params.find(pname)!= Params.end()) return Params[pname]->valuestring;
		return nullptr;
	}

	AssetVar* incParam(const char* pname, double val)
	{
		AssetVar*av = addCJParam(pname, cJSON_Number);
		//av->valueint+=val;
		av->valuedouble+=val;
		return av;
	}

	
	int getiParam(const char* pname)
	{
		int ival = 0;
		if (Params.find(pname)!= Params.end())
		{ 

			double number = Params[pname]->valuedouble;
			/* use saturation in case of overflow */
    		if (number >= INT_MAX)
			{
				ival = INT_MAX;
			}
			else if (number <= (double)INT_MIN)
			{
				ival = INT_MIN;
			}
			else
			{
				ival = (int)number;
			}
		}
		return ival;
	}

	// shadows type 
	bool getbParam(const char* pname)
	{
		if (Params.find(pname)!= Params.end()) return Params[pname]->valuebool;
		return false;
	}

	AssetVar* incParam(const char* pname, int val)
	{
		AssetVar*av = addCJParam(pname, cJSON_Number);
		//av->valueint+=val;
		av->valuedouble+=val;
		return av;
	}

	AssetVar* setParam(const char* pname, int val)
	{
		AssetVar*av = addCJParam(pname, cJSON_Number);
		//av->valueint=val;
		av->valuedouble=val;
		return av;
	}
	
	AssetVar* addCJParam(const char* name, int cjtype=cJSON_Object);

	AssetVar* setVal(const char* name, const char* val);
	AssetVar* setVal(const char* name, int val);
	AssetVar* setVal(const char* name, double val);
	AssetVar* setVal(const char* name, bool val);
	
	AssetVar* getActions(const char* actname);
	
	AssetVar* addAction(const char* name, const char* when, const char* act, const char* args);
	//"/components/pcs/fast/statusPower"
    //            ,"enum"
    //            , "{\"iValue\": 21 , \"outValue\":\"The value is 21\"}");

	void setType(const char* _atype){atype = _atype;}
    string getName(){return name;}
    string getType(){return atype;}
	char*  getString(){return valuestring;}
	double getDouble(){return valuedouble;}
	bool   getBool(){return valuebool;}
	const char* getBoolStr(){if(valuebool) return "true"; else return "false";}

    void show(int level = 0);// { cout << name << " type :" << atype << " int :" <<valueint << " dbl :"<< valuedouble<<endl; }
    void showKids(int level = 0);// { cout << name << " type :" << atype << " int :" <<valueint << " dbl :"<< valuedouble<<endl; }
};

//AssetVar* decodeStr(AssetVar *av, const char* str);
// decode options
// '{"a":b}'   -- simple av(a) ->value = b what ever b's type is stick it in a value
// '{"a":{"value":c,"p1":d}}' populate params value p1 etc
//                    
// we loose the varsmap instead root everything from a base av.


/* securely comparison of floating-point variables */
bool compare_double(double a, double b);
// // now import the real cJSON Parser
// // first into a cJSON object  
// // then into out AssetVar
// typedef struct {
//     const unsigned char *json;
//     size_t position;
// } error;
// //static error global_error = { NULL, 0 };
// /* get the decimal point character of the current locale */
// //static 
// unsigned char get_decimal_point(void);

// typedef struct
// {
//     const unsigned char *content;
//     size_t length;
//     size_t offset;
//     size_t depth; /* How deeply nested (in arrays/objects) is the input at the current offset. */
//     //internal_hooks hooks;
// } parse_buffer;

// /* check if the given size is left to read in a given parse buffer (starting with 1) */
// #define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
// /* check if the buffer can be accessed at the given index (starting with 0) */
// #define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
// #define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))
// /* get a pointer to the buffer at the position */
// #define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)
// /* parse 4 digit hexadecimal number */
// //static 
// unsigned parse_hex4(const unsigned char * const input);
// /* converts a UTF-16 literal to UTF-8
// / * A literal can be one or two sequences of the form \uXXXX */
// //static 
// unsigned char utf16_literal_to_utf8(const unsigned char * const input_pointer, const unsigned char * const input_end, unsigned char **output_pointer);
// /* Utility to jump whitespace and cr/lf */
// //static 
// parse_buffer *buffer_skip_whitespace(parse_buffer * const buffer);
// /* skip the UTF-8 BOM (byte order mark) if it is at the beginning of a buffer */
// //static 
// parse_buffer *skip_utf8_bom(parse_buffer * const buffer);
////////////////////////////////
// thse are all taken from cJSON
// converted a bit to c++
///////////////////////////////bool parse_CJvalue(cJSON * const item, parse_buffer * const input_buffer);
bool parse_Avarray(AssetVar * const item, parse_buffer * const input_buffer);
bool parse_Avobject(AssetVar * const item, parse_buffer * const input_buffer);
bool parse_Avstring(AssetVar * const item, parse_buffer * const input_buffer);
bool parse_Avnumber(AssetVar * const item, parse_buffer * const input_buffer);
bool parse_Avvalue(AssetVar* const item, parse_buffer* const input_buffer);
AssetVar* cJSON_AvParseWithOpts(const char *value, const char **return_parse_end, bool require_null_terminated);

/* Default options for cJSON_Parse */
AssetVar* cJSON_AvParse(const char *value);

AssetVar* cJSON_AvParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, bool require_null_terminated);

/* Default options for cJSON_Parse */
AssetVar* cJSON_AVParse(const char *value);


// decode options
// '{"a":b}'   -- simple av(a) ->value = b what ever b's type is stick it in a value
// '{"a":{"value":c,"p1":d}}' populate params value p1 etc
//                    
// we loose the varsmap instead root everything from a base av.
AssetVar* findAv(AssetVar *av, const char* comp, const char*name, int type = (int)cJSON_Object);

AssetVar* getActs(AssetVar *av, const char *act);

AssetVar* runAction(AssetVar* av, AssetVar* avn, const char* when);

AssetVar* setValue(AssetVar* av, const char* comp, const char*name, const char* val );
AssetVar* setValue(AssetVar* av, const char* comp, const char*name, double val);
AssetVar* setValue(AssetVar* av, const char* comp, const char*name, bool val);
AssetVar* setValue(AssetVar* av, const char* comp, const char*name, int val);

bool parse_CJvalue(cJSON * const item, parse_buffer * const input_buffer);
bool parse_CJarray(cJSON * const item, parse_buffer * const input_buffer);
//static cJSON_bool print_array(const cJSON * const item, printbuffer * const output_buffer);
bool parse_CJobject(cJSON * const item, parse_buffer * const input_buffer);
bool parse_CJstring(cJSON * const item, parse_buffer * const input_buffer);
bool parse_CJnumber(cJSON * const item, parse_buffer * const input_buffer);

bool print_CJvalue(const cJSON* const item, int &depth);
bool print_CJarray(const cJSON * const item, int &depth);
bool print_CJnumber(const cJSON * const item, int &depth);
bool print_CJstring(const cJSON * const item, int &depth);
bool print_CJobject(const cJSON * const item, int &depth);
cJSON* cJSON_CJParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, bool require_null_terminated);
CJSON_PUBLIC(cJSON *) cJSON_CJParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated);
cJSON *cJSON_CJParse(const char *value);
AssetVar* cJSON_AvParse(const char *value);
AssetVar* cJSON_AVParse2(AssetVar*item, const char *value);
bool stream_Avvalue(ostringstream &out, const AssetVar* const item, int &depth);
bool stream_Avarray(ostringstream &out, const AssetVar* const item, int &depth);
bool stream_Avnumber(ostringstream &out, const AssetVar* const item, int &depth);
bool stream_Avstring(ostringstream &out, const AssetVar* const item, int &depth);
bool stream_Avobject(ostringstream &out, const AssetVar* const item, int &depth);

void* getFunc(AssetVar* av, const char* aname, const char* fname);
AssetVar* setFunc(AssetVar* av, const char* aname, const char* fname, void* func);


#endif
