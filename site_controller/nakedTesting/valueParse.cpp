#include <iostream>
#include <fstream>
#include <string>

bool expected(char* argv[]){
    std::ifstream data;
    data.open(argv[1], std::ios::in);
    if(!data){
        std::cout<<"File failed to open\n";
        return false;
    }

    std::string value;
    std::string dataAsString;
    dataAsString.assign((std::istreambuf_iterator<char>(data)), (std::istreambuf_iterator<char>()));
    size_t found = dataAsString.find("value");
    //std::cout<<dataAsString<<'\n';
    if(found == std::string::npos){
        if(dataAsString[0] == '"'){
            found = 1;
            while(dataAsString[found] != '"'){
                value.push_back(dataAsString[found]);
                found++;
            }
        }
        else{
            std::cout<<value<<'\n';
            std::cout<<"failure :(";
        }
    }
    else{
        while(dataAsString[found]!='"'){
            found++;
        }
        found+=2;
        //std::cerr<<"current location = " << dataAsString[found]<<"\n";
        if(dataAsString[found] == '"'){
            found++;
            while(dataAsString[found]!='"'){
                value.push_back(dataAsString[found]);
                found++;
            }
        }else{
            while(dataAsString[found] != '.' && dataAsString[found] != ','){
                value.push_back(dataAsString[found]);
                found++;
            }
            int decimal = 3;
            while(decimal!=0 && dataAsString[found] != ','){
                value.push_back(dataAsString[found]);
                found++;
                decimal--;
            }
            //std::cerr<<value<<'\n';
        }
    }
    
    //std::cout<<value<<"\n";
    if(value != argv[2]){
        std::cout<<value<<'\n';
        std::cout<<"Unexpected value at "<< argv[2] << "\n";
    }else{
        std::cout<<"success\n";
    }
    data.close();
}

int main(int argc, char* argv[]){
    expected(argv);
    return 0;
}