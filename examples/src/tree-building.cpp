#include <iostream>
#include <vs-xml/builder.hpp>
using namespace xml;

int main(){
    Builder bld;
    //....

    auto tree = bld.close();
    tree->print(std::cout);
    return 0;
}