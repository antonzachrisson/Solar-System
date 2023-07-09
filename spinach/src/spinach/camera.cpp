// camera.cpp

#include "spinach.hpp"
#include "render.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

camera::camera(const glm::mat4 &projection)
   : m_right(0.0f)
   , m_up(0.0f)
   , m_forward(0.0f)
   , m_position(0.0f, 0.0f, 0.0f)
   , m_view(1.0f)
   , m_projection(projection)
{
}

void camera::update()
{
   glm::vec3 ax = { 1.0f, 0.0f, 0.0f };
   glm::vec3 ay = { 0.0f, 1.0f, 0.0f };
   glm::vec3 az = { 0.0f, 0.0f, 1.0f };

   glm::mat4 ry = glm::rotate(glm::mat4(1.0f), -m_yaw, ay);
   ax = glm::normalize(glm::mat3(ry) * ax);
   az = glm::normalize(glm::mat3(ry) * az);
   
   glm::mat4 rx = glm::rotate(glm::mat4(1.0f), -m_pitch, ax);
   ay = glm::normalize(glm::mat3(rx) * ay);
   az = glm::normalize(glm::mat3(rx) * az);
   
   m_view[0][0] = ax.x; m_view[0][1] = ay.x; m_view[0][2] = az.x;
   m_view[1][0] = ax.y; m_view[1][1] = ay.y; m_view[1][2] = az.y;
   m_view[2][0] = ax.z; m_view[2][1] = ay.z; m_view[2][2] = az.z;
   m_view[3][0] = -glm::dot(m_position, ax);
   m_view[3][1] = -glm::dot(m_position, ay);
   m_view[3][2] = -glm::dot(m_position, az);

   //glm::mat4 rotation = glm::eulerAngleXYZ(m_pitch, m_yaw, m_roll);
   //glm::mat4 translation = glm::translate(glm::mat4(1.0f), -m_position);
   //m_view = rotation * translation;
   //
   //ax = glm::vec3(m_view[0][0], m_view[1][0], m_view[2][0]);
   //ay = glm::vec3(m_view[0][1], m_view[1][1], m_view[2][1]);
   //az = glm::vec3(m_view[0][2], m_view[1][2], m_view[2][2]);

   //glm::quat rx = glm::angleAxis(m_pitch, ax);
   //glm::quat ry = glm::angleAxis(m_yaw, ay);
   //glm::quat rz = glm::angleAxis(m_roll, az);
   //
   //glm::quat orientation = glm::normalize(rx * ry * rz);
   //glm::mat4 rotation = glm::mat4_cast(orientation);
   //glm::mat4 translation = glm::translate(glm::mat4(1.0f), -m_position);
   //m_view = rotation * translation;
   //
   //ax = glm::normalize(glm::conjugate(orientation) * ax);
   //ay = glm::normalize(glm::conjugate(orientation) * ay);
   //az = glm::normalize(glm::conjugate(orientation) * az);

   m_right = ax;
   m_up = ay;
   m_forward = az;
}

void camera::move_x(const float amount)
{
   m_position += m_right * amount;
}

void camera::move_y(const float amount)
{
   m_position += m_up * amount;
}

void camera::move_z(const float amount)
{
   m_position += m_forward * amount;
}

void camera::rotate_x(const float amount)
{
   m_pitch += amount;
}

void camera::rotate_y(const float amount)
{
   m_yaw += amount;
}

void camera::rotate_z(const float amount)
{
   m_roll += amount;
}

void camera::set_projection(const glm::mat4 &projection)
{
   m_projection = projection;
}

void camera::bind(render_backend &backend, const shader_program &program)
{
   backend.set_shader_program(program);
   backend.set_shader_uniform(program, UNIFORM_TYPE_MATRIX, "u_projection", 1, glm::value_ptr(m_projection));
   backend.set_shader_uniform(program, UNIFORM_TYPE_MATRIX, "u_view", 1, glm::value_ptr(m_view));
}
