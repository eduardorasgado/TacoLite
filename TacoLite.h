//
// Created by cheetos on 16/12/18.
//

#pragma once
#include <string>
// imported through conan package manager
#include "sqlite3.h"
// resource handler
#include "Handle.h"


// macros
#ifdef _DEBUG
#define VERIFY ASSERT
#define VERIFY_(result, expression) ASSERT(result == expression)
#else
#define VERIFY(expression) (expression)
#define VERIFY_(result, expression) (expression)
#endif

enum class Type
{
        // type system in cqlite3
        Integer = SQLITE_INTEGER,
        Float = SQLITE_FLOAT,
        Blob = SQLITE_BLOB,
        Null = SQLITE_NULL,
        Text = SQLITE_TEXT,
};

// Turning error into exceptions
struct Exception
{
    int Result = 0;
    std::string Message;
    explicit Exception(sqlite3 * const connection) :
    Result(sqlite3_extended_errcode(connection)),
    Message(sqlite3_errmsg(connection))
    {}
};

// Modeling connections
class Connection
{
    private:
        // to manipulate sqlite connections as handles
        struct ConnectionHandleTraits : HandleTraits<sqlite3 *>
        {
            static void Close(Type value) noexcept
            {
                // call the macro instead
                VERIFY_(SQLITE_OK, sqlite3_close(value));
            }
        };

        // alias for convenience
        using ConnectionHandle = Handle<ConnectionHandleTraits>;

        ConnectionHandle m_handle;

        // function and a character
        template <typename F, typename C>
        void InternalOpen(F open, C const * const filename)
        {
            Connection temp;

            if(SQLITE_OK != open(filename, temp.m_handle.Set()))
            {
                // reporting error
                temp.ThrowLastError();
            }
            // In handle header
            Swap(m_handle, temp.m_handle);
        }
    public:

        Connection() noexcept = default;

        template <typename C>
        explicit Connection(C const * const filename)
        {
            Open(filename);
        }

        static Connection Memory()
        {
            // to create in memory connections
            return Connection(":memory:");
        }

        static Connection WideMemory()
        {
            // L encoding variation
            return Connection(L":memory:");
        }

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(m_handle);
        }

        sqlite3 * GetAbi() const noexcept
        {
            return m_handle.Get();
        }

        // it was a __declspec
        void ThrowLastError() const
        {
            // Application binary interface
            throw Exception(GetAbi());
        }

        // Open overloads
        void Open(char const * const filename)
        {
            InternalOpen(sqlite3_open, filename);
        }

        // wchar_t stands for Wide character
        void Open(wchar_t const * const filename)
        {
            InternalOpen(sqlite3_open16, filename);
        }

        // to be able to get the las RowId inserted
        long RowId() const noexcept
        {
            return sqlite3_last_insert_rowid(GetAbi());
        }


};

// ROW READER FOR STATEMENTS
template <typename T>
struct Reader
{
    // to be able to read rows by columns in the controller or main
    int GetInt(int const column = 0) const noexcept
    {
        // doing a static cast because GetAbi is a const method
        return sqlite3_column_int(static_cast<T const *>(this)->GetAbi(), column);
    }

    // to be able to read rows by columns in the controller or main
    double GetFloat(int const column = 0) const noexcept
    {
        // doing a static cast because GetAbi is a const method
        return sqlite3_column_double(static_cast<T const *>(this)->GetAbi(), column);
    }

    char const * GetString(int const column = 0) const noexcept
    {
        return reinterpret_cast<char const *>(sqlite3_column_text(
                static_cast<T const *>(this)->GetAbi(), column
                ));
    }

    // string overloaded for no regular string
    wchar_t const * GetWideString(int const column = 0) const noexcept
    {
        return static_cast<wchar_t const *>(sqlite3_column_text16(
                static_cast<T const *>(this)->GetAbi(), column
                ));
    }

    int GetStringLength(int const column = 0) const noexcept
    {
        return sqlite3_column_bytes(static_cast<T const *>(this)->GetAbi(), column);
    }

    int GetWideStringLength(int const column = 0) const noexcept
    {
        return sqlite3_column_bytes16(static_cast<T const *>(this)->GetAbi(), column) / sizeof(wchar_t);
    }

    Type GetType(int const column = 0) const noexcept
    {
        // hanlding sqlite3 dynamic type selection
        return static_cast<Type>(sqlite3_column_type(static_cast<T const *>(this)->GetAbi(), column));
    }
};

// ROW READER BASED ON A MODERN C++ FOR LOOP
class Row : public Reader<Row>
{
        sqlite3_stmt * m_statement = nullptr;
    public:
        sqlite3_stmt * GetAbi() const noexcept
        {
            return m_statement;
        }

        Row(sqlite3_stmt * const statement) noexcept :
        m_statement{statement}
        {}
        // row iterator class is at the bottom if this header
};

// SQLITE STATEMENTS HANDLERS

class Statement : public Reader<Statement>
{
    private:
        struct StatementHandleTraits : HandleTraits<sqlite3_stmt *>
        {
            static void Close(Type value) noexcept
            {
                VERIFY_(SQLITE_OK, sqlite3_finalize(value));
            }
        };

        // statement alias
        using StatementHandle = Handle<StatementHandleTraits>;

        StatementHandle m_handle;

        // variatic template ...
        template <typename F, typename C, typename ... Values>
        void InternalPrepare(Connection const & connection, F prepare, C const * const text,
                Values && ... values)
        {
            // in case a bad connection
            assert(connection);
            // STATEMENT TEXT OR QUERY PROCESSING
            if(SQLITE_OK != prepare(connection.GetAbi(), text, -1, m_handle.Set(), nullptr))
            {
                connection.ThrowLastError();
            }
            // to bind in same line Prepare is calling
            BindAll(std::forward<Values>(values)...);
        }

        void InternalBind(int) const noexcept
        {
            // terminanting function for binding all
            // last element of the expansion pack bellow, ends here
        }
        // called for automatic binding
        template <typename First, typename ... Rest>
        void InternalBind(int const index, First && first, Rest && ... rest) const
        {
            // fouding correct binding for each value in the parameter pack
            Bind(index, std::forward<First>(first));
            // the pack minus the element binded yet
            InternalBind(index+1, std::forward<Rest>(rest)...);
        }


    public:
        Statement() noexcept = default;

        // prepare statement in one step in constructor
        template <typename C, typename ... Values>
        Statement(Connection const & connection, C const * const text,
                Values && ... values)
        {
            Prepare(connection, text, std::forward<Values>(values) ...);
        }

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(m_handle);
        }

        // abi
        sqlite3_stmt * GetAbi() const noexcept
        {
            return m_handle.Get();
        }

        void ThrowLastError() const
        {
            throw Exception (sqlite3_db_handle(GetAbi()));
        }

        // prepare overloading for different encodings
        // to accepts any source arguments
        template <typename ... Values>
        void Prepare(Connection const & connection, char const * const text,
                Values && ... values)
        {
            // calling query proccessor
            // prepare_V2 much more stable with error recording
            // std::forward<Values>(values)... is for unpacking the values in template ...
            InternalPrepare(connection, sqlite3_prepare_v2,
                    text, std::forward<Values>(values)...);
        }

        // to accepts any source arguments
        template <typename ... Values>
        void Prepare(Connection const & connection, wchar_t const * const text,
                Values && ... values)
        {
            // calling query proccessor
            InternalPrepare(connection, sqlite3_prepare16_v2,
                    text, std::forward<Values>(values)...);
        }

        bool Step() const
        {
            // statement executions
            int const result = sqlite3_step(GetAbi());
            // it is still executing
            if(result == SQLITE_ROW) return true;
            // it says it has been executed successfully
            if(result == SQLITE_DONE) return false;

            // in case no response is received
            ThrowLastError();
        }

        void Execute() const {
            VERIFY(!Step());
        }

        // BINDING STATEMENT ARGUMENTS
        //integers
        void Bind(int const index, int const value) const
        {
            // zero is not valid in sql index
            // binding inside logic
            if(SQLITE_OK != sqlite3_bind_int(GetAbi(), index, value))
            {
                ThrowLastError();
            }
        }

        // double binding
        void Bind(int const index, double const value) const
        {
            // zero is not valid in sql index
            // binding inside logic
            if(SQLITE_OK != sqlite3_bind_double(GetAbi(), index, value))
            {
                ThrowLastError();
            }
        }

        void Bind(int const index, float const value) const
        {
            Bind(index, static_cast<double>(value));
        }

        // characters
        //-1 sqlite binds all character as default
        void Bind(int const index, char const * const value, int const size = -1) const
        {
            if(SQLITE_OK != sqlite3_bind_text(GetAbi(), index, value, size, SQLITE_STATIC))
            {
                // in case binding is not completed
                ThrowLastError();
            }
        }

        // wchat_t wide characters
        void Bind(int const index, wchar_t const * const value, int const size = -1) const
        {
            // size is in bytes
            // SQLITE_STATIC?
            if(SQLITE_OK != sqlite3_bind_text16(GetAbi(), index, value, size, SQLITE_STATIC))
            {
                ThrowLastError();
            }
        }

        void Bind(int const index, std::string const & value) const
        {
            Bind(index, value.c_str(), value.size());
        }

        void Bind(int const index, std::wstring const & value) const
        {
            Bind(index, value.c_str(), value.size() * sizeof(wchar_t));
        }

        // bindings for avoiding SQLITE_TRANSIENT in Bind chars
        // Overloading to let sqlite doing a private copy
        void Bind(int const index, std::string && value) const
        {
            if(SQLITE_OK != sqlite3_bind_text(GetAbi(),
                    index, value.c_str(),value.size(), SQLITE_TRANSIENT))
            {
                ThrowLastError();
            }
        }

        // for std::wstring binding
        void Bind(int const index, std::wstring && value) const
        {
            if(SQLITE_OK != sqlite3_bind_text16(GetAbi(),
                    index, value.c_str(), value.size() * sizeof(wchar_t), SQLITE_TRANSIENT))
            {
                ThrowLastError();
            }
        }

        // AUTOMATIC BINDING FEATURE METHODS (courtesy of variatic templates)

        //A "template parameter pack" is a template parameter that accepts zero or more template
        // arguments (non-types, types, or templates)
        template <typename ... Values>
        // we dont know what arguments we are receiving
        void BindAll(Values && ... values) const
        {
            // passing all the uncertain values
            InternalBind(1, std::forward<Values>(values) ...);
        }

        template <typename ... Values>
        void Reset(Values && ... values) const {
            // in order to reset the sqlite state machine
            if(SQLITE_OK != sqlite3_reset(GetAbi()))
            {
                ThrowLastError();
            }
            BindAll(values ...);
        }
};

class RowIterator
{
        Statement const  * m_statement = nullptr;

    public:
        // default constructor
        RowIterator() noexcept = default;

        // copy constructor overloaded
        RowIterator(Statement const & statement) noexcept
        {
            if(statement.Step())
            {
                m_statement = &statement;
            }
        }

        // increments operator overloaded
        RowIterator & operator++() noexcept
        {
            if(!m_statement->Step())
            {
                // if iteration is over
                m_statement = nullptr;
            }
            // if it is not iver then return the self object
            return *this;
        }

        // different than operator overloaded
        bool operator!=(RowIterator const & other) const noexcept
        {
            return m_statement != other.m_statement;
        }
        // row class is above Statement class
        Row operator *() const noexcept
        {
            return Row(m_statement->GetAbi());
        }
};

//NO MEMBER FUNCTION HELPERS

// c++ for auto& e : statement calls begin and end
inline RowIterator begin(Statement const & statement) noexcept
{
    return RowIterator(statement);
}

inline RowIterator end(Statement const &) noexcept
{
    // nullptr?
    return RowIterator();
}


// To be able to execute queries and binding data inline with Connection creation
template <typename C, typename ... Values>
void Execute(Connection const & connection, C const * const text,
        Values && ... values)
{
    Statement(connection, text, std::forward<Values>(values) ...).Execute();
}