// material.cpp

#include "spinach.hpp"

material::parameter::parameter(uniform_type type,
                               std::string  name,
                               const glm::vec4 &data)
   : m_type(type)
   , m_name(std::move(name))
   , m_data{}
{
   m_data.m_v4 = data;
}

material::parameter::parameter(uniform_type type,
                               std::string  name,
                               const glm::mat4 &data)
   : m_type(type)
   , m_name(std::move(name))
   , m_data{}
{
   m_data.m_m4 = data;
}

material::material(const shader_program *program, const texture *texture, const sampler_state *sampler)
   : m_program(program)
   , m_texture(texture)
   , m_sampler(sampler)
{
}

void material::set_shader_program(const shader_program *program)
{
   m_program = program;
}

void material::set_texture(const texture *texture)
{
   m_texture = texture;
}

void material::set_sampler_state(const sampler_state *sampler)
{
   m_sampler = sampler;
}

void material::set_parameter(const std::string &name, const glm::vec2 &value)
{
   auto data = glm::vec4(value, 0, 0);
   for (auto &param : m_parameters) {
      if (param.m_name == name) {
         assert(param.m_type == UNIFORM_TYPE_VEC2);
         param.m_data.m_v4 = data;
         return;
      }
   }

   m_parameters.push_back(parameter{ UNIFORM_TYPE_VEC2, name, data });
}

void material::set_parameter(const std::string &name, const glm::vec3 &value)
{
   auto data = glm::vec4(value, 0);
   for (auto &param : m_parameters) {
      if (param.m_name == name) {
         assert(param.m_type == UNIFORM_TYPE_VEC3);
         param.m_data.m_v4 = data;
         return;
      }
   }

   m_parameters.push_back(parameter{ UNIFORM_TYPE_VEC3, name, data });
}

void material::set_parameter(const std::string &name, const glm::vec4 &value)
{
   auto data = value;
   for (auto &param : m_parameters) {
      if (param.m_name == name) {
         assert(param.m_type == UNIFORM_TYPE_VEC4);
         param.m_data.m_v4 = data;
         return;
      }
   }

   m_parameters.push_back(parameter{ UNIFORM_TYPE_VEC4, name, data });
}

void material::set_parameter(const std::string &name, const glm::mat4 &value)
{
   for (auto &param : m_parameters) {
      if (param.m_name == name) {
         assert(param.m_type == UNIFORM_TYPE_MATRIX);
         param.m_data.m_m4 = value;
         return;
      }
   }

   m_parameters.push_back(parameter{ UNIFORM_TYPE_MATRIX, name, value });
}

void material::bind(render_backend &backend)
{
   backend.set_shader_program(*m_program);
   for (auto &param : m_parameters) {
      backend.set_shader_uniform(*m_program, 
                                 param.m_type, 
                                 param.m_name.c_str(), 
                                 1, 
                                 (const void *)&param.m_data);
   }
   backend.set_texture(*m_texture);
   backend.set_sampler_state(*m_sampler);
}
