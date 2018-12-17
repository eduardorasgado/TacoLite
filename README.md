# TacoLite: A simple SQLite and Modern C++ Integration library

This project is a handler for C++ SQLite3 API. It is intended to create a simple way to interact with database.
All the commits behind its implementation are here:

https://github.com/eduardorasgado/Cpp-AdvancedTopics/tree/master/SQLiteInteraction

## Compile the project
Linux based compilation:

To compile the project install [Conan C++ Package manager](http://conan.io) and install sqlite3 using conan in your system, you can find it in conan center,
then just run:
```console
foo@bar :~$ mkdir build && cd build
foo@bar :~$ conan install ..
foo@bar :~$ cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
foo@bar :~$ cmake --build .
``` 

## How to use
Handle.h and TacoLite.h are the celebrities here.\
Once TacoLite is installed in your project by using conan, you can interact with sqlite in a easy way.

### Include in your project

```C++
#include "TacoLite.h"
```

Now, be careful and try to using try and catch when using TacoLite.
### In a nutshell:

```C++
Connection connection = Connection::Memory();

// Creating connection and statement handler and doing the query at the same time
// preparing the query inside the Statement constructor by calling no member function
// Execute in TacoLite.h
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
```

### Creating a connection

```C++
try {
    // creating a database entirely in memory
    Connection connection = Connection::Memory();
    //or if you're using it to store wide characters 
    Connection connection = Connection::WideMemory();
}
```

### Creating databases and inserting data

```C++
/// instead of in memory
// creating 2 physical databases, empties
Connection utf8("/home/cheetos/Developer/CProgramming/Cpp-AdvancedTopics/SQLiteInteraction/utf8database.db");
Connection utf16("/home/cheetos/Developer/CProgramming/Cpp-AdvancedTopics/SQLiteInteraction/utf16database.db");
```

### Executing a query and binding data
```C++
// statement handler
Statement statement;

// in case not a utf8 encoding, statements first convert any encoding type
// into utf8
// preparing the query
statement.Prepare(connection, "select ?1 union all select "
                              "?2 union all select ?3 union all select ?4 union all select ?5"
                              "union all select ?6");

// BINDING regular encodings
std::vector<std::string> data{"Hello", " ", "SQLite3 and C++"};

// if bindingAll is called and Bind too then BindAll should be called first
// automatic binding
statement.BindAll("hello", std::string("Your"), std::string("Name"));

// common bindings
statement.Bind(4, data[0]);
// binding a r-value is possible
statement.Bind(5, std::string("everybody over"));
statement.Bind(6, data[2]);
```

### Inline automatic binding

```C++
Connection connection = Connection::Memory();

Statement statement;

statement.Prepare(connection, "select ?1 union all select "
                                      "?2 union all select ?3 union all select ?4 union all select ?5"
                                      "union all select ?6", "Eduardo", 24, 16190278, "single", 70, "programmer");
                                      
//or:

// Creating statement handler and doing the query at the same time
// preparing the query inside the Statement constructor
Statement statement(connection, "select ?1 union all select "
                              "?2 union all select ?3 union all select ?4 union all select ?5"
                              "union all select ?6", "Eduardo", 24, 16190278, "single", 70, "programmer");
```

### Read the rows in your statement

```C++
// calling the Row iterator classes
for(Row const & row : statement)
{
    // simple loop for reading rows
    // using the reader
    std::cout << row.GetString(0) << "\n";
}
```

### Profiling to check performance

```C++
Connection connection = Connection::Memory();

// creating a profile and passing it a callback
connection.Profile([](void *, char const * const statement,
                      unsigned long long const time){
    // 6 zeros: nano seconds in milliseconds
    unsigned long long const ms = time / 1000000;

    if(ms > 10)
    {
        std::cout << "Profiler: " << ms << statement << "\n";
    }
});

```

### Use case 1:

```C++
// creating a database entirely in memory
Connection connection = Connection::Memory();

Execute(connection, "create table Users (Id primary key, Name)");

Execute(connection, "insert into Users (Id, Name) values (?, ?)", 1, "Eduardo");
Execute(connection, "insert into Users (Id, Name) values (?, ?)", 2, "Ana Belen");
Execute(connection, "insert into Users (Id, Name) values (?, ?)", 3, "Juanisimo");
Execute(connection, "insert into Users (Id, Name) values (?, ?)", 4, "Angelo");

for (Row row : Statement(connection, "select Id, Name from Users"))
{
    std::cout << row.GetInt(0) << ", " << row.GetString(1) << "\n";
}
```

### Use case 2:

```C++
// traditional database file
Connection connection("/home/cheetos/Developer/CProgramming/TacoLite/test.db");
// creating table
// taking RowId autogenerated by sqlite as Id
Execute(connection, "create table Users (Name text, Age integer, Weight real)");

Statement insert(connection, "insert into Users (Name, Age, Weight) values (?1, ?2, ?3)");

// inserting data
// avoid flushing in memory cause slow transactions
Execute(connection, "begin");

// transctions can be accomplished by reseting the insert object
for(int i = 0;  i < 1000; ++i)
{
    // reset the sqlite state machine and BindAll at the same time
    insert.Reset("Eduardo", i, 70.43);

    // Execute method calls the step -> initialize the transaction
    insert.Execute();
    std::cout << "Inserted item: " << connection.RowId() << "\n";
}

// end transaction, all changes after this will be discarted
Execute(connection, "commit");

// counting the nummber of rows
Statement count(connection, "select count(*) from Users");
//executing the statement
count.Step();
std::cout << "Rows: " << count.GetInt() << std::endl;
```