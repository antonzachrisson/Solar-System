// keyboard.cpp

#include "spinach.hpp"

bool keyboard::key_down(const int index) const
{
   return m_keys[index].m_current;
}

bool keyboard::key_pressed(const int index) const
{
   return m_keys[index].m_current && !m_keys[index].m_previous;
}

bool keyboard::key_released(const int index) const
{
   return !m_keys[index].m_current && m_keys[index].m_previous;
}

void keyboard::update()
{
   for (auto &key : m_keys) {
      key.m_previous = key.m_current;
   }
}

void keyboard::on_key(const int index, bool state)
{
   m_keys[index].m_current = state;
}
