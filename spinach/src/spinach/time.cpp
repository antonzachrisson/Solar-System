// time.cpp

#include "spinach.hpp"

// static
time time::now()
{
   return time{ utility::get_current_tick() };
}

time time::deltatime()
{
   // note: call only one per frame!
   static time p = time::now();
   time n = time::now();
   time d = n - p;
   p = n;
   return d;
}

time &time::operator+=(const time & rhs)
{
   m_duration += rhs.m_duration;
   return *this;
}

time &time::operator-=(const time & rhs)
{
   m_duration -= rhs.m_duration;
   return *this;
}

time time::operator+(const time & rhs) const
{
   return { m_duration + rhs.m_duration };
}

time time::operator-(const time & rhs) const
{
   return { m_duration - rhs.m_duration };
}

float time::as_seconds() const
{
   return float(m_duration) / 1000.0f;
}

float time::as_milliseconds() const
{
   return float(m_duration);
}