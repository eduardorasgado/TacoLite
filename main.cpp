#include <iostream>
#include <vector>

#include "TacoLite.h"

int main() {
    // Exaple using TACO LITE.

    std::cout << "Welcome to SQLite integration!" << std::endl;

    try {
        // creating a database entirely in memory
        Connection connection = Connection::Memory();

        // Creating connection and statement handler and doing the query at the same time
        // preparing the query inside the Statement constructor by calling no member function
        // Execute in sqlite.h
        Execute(connection, "create table Users (Name)");
        Execute(connection, "insert into Users values (?)", "Eduardo");
        Execute(connection, "insert into Users values (?)", "Ana Belen");
        Execute(connection, "insert into Users values (?)", "Juanisimo");

        // to take all users getting by the iteration over the query
        auto Users = std::vector<std::string>();

        // getting the elements returned by the query
        for(Row row : Statement(connection, "select Name from Users"))
        {
            // GetString has a default parameter 0, but you can write it instead
            Users.push_back(row.GetString());
        }

        // showing all the users inside the vector
        for(auto & user : Users)
        {
            std::cout << "User: " << user << "\n";
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