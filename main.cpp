#include <iostream>
#include <vector>

#include "TacoLite.h"

// This is not part of the library, it is just to prove types in db fields
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

        Connection connection = Connection::Memory();

        Execute(connection, "create table Things (Content real)");

        Statement statement(connection, "insert into Things values (?)");

        Execute(connection, "begin");

        // inserting a million rows
        for (int i = 0; i < 1000000; ++i) {
            statement.Reset(i);
            statement.Execute();
        }
        Execute(connection, "commit");

        // size in backup: free all pages in backup test
        Execute(connection, "delete from Things where Content > 20");
        // vacumm the database before saving to disk
        // this will delete all free pages in the database before saving
        Execute(connection, "vacuum");

        // creating a backup if a in memory database
        SaveToDisk(connection,"/home/cheetos/Developer/CProgramming/TacoLite/backup.db");

        Statement count(connection, "select count(*) from Things");
        // executing the statement
        count.Step();
        std::cout << "Rows: " << count.GetInt() << std::endl;

    } catch(Exception const & e)
    {
        // c_str: Returns a pointer to an array that contains a
        // null-terminated sequence of characters (i.e., a C-string) representing the current value of the string object.
        std::cout << "Error: " << e.Message.c_str() << " => " << e.Result;
    }
    std::cout << "\nGood bye! [CONNECTION CLOSED]" << std::endl;
    return 0;
}