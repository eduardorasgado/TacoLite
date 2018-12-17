#include <iostream>
#include <vector>

#include "TacoLite.h"

// to be able ti string without not converting types to string
// Type in the TacoLite library
static char const * TypeName(Type const type)
{
    //printing over the own type
    switch(type)
    {
        // to prove dynamic type insertion in statements
        case Type::Integer: return "Integer";
        case Type::Blob: return "Blob";
        case Type::Float: return "Float";
        case Type::Null: return "Null";
        case Type::Text: return "Text";
    }
    // in case it fails
    assert(false);
    return "Invalid";
}

int main() {
    // Exaple using TACO LITE.

    std::cout << "Welcome to SQLite integration!" << std::endl;

    try {
        /*
         * YOUR CODE HERE
         */

    } catch(Exception const & e)
    {
        // c_str: Returns a pointer to an array that contains a
        // null-terminated sequence of characters (i.e., a C-string) representing the current value of the string object.
        std::cout << "Error: " << e.Message.c_str() << " => " << e.Result;
    }
    std::cout << "\nGood bye! [CONNECTION CLOSED]" << std::endl;
    return 0;
}