//
// Created by cheetos on 16/12/18.
//
#pragma once

#include <assert.h>

/*
 * CLASS TO HANDLE THE SQLITE RESOURCES
 * */

template <typename T>
struct HandleTraits
{
    // alias Type
    using Type = T;

    static Type Invalid() noexcept
    {
        return nullptr;
    }
};

template <typename Traits>
class Handle
{
        using Type = decltype(Traits::Invalid());
        Type m_value;

        void Close() noexcept
        {
            if(*this)
            {
                Traits::Close(m_value);
            }
        }

    public:
        // delete Copy constructor
        Handle(Handle const &) = delete;
        // delete asignation operator
        Handle & operator=(Handle const &) = delete;

        //The explicit function specifier controls unwanted implicit type conversions.
        explicit Handle(Type value = Traits::Invalid()) noexcept :
        m_value(value)
        { }

        // reset
        Handle(Handle && other) noexcept :
        m_value{other.Detach()}
        {}

        // move assignment operator
        Handle & operator=(Handle && other) noexcept
        {
            // check for auto assignment
            if(this != &other)
            {
                Reset(other.Detach());
            }
            return *this;
        }

        ~Handle() noexcept
        {
            Close();
        }

        explicit operator bool() const noexcept
        {
            return m_value != Traits::Invalid();
        }

        Type Get() const noexcept
        {
            return m_value;
        }

        Type * Set() noexcept
        {
            assert(!*this);
            // return address, because of the SQLite C api
            return &m_value;
        }

        Type Detach() noexcept
        {
            // detach the ownership of some handle
            Type value = m_value;
            m_value = Traits::Invalid();
            return value;
        }

        bool Reset(Type value = Traits::Invalid()) noexcept
        {
            if (m_value != value)
            {
                Close();
                // assume ownnership
                m_value = value;
            }
            // maybe to do a validity chech as a bonus
            return static_cast<bool>(*this);
        }

        void Swap(Handle<Traits> &other) noexcept
        {
            // member swap
            Type temp = m_value;
            m_value = other.m_value;
            other.m_value = temp;
        }
};

// no member swap
template <typename Traits>
void Swap(Handle<Traits> &left, Handle<Traits> &right) noexcept
{
    left.Swap(right);
}

// operators
template <typename Traits>
bool operator==(Handle<Traits> const & left, Handle<Traits> const & right) noexcept
{
    return left.Get() == right.Get();
}

template <typename Traits>
bool operator !=(Handle<Traits> const & left, Handle<Traits> const & right) noexcept
{
    // calling the operator ==
    return !(left == right);
}