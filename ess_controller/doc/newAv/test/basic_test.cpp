NewUtils* vmp;

int AssetVar::av_id = 0;

int test_main()
{
    std::vector<std::string> svec;
    int depth = 0;
    AssetVar avess("ess");
    avess["status"] = 123;  // todo put this into a value param

    avess["status"]["ess_1"]["unit"] = 1.5;
    // return 0;
    ////avess["status"]["ess_1"]["voltage"] = 1300;
    ////avess["config"]["ess_1"]["max_voltage"] = 1500;
    cout << endl << endl;

    cout << " all ess after [] operations >>> ess" << endl;
    // print_Avvalue(&avess, depth);
    cout << endl << endl;

    avess["status"]["ess_1"]["unit"]["name"] = "This is my name";
    cout << " all ess after [] adding name >>> ess" << endl;
    // print_Avvalue(&avess, depth);
    cout << endl << endl;
    // close but no cigar
    // avess["array"][0] = 21;
    cout << " all ess after [] adding array >>> ess" << endl;
    // print_Avvalue(&avess, depth);
    cout << endl << endl;
    // return 0;
    // cout << "show ess/status/ess" <<endl;

    ////print_Avvalue(&avess["status"]["ess_1"], depth);
    // cout << endl << endl;

    const char* sp2 =
        "{\"foo\":{\"value2\":22},\"foo3\":{\"value3\":33}"
        ",\"foo4\":{\"myval2\":{\"extra\":345}}}";
    // const char* sp2 = "{\"foo\":22}";
    // ,\"foo3\":{\"value3\":33}"
    //         ",\"foo4\":{\"myval2\":{\"extra\":345}}}";

    cout << " test string [" << sp2 << "]" << endl << endl;
    cout << " trying << operator" << endl << endl;

    avess << sp2;  // this is broken
    cout << " Test done " << endl;
    // return 0;
    cout << " Value from print_Avvalue   ##########" << endl;
    // print_Avvalue(&avess, depth);
    cout << " Done Value from print_Avvalue   ##########" << endl;

    // return 0;
    ostringstream sfoo, sout;
    // sfoo << endl<<" hi sfoo"<<
    sfoo << avess["status"];
    cout << sfoo.str() << endl;
    cout << " Value from [status] to sout   ##########" << endl;
    // needs comma's
    stream_Avvalue(sout, &avess["status"], depth);
    cout << sout.str() << endl;

    // return 0;
    // these are not in the delete list.
    AssetVar avint("intAv");
    AssetVar avdbl("dblAv");
    AssetVar avtest("testAv");
    // AssetVar avkid("Avkid", &avtest );

    avint = 22;
    // adds to vectors (old Params)
    avint["MaxVal"] = 25;
    avint["MinVal"] = 15;

    avdbl = 22.3;
    avint.show();
    avint["MaxVal"].show();
    avdbl.show();
    // avtest["one"]["two"]["three"] =  3;   //deep param map
    // avtest["one"]["two"]["three"].show();
    // avtest.find("/one/two")  will split string and cycle through vecs
    // avtest["one"]["two"].showKids();
    // avtest.showKids();
    split(svec, "/one/two/three", '/');
    for (auto s : svec)
    {
        cout << "[" << s << "]" << endl;
    }
    svec.clear();
    const char* sp =
        "{\"foo\":{\"fvalue\":21,\"myval\":100,\"actions\":{"
        "\"onSet\":[{\"enum\":["
        "{\"eone\":1, \"two_1\":2, \"three_1\":\"sthree\"},"
        "{\"etwo\":1, \"two_2\":2, \"three_2\":\"sthree\"}"
        "]}]},\"value\":234}}";

    // as you can see, you no longer have to type "\" for escape characters
    // anymore, it is also easier to read. format is as follows: R"(everything
    // inside these paranthesis is your raw string)";
    const char* raw = R"(
        {
            "this": "is", 
            "a": "json", 
            "raw strings": "are cool"
        }
    )";
    cout << raw << endl;

    //"]}]}}}";
    double tNow = get_time_dbl();

    AssetVar avmap("avMap");
    avmap.setType("Vmap");
    cJSON* cj = cJSON_Parse(sp);
    // decodeCJ(&avmap, cj);
    double tCJ = get_time_dbl() - tNow;

    avmap.show();

    tNow = get_time_dbl();
    AssetVar avmap2("avMap2");
    avmap2.setType("Vmap");

    // decodeStr(&avmap2,sp);
    double tSTR = get_time_dbl() - tNow;
    avmap2.show();
    // cout << " looking for foo "<< avmap2.getParam(0)->gotParam("foo") << endl;
    // cout << " looking for foo :actions "<<
    // avmap2.getParam(0)->gotParam("foo")->gotParam("actions") << endl;

    cout << " Parse Results CJ :[" << tCJ * 1000000.0 << "] STR :[" << tSTR * 1000000.0 << "]" << endl;

    char* tmp = cJSON_Print(cj);
    cout << tmp << endl;
    cout << " our own CJ Tree " << endl;
    // printCJTree(cj);
    // int depth = 0;

    print_CJvalue(cj, depth);
    cout << " our own CJ Parser " << endl;
    tNow = get_time_dbl();
    cJSON* cj2 = cJSON_CJParse(sp);
    cout << " our own CJ Parser Output" << endl;
    depth = 0;
    print_CJvalue(cj2, depth);
    tCJ = get_time_dbl() - tNow;

    cout << "Now do our own AVParser " << endl << endl;

    cout << endl;
    cout << " our own Av Parser original sp" << endl;
    tNow = get_time_dbl();
    AssetVar* vmap = cJSON_AVParse(sp);
    cout << " ###########################our own Av Parser after original sp" << endl;

    // print_Avvalue(vmap, depth);
    tSTR = get_time_dbl() - tNow;
    cout << " Parse Results CJ :[" << tCJ * 1000000.0 << "] Av :[" << tSTR * 1000000.0 << "]" << endl;
    cout << " now put in the extra stuff " << endl;
    cJSON_AVParse2(vmap, sp2);
    depth = 0;
    cout << " our own Av Parser extended " << endl;
    // print_Avvalue(vmap, depth);

    setValue(vmap, "foo", "value4", " this is value 4");
    setValue(vmap, "foo", "double4", 123.4);
    setValue(vmap, "foo", "bool5f", false);
    setValue(vmap, "foo", "bool5t", true);
    setValue(vmap, "foo", "value", 1023.4);
    cout << " after setValue" << endl;
    // print_Avvalue(vmap, depth);
    cout << "AVtest" << endl;
    depth = 0;
    setValue(&avtest, "test", "value4", " this is value 4");
    setValue(&avtest, "test", "valueint", 21);
    // print_Avvalue(&avtest, depth);

    avtest["one"]["two"]["three"].show();
    cout << endl;
    cout << "AVarMap 1 at End " << endl;
    AssetVar::showMap();

    cout << "AVarMap 2 at End " << endl;
    for (auto x : AVarMap)
    {
        cout << x.first;
        if (x.second)
        {
            cout << " id : " << x.first->id;
            if (x.first->cstring)
            {
                cout << " cstring [" << x.first->cstring << "]";
            }
            else
            {
                cout << " name [" << x.first->name << "]";
            }
        }
        else
        {
            cout << " ... deleted";
        }
        cout << endl;
    }
    AssetVar* avFoo = vmap->gotParam("foo");
    cout << " AvFoo == " << avFoo << endl;
    for (auto x : avFoo->aList)
    {
        if (x.second)
        {
            cout << " ID :" << x.first->id << " myName :" << x.second << endl;
        }
        else
        {
            cout << " ID :" << x.first->id << " NULL :" << endl;
        }
    }
    cout << "########################" << endl;
    ostringstream sfoo2, sfoo3;
    AssetVar vm2("lroot");
    vm2["lbase"]["value21"] = 21;
    vm2["lbase"]["value22"] = "new value 22";
    vm2["lbase"]["value23"] = "new value 23";
    // vm2["lbase"]["value23"].fixupAlist();
    sfoo2 << endl << " hi sfoo2" << endl;
    depth = 0;
    stream_Avvalue(sfoo2, &vm2, depth);
    // sfoo2<< vm2;
    cout << sfoo2.str() << endl;
    cout << "###########create link a[xx]=b[yy] #############" << endl;

    // this is one way to create a link
    vm2["lbase"]["value21"] = vm2["lbase"]["value23"];
    depth = 0;
    stream_Avvalue(sfoo3, &vm2, depth);
    cout << sfoo3.str() << endl;
    cout << endl;

    cout << "###########vm.make_link(\"/a/b/c:d\",\"/x/y:z\"); #############" << endl;
    // this another way to create a link
    vm2.makeLink("/control/pcs/powerDemand", "/components/pcs/fast/statusPower");
    depth = 0;

    // this is how we clear an ostringstream
    sfoo3.str(string());
    sfoo3.clear();
    // ostringstream sfoo4;
    stream_Avvalue(sfoo3, &vm2, depth);
    cout << sfoo3.str() << endl;
    cout << endl;

    cout << "###########check link(\"/a/b/c:d\",\"/x/y:z\"); #############" << endl;
    // vm2["components"]["pcs"]["fast"]["statusPower"] = "a lot of power";
    vm2.setVal("/components/pcs/fast/statusPower/value", "a lot of power here");

    sfoo3.str(string());
    sfoo3.clear();
    // ostringstream sfoo4;
    stream_Avvalue(sfoo3, &vm2, depth);
    cout << sfoo3.str() << endl;
    cout << endl;
    // OK add an action for statusPower
    vm2.addAction("/components/pcs/fast/statusPower", "onSet", "enum",
                  "{\"iValue\": 21 , \"outValue\":\"The value is 21\"}");
    vm2.addAction("/components/pcs/fast/statusPower", "onSet", "enum",
                  "{\"iValue\": 22 , \"outValue\":\"The value is 22\"}");
    vm2.addAction("/components/pcs/fast/statusPower", "onSet", "remap", "{\"uri\":\"/components/bcs/outThere\"}");
    vm2.setVal("/components/pcs/fast/statusPower/value", "less is more");

    vm2.setVal("/components/pcs/test/TestValues:val1@myParam45", 45);

    sfoo3.str(string());
    sfoo3.clear();
    // ostringstream sfoo4;
    stream_Avvalue(sfoo3, &vm2, depth);
    cout << sfoo3.str() << endl;
    cout << endl;
    // depth = 0;
    // print_CJvalue(vm2, depth);
    cout << "########################" << endl;
    return 0;
}
