#include <iostream>
//g++ -std=c++11 -o s Singleton.cpp

class System
{
private:
    System() { value=23;}

public:
    static System& getInstance(){
        static System theInstance;
        return theInstance;
    }

    void prn(){
        std::cout<<"this works! value =" << value << " addr : " << &value <<"\n";
    }
    int value;
};

int main()
{
    System* sysp = nullptr;

    System& sys = System::getInstance();
    sys.prn();
    sys.value = 25;
    sys.prn();    

    sysp = &sys;

    System& sys2 = System::getInstance();
    sys2.prn();    
    sysp->value = 45;

    sys2.prn();
}

// see also
// #include <iostream>

// using namespace std;

// class Singleton {
//    static Singleton *instance;
//    int data;
 
//    // Private constructor so that no objects can be created.
//    Singleton() {
//       data = 0;
//    }

//    public:
//    static Singleton *getInstance() {
//       if (!instance)
//       instance = new Singleton;
//       return instance;
//    }

//    int getData() {
//       return this -> data;
//    }

//    void setData(int data) {
//       this -> data = data;
//    }
// };

// //Initialize pointer to zero so that it can be initialized in first call to getInstance
// Singleton *Singleton::instance = 0;

// int main(){
//    Singleton *s = s->getInstance();
//    cout << s->getData() << endl;
//    s->setData(100);
//    cout << s->getData() << endl;
//    return 0;
// }