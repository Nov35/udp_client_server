#include <iostream>
#include "connection.h"

int main(int argc, const char** argv) 
{
    std::cout << "Client: " << Connection::test() << '\n';
    return 0;
}