// mouse.cpp

#include "spinach.hpp"

int mouse::x() const
{
   return m_x;
}

int mouse::y() const
{
   return m_y;
}

bool mouse::button_down(const int index) const
{
   return m_buttons[index].m_current;
}

bool mouse::button_pressed(const int index) const
{
   return m_buttons[index].m_current && !m_buttons[index].m_previous;
}

bool mouse::button_released(const int index) const
{
   return !m_buttons[index].m_current && m_buttons[index].m_previous;
}

void mouse::update()
{
   for (auto &button : m_buttons) {
      button.m_previous = button.m_current;
   }
}

void mouse::on_move(const int x, const int y)
{
   m_x = x;
   m_y = y;
}

void mouse::on_button(const int index, bool state)
{
   m_buttons[index].m_current = state;
}
