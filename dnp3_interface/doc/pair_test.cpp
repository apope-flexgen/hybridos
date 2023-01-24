#include <iostream>
#include <utility>
#include <functional>
#include <vector>

int main()
{
    //std::vector<std::pair<DbVar*,int>>dbs; 
    std::vector<std::pair<int,int*>>dbs; 
    int n = 1;
    int a[5] = {1, 2, 3, 4, 5};
 
    // build a pair from two ints
    std::pair<int,int*> p1 = std::make_pair(n, &a[1]);
    std::cout << "The value of p1 is "
              << "(" << p1.first << ", " << *p1.second << ")\n";
 
    // build a pair from a reference to int and an array (decayed to pointer)
    std::pair<int ,int*> p2 = std::make_pair(n+1, &a[0]);
    n = 7;
    std::cout << "The value of p2 is "
              << "(" << p2.first << ", " << *(p2.second + 2) << ")\n";
    dbs.push_back(std::make_pair(n++, &a[0]));
    dbs.push_back(std::make_pair(n++, &a[1]));
    dbs.push_back(std::make_pair(n++, &a[1]));
    dbs.push_back(std::make_pair(n++, &a[1]));
    dbs.push_back(std::make_pair(n++, &a[1]));
    dbs.push_back(std::make_pair(n++, &a[1]));
    dbs.push_back(std::make_pair(n++, &a[1]));
    dbs.push_back(std::make_pair(n++, &a[1]));
    while (!dbs.empty())
    { 
        std::pair<int, int*>dbp = dbs.back();
        std::cout << "popping dbp " << dbp.first <<"\n";
        //delete dbp;
        dbs.pop_back();
    }

//std::pair<DbVar*,int>dbp = dbs.back();
}