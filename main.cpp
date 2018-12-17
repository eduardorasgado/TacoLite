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
        // creating a database entirely in memory
        Connection connection = Connection::Memory();

        // Creating connection and statement handler and doing the query at the same time
        // preparing the query inside the Statement constructor by calling no member function
        // Execute in sqlite.h
        Execute(connection, "create table Things (Content)");
        // saving data as floats
        //Execute(connection, "create table Things (Content real)");
        // saving data as text
        //Execute(connection, "create table Things (Content  text)");

        Execute(connection, "insert into Things values (?)", "Eduardo");
        Execute(connection, "insert into Things values (?)", 255);
        auto mass = 75.3325;
        auto atoms = 7500023342.423325;
        Execute(connection, "insert into Things values (?)", mass);
        Execute(connection, "insert into Things values (?)", atoms);

        // getting the elements returned by the query
        for(Row row : Statement(connection, "select Content from Things"))
        {
            // proving we are getting the sqlite3 dynamic type selection
            std::cout << TypeName(row.GetType()) << " : " << row.GetString() << "\n";
        }

    } catch(Exception const & e)
    {
        // c_str: Returns a pointer to an array that contains a
        // null-terminated sequence of characters (i.e., a C-string) representing the current value of the string object.
        std::cout << "Error: " << e.Message.c_str() << " => " << e.Result;
    }
    std::cout << "\nGood bye! [CONNECTION CLOSED]" << std::endl;
    return 0;
}