// debug_overlay.cpp

#include "spinach.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct vertex2d
{
   float x, y;
   float u, v;
};

static void
update_debug_text(vertex_buffer &buffer, int &count, const int scale, const float x, float y, const std::vector<std::string> &lines)
{
   std::size_t capacity = 0;
   for (const auto &text : lines) {
      capacity += text.size() * 6;
   }

   std::vector<float> vertices;
   vertices.reserve(capacity);

   const int32 character_column_count = 16;
   const int32 character_row_count = 16;
   const float character_width = 8.0f;
   const float character_height = 8.0f;
   const float newline_advance_y = 10.0f;
   const float tab_advance_x = character_width * 4;
   const float texture_width_factor = 1.0f / character_column_count;
   const float texture_height_factor = 1.0f / character_row_count;

   const int32 first_valid_character = ' ';
   const int32 last_valid_character = '~';
   const int32 invalid_character_index = '?';

   float x_position = x;
   float y_position = y;

   for (const auto &text : lines) {
      for (auto &ch : text) {
         int32 character_index = static_cast<int32>(ch);
         if (character_index == int32('\n')) {
            x_position = x;
            y_position += newline_advance_y * scale;
            continue;
         }
         else if (character_index == int32('\t')) {
            x_position += tab_advance_x;
            continue;
         }

         if (character_index < first_valid_character ||
             character_index > last_valid_character)
         {
            character_index = invalid_character_index;
         }

         const float x0 = x_position;
         const float y0 = y_position;
         const float x1 = x0 + character_width * scale;
         const float y1 = y0 + character_height * scale;

         const float u0 = (character_index % character_column_count) * texture_width_factor;
         const float v0 = (character_index / character_column_count) * texture_height_factor;
         const float u1 = u0 + texture_width_factor;
         const float v1 = v0 + texture_height_factor;

         const float verts[] =
         {
            x0, y0, u0, v0,
            x1, y0, u1, v0,
            x1, y1, u1, v1,

            x1, y1, u1, v1,
            x0, y1, u0, v1,
            x0, y0, u0, v0,
         };

         for (auto &vert : verts) {
            vertices.push_back(vert);
         }

         x_position += character_width * scale;
      }

      x_position = x;
   }

   if (!buffer.is_valid()) {
      if (!buffer.create(sizeof(vertex2d) * int(vertices.size()), vertices.data(), BUFFER_USAGE_HINT_DYNAMIC)) {
         assert(false);
      }
   }
   else {
      buffer.update(sizeof(vertex2d) * int(vertices.size()), vertices.data());
   }

   count = int(vertices.size());
}

debug_overlay::debug_overlay(shader_program *program, 
                             texture *texture, 
                             sampler_state *sampler,
                             vertex_layout *layout)
   : m_projection(glm::mat4(1.0f))
   , m_material(program, texture, sampler)
   , m_layout(layout)
{
}

void debug_overlay::push_line(const char *format, ...)
{
   char message[1024] = {};
   va_list args;
   va_start(args, format);
   vsprintf_s(message, format, args);
   va_end(args);

   m_lines.emplace_back(message);
}

void debug_overlay::pre_frame(const int width, const int height)
{
   m_lines.clear();
   m_projection = glm::ortho(0.0f, float(width), float(height), 0.0f);
   m_material.set_parameter("u_projection", m_projection);
}

void debug_overlay::draw(render_backend &backend)
{
   update_debug_text(m_buffer, m_vertex_count, 2, 2.0f, 2.0f, m_lines);

   m_material.bind(backend);

   backend.set_vertex_buffer(m_buffer);
   backend.set_vertex_layout(*m_layout);
   backend.set_blend_state(true);
   backend.set_depth_state(false, true);
   backend.set_rasterizer_state(CULL_MODE_NONE, FRONT_FACE_CW);
   backend.draw(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, m_vertex_count);
}
