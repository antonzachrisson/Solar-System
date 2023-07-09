// controller.cpp

#include "spinach.hpp"
#include <GLFW/glfw3.h>

controller::controller(camera &camera)
   : m_camera(camera)
   , m_speed(10.0f)
   , m_yaw(0.022f)
   , m_pitch(-0.022f)
   , m_sensitivity(0.05f)
   , m_mouse_position(0.0f)
{
}

void controller::update(const keyboard &keyboard, const mouse &mouse, const time &deltatime)
{
   if (keyboard.key_down(GLFW_KEY_W)) {
      m_camera.move_z(-m_speed * deltatime.as_seconds());
   }
   if (keyboard.key_down(GLFW_KEY_S)) {
      m_camera.move_z(m_speed * deltatime.as_seconds());
   }
   if (keyboard.key_down(GLFW_KEY_A)) {
      m_camera.move_x(-m_speed * deltatime.as_seconds());
   }
   if (keyboard.key_down(GLFW_KEY_D)) {
      m_camera.move_x(m_speed * deltatime.as_seconds());
   }
   if (keyboard.key_down(GLFW_KEY_Q)) {
      m_camera.move_y(-m_speed * deltatime.as_seconds());
   }
   if (keyboard.key_down(GLFW_KEY_E)) {
      m_camera.move_y(m_speed * deltatime.as_seconds());
   }

   const glm::vec2 mouse_position(float(mouse.x()), float(mouse.y()));
   const glm::vec2 mouse_delta = mouse_position - m_mouse_position;
   if (mouse.button_down(GLFW_MOUSE_BUTTON_LEFT)) {
      m_camera.rotate_x(mouse_delta.y * m_sensitivity * m_pitch);
      m_camera.rotate_y(mouse_delta.x * m_sensitivity * m_yaw);
   }

   m_mouse_position = mouse_position;
   m_camera.update();
}

void controller::set_camera_speed(const float speed)
{
   m_speed = speed;
}

void controller::set_mouse_invert_yaw(const bool invert)
{
   m_yaw = invert ? -m_yaw : m_yaw;
}

void controller::set_mouse_invert_pitch(const bool invert)
{
   m_pitch = invert ? -m_pitch : m_pitch;
}

void controller::set_mouse_sensitivity(const float sensitivity)
{
   m_sensitivity = sensitivity;
}
