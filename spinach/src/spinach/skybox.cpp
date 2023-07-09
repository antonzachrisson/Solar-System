// skybox.cpp

#include "spinach.hpp"
#include <glm/gtc/type_ptr.hpp>

bool skybox::valid() const
{
   return m_program.is_valid() &&
      m_cubemap.is_valid() &&
      m_sampler.is_valid() &&
      m_buffer.is_valid();
}

bool skybox::create(const char *path)
{
   std::string base_path(path);
   if (base_path.back() != '/' ||
       base_path.back() != '\\')
   {
      base_path.append("/");
   }

   std::string vsource_path = base_path + "shader.vs.glsl";
   std::string fsource_path = base_path + "shader.fs.glsl";

   // note: create shader program
   if (!utility::create_shader_program_from_files(m_program, vsource_path.c_str(), fsource_path.c_str())) {
      debug::log("could not create shader program - path: '%s'", path);
      return false;
   }

   // note: load images and create cubemap
   std::string faces[6];
   const char *names[6] = { "xpos.jpg", "xneg.jpg", "ypos.jpg", "yneg.jpg", "zpos.jpg", "zneg.jpg" };
   const char *files[6] = { nullptr };
   for (int i = 0; i < 6; i++) {
      faces[i] = base_path + names[i];
      files[i] = faces[i].c_str();
   }

   if (!utility::create_cubemap_from_files(m_cubemap, 6, files)) {
      return false;
   }

   // note: create sampler state
   if (!m_sampler.create(SAMPLER_FILTER_MODE_LINEAR, SAMPLER_ADDRESS_MODE_CLAMP, SAMPLER_ADDRESS_MODE_CLAMP)) {
      return false;
   }


   { // note: create vertex buffer and set vertex layout attributes
      const float Q = 2.0f;
      const glm::vec3 vertices[] =
      {
         // x positive
         {  Q,  Q, -Q },
         {  Q,  Q,  Q },
         {  Q, -Q,  Q },
         {  Q, -Q,  Q },
         {  Q, -Q, -Q },
         {  Q,  Q, -Q },

         // x negative
         { -Q,  Q,  Q },
         { -Q,  Q, -Q },
         { -Q, -Q, -Q },
         { -Q, -Q, -Q },
         { -Q, -Q,  Q },
         { -Q,  Q,  Q },

         // y positive
         { -Q,  Q,  Q },
         {  Q,  Q,  Q },
         {  Q,  Q, -Q },
         {  Q,  Q, -Q },
         { -Q,  Q, -Q },
         { -Q,  Q,  Q },

         // y negative
         { -Q, -Q, -Q },
         {  Q, -Q, -Q },
         {  Q, -Q,  Q },
         {  Q, -Q,  Q },
         { -Q, -Q,  Q },
         { -Q, -Q, -Q },

         // z positive
         { -Q,  Q, -Q },
         {  Q,  Q, -Q },
         {  Q, -Q, -Q },
         {  Q, -Q, -Q },
         { -Q, -Q, -Q },
         { -Q,  Q, -Q },

         // z negative
         {  Q,  Q,  Q },
         { -Q,  Q,  Q },
         { -Q, -Q,  Q },
         { -Q, -Q,  Q },
         {  Q, -Q,  Q },
         {  Q,  Q,  Q },
      };

      m_primitive_count = sizeof(vertices) / sizeof(vertices[0]);
      if (!m_buffer.create(sizeof(vertices), vertices)) {
         return false;
      }

      m_layout.add_attribute(0, vertex_layout::ATTRIBUTE_FORMAT_FLOAT, 3, false);
   }

   return true;
}

void skybox::destroy()
{
   m_program.destroy();
   m_cubemap.destroy();
   m_sampler.destroy();
   m_buffer.destroy();
}

void skybox::draw(render_backend &backend, const camera &camera)
{
   glm::mat4 proj = camera.m_projection;
   glm::mat4 view = camera.m_view;
   view[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

   backend.set_shader_program(m_program);
   backend.set_shader_uniform(m_program, UNIFORM_TYPE_MATRIX, "u_projection", 1, glm::value_ptr(proj));
   backend.set_shader_uniform(m_program, UNIFORM_TYPE_MATRIX, "u_view", 1, glm::value_ptr(view));
   backend.set_vertex_buffer(m_buffer);
   backend.set_vertex_layout(m_layout);
   backend.set_cubemap(m_cubemap);
   backend.set_sampler_state(m_sampler);
   backend.set_blend_state(false);
   backend.set_depth_state(false, false);
   backend.set_rasterizer_state(CULL_MODE_NONE, FRONT_FACE_CCW, POLYGON_MODE_FILL);
   backend.draw(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, m_primitive_count);
}
