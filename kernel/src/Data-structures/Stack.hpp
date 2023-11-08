/*
Copyright (©) 2023  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _STACK_HPP
#define _STACK_HPP

#include <stddef.h>

template <typename T>
class Stack {
public:
    Stack() : m_data(nullptr), m_size(0), m_currentPos(0) {
        
    }

    Stack(size_t size) : m_data(nullptr), m_size(size), m_currentPos(0) {
        m_data = new T*[m_size];
    }

    ~Stack() {
        delete[] m_data;
    }

    void Push(T* data) {
        if (isFull())
            return;

        m_data[m_currentPos++] = data;
    }

    T* Pop() {
        if (isEmpty())
            return nullptr;

        return m_data[--m_currentPos];
    }

    T* Peek() {
        if (isEmpty())
            return nullptr;

        return m_data[m_currentPos - 1];
    }

    bool isEmpty() {
        return m_currentPos == 0;
    }

    bool isFull() {
        return m_currentPos == m_size;
    
    }

    void clear() {
        m_currentPos = 0;
    }

private:
    T** m_data;
    size_t m_size;
    size_t m_currentPos;
};

#endif /* _STACK_HPP */