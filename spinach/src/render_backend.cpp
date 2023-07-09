// render_backend.cpp

#include "render.hpp"

#include <cassert>
#include <glad/glad.h>

template<class T, size_t N>
constexpr size_t array_size(T(&)[N])
{
   return N;
}

static const GLenum gl_uniform_type[] =
{
   GL_FLOAT,
   GL_FLOAT_VEC2,
   GL_FLOAT_VEC3,
   GL_FLOAT_VEC4,
   GL_INT,
   GL_BOOL,
   GL_SAMPLER_2D,
   GL_FLOAT_MAT4,
};

static const GLuint gl_uniform_size[] =
{
   sizeof(float),
   sizeof(float) * 2,
   sizeof(float) * 3,
   sizeof(float) * 4,
   sizeof(int),
   sizeof(int),
   sizeof(int),
   sizeof(float) * 16,
};

static const GLenum gl_texture_format_internal[] =
{
   GL_RGB8,
   GL_RGBA8,
};

static const GLenum gl_texture_format[] =
{
   GL_RGB,
   GL_RGBA,
};

static const GLenum gl_texture_format_type[] =
{
   GL_UNSIGNED_BYTE,
   GL_UNSIGNED_BYTE,
};

static const GLenum gl_sampler_filter[] =
{
   GL_NEAREST,
   GL_LINEAR,
   GL_NEAREST_MIPMAP_NEAREST,
   GL_NEAREST_MIPMAP_LINEAR,
   GL_LINEAR_MIPMAP_NEAREST,
   GL_LINEAR_MIPMAP_LINEAR,
};

static const GLenum gl_sampler_address[] =
{
   GL_CLAMP_TO_EDGE,
   GL_REPEAT,
   GL_MIRRORED_REPEAT
};

const GLenum gl_buffer_usage_hint[] =
{
   GL_STATIC_DRAW,
   GL_STREAM_DRAW,
};

static const GLenum gl_blend_eq[] =
{
   GL_FUNC_ADD,
   GL_FUNC_SUBTRACT,
   GL_FUNC_REVERSE_SUBTRACT,
   GL_MIN,
   GL_MAX,
};

static const GLenum gl_blend_ft[] =
{
   GL_ZERO,
   GL_ONE,
   GL_SRC_COLOR,
   GL_ONE_MINUS_SRC_COLOR,
   GL_DST_COLOR,
   GL_ONE_MINUS_DST_COLOR,
   GL_SRC_ALPHA,
   GL_ONE_MINUS_SRC_ALPHA,
   GL_DST_ALPHA,
   GL_ONE_MINUS_DST_ALPHA,
   GL_CONSTANT_COLOR,
   GL_ONE_MINUS_CONSTANT_COLOR,
   GL_CONSTANT_ALPHA,
   GL_ONE_MINUS_CONSTANT_ALPHA,
   GL_SRC_ALPHA_SATURATE,
};

static const GLenum gl_compare_func[] =
{
   GL_NEVER,
   GL_LESS,
   GL_EQUAL,
   GL_LEQUAL,
   GL_GREATER,
   GL_NOTEQUAL,
   GL_GEQUAL,
   GL_ALWAYS,
};

static const GLenum gl_stencil_op[] =
{
   GL_KEEP,
   GL_ZERO,
   GL_REPLACE,
   GL_INCR,
   GL_INCR_WRAP,
   GL_DECR,
   GL_DECR_WRAP,
   GL_INVERT,
};

static const GLenum gl_cull_mode[] =
{
   GL_NONE,
   GL_BACK,
   GL_FRONT,
   GL_FRONT_AND_BACK,
};

static const GLenum gl_front_face[] =
{
   GL_CCW,
   GL_CW,
};

static const GLenum gl_primitive_topology[] =
{
   GL_POINTS,
   GL_LINES,
   GL_TRIANGLES,
};

static const GLenum gl_index_type[] =
{
   GL_UNSIGNED_BYTE,
   GL_UNSIGNED_SHORT,
   GL_UNSIGNED_INT,
};

static const GLuint gl_index_size[] =
{
   sizeof(uint8),
   sizeof(uint16),
   sizeof(uint32),
};

const GLenum gl_framebuffer_format_internal[] =
{
   GL_NONE,
   GL_RGB8,
   GL_RGBA8,
   GL_DEPTH24_STENCIL8,
};

const GLenum gl_framebuffer_format[] =
{
   GL_NONE,
   GL_RGB,
   GL_RGBA,
   GL_DEPTH_STENCIL,
};

static GLenum gl_framebuffer_type[] =
{
   GL_NONE,
   GL_UNSIGNED_BYTE,
   GL_UNSIGNED_BYTE,
   GL_UNSIGNED_INT_24_8,
};

static const GLenum gl_attribute_type[] =
{
   GL_FLOAT,
   GL_UNSIGNED_BYTE,
};

static const GLuint gl_attribute_size[] =
{
   sizeof(float),
   sizeof(char),
};

shader_program::shader_program()
   : id_(0)
{
}

bool shader_program::is_valid() const
{
   return id_ != 0;
}

bool shader_program::create(const char *vertex_shader_source,
                            const char *fragment_shader_source)
{
   GLuint vid = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vid, 1, &vertex_shader_source, nullptr);
   glCompileShader(vid);

   GLuint fid = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fid, 1, &fragment_shader_source, NULL);
   glCompileShader(fid);

   GLuint pid = glCreateProgram();
   glAttachShader(pid, vid);
   glAttachShader(pid, fid);
   glLinkProgram(pid);

   GLint link_status = 0;
   glGetProgramiv(pid, GL_LINK_STATUS, &link_status);
   if (link_status == GL_FALSE) {
      GLchar vertex_error[1024]{};
      glGetShaderInfoLog(fid, sizeof(vertex_error), NULL, vertex_error);

      GLchar fragment_error[1024]{};
      glGetShaderInfoLog(fid, sizeof(fragment_error), NULL, fragment_error);

      GLchar program_error[1024];
      glGetProgramInfoLog(pid, sizeof(program_error), NULL, program_error);
      //debug::error_box("ERROR!", "[program-log]:\n%s", program_error);
      assert(!"shader program error");

      glDetachShader(pid, vid);
      glDetachShader(pid, fid);
      glDeleteProgram(pid);
   }
   else {
      id_ = pid;
   }

   glDeleteShader(vid);
   glDeleteShader(fid);

   // note: setting the sampler uniform locations in the order
   //       the sampler uniforms are defined in the shaders 
   glUseProgram(pid);
   GLint sampler_count = 0;
   GLint active_uniform_count = 0;
   glGetProgramiv(pid, GL_ACTIVE_UNIFORMS, &active_uniform_count);
   for (int32 index = 0; index < active_uniform_count; index++) {
      GLint size = 0;
      GLenum type;
      GLchar name[128] = {};
      glGetActiveUniform(pid, index, sizeof(name), nullptr, &size, &type, name);
      if (type == GL_SAMPLER_2D ||
          type == GL_SAMPLER_CUBE) {
         GLint location = glGetUniformLocation(pid, name);
         if (location == -1) {
            continue;
         }

         glUniform1i(location, sampler_count);
         sampler_count++;
      }
   }

   return is_valid();
}

void shader_program::destroy()
{
   glDeleteProgram(id_);
   id_ = 0;
}

texture::texture()
   : id_(0)
{
}

bool texture::is_valid() const
{
   return id_ != 0;
}

bool texture::create(const texture_format format,
                     const int32 width,
                     const int32 height,
                     const void *data)
{
   GLuint id = 0;
   glGenTextures(1, &id);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, id);
   glTexImage2D(GL_TEXTURE_2D,
                0, // mip level
                gl_texture_format_internal[format],
                width,
                height,
                0,
                gl_texture_format[format],
                gl_texture_format_type[format],
                data);
   glBindTexture(GL_TEXTURE_2D, 0);

   id_ = id;

   return is_valid();
}

bool texture::create(const texture_format format,
                     const int32 width,
                     const int32 height,
                     const int32 count,
                     const void **data)
{
   GLuint id = 0;
   glGenTextures(1, &id);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, id);
   for (int32 index = 0; index < count; index++) {
      glTexImage2D(GL_TEXTURE_2D,
                   index, // mip level
                   gl_texture_format_internal[format],
                   (width >> index),
                   (height >> index),
                   0,
                   gl_texture_format[format],
                   gl_texture_format_type[format],
                   data[index]);
   }

   glBindTexture(GL_TEXTURE_2D, 0);

   id_ = id;

   return is_valid();
}

void texture::update(const texture_format format,
                     const int32 width,
                     const int32 height,
                     const void *data)
{
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, id_);
   glTexImage2D(GL_TEXTURE_2D,
                0,
                gl_texture_format_internal[format],
                width,
                height,
                0,
                gl_texture_format[format],
                GL_UNSIGNED_BYTE,
                data);
   glBindTexture(GL_TEXTURE_2D, 0);
}

void texture::destroy()
{
   glBindTexture(GL_TEXTURE_2D, 0);
   glDeleteTextures(1, &id_);
   id_ = 0;
}

cubemap::cubemap()
   : id_(0)
{
}

bool cubemap::is_valid() const
{
   return id_ != 0;
}

bool cubemap::create(const texture_format format,
                     const int32 width,
                     const int32 height,
                     const void *data[6])
{
   GLuint id = 0;
   glGenTextures(1, &id);
   glBindTexture(GL_TEXTURE_CUBE_MAP, id);
   for (int32 index = 0; index < 6; index++) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index,
                   0,
                   gl_texture_format_internal[format],
                   width,
                   height,
                   0,
                   gl_texture_format[format],
                   GL_UNSIGNED_BYTE,
                   data[index]);
   }

   id_ = id;

   return is_valid();
}

void cubemap::destroy()
{
   glBindTexture(GL_TEXTURE_2D, 0);
   glDeleteTextures(1, &id_);
   id_ = 0;
}

vertex_buffer::vertex_buffer()
   : id_(0)
   , size_(0)
{
}

bool vertex_buffer::is_valid() const
{
   return id_ != 0;
}

bool vertex_buffer::create(const int32 size,
                           const void *data,
                           const buffer_usage_hint hint)
{
   GLuint id = 0;
   glGenBuffers(1, &id);
   glBindBuffer(GL_ARRAY_BUFFER, id);
   glBufferData(GL_ARRAY_BUFFER, size, data, gl_buffer_usage_hint[hint]);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   id_ = id;
   size_ = size;

   return is_valid();
}

void vertex_buffer::update(const int32 size,
                           const void *data)
{
   glBindBuffer(GL_ARRAY_BUFFER, id_);
   if (size < size_) {
      glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
   }
   else {
      glBufferData(GL_ARRAY_BUFFER, size_, 0, GL_STREAM_DRAW);
      glBufferData(GL_ARRAY_BUFFER, size, data, GL_STREAM_DRAW);
      size_ = size;
   }
   glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void vertex_buffer::destroy()
{
   glDeleteBuffers(1, &id_);
   id_ = 0;
   size_ = 0;
}

index_buffer::index_buffer()
   : id_(0)
{
}

bool index_buffer::is_valid() const
{
   return id_ != 0;
}

bool index_buffer::create(const int32 size,
                          const void *data)
{
   GLuint id = 0;
   glGenBuffers(1, &id);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   id_ = id;

   return is_valid();
}

void index_buffer::destroy()
{
   glDeleteBuffers(1, &id_);
   id_ = 0;
}

sampler_state::sampler_state()
   : id_(0)
{
}

bool sampler_state::is_valid() const
{
   return id_ != 0;
}

bool sampler_state::create(const sampler_filter_mode filter,
                           const sampler_address_mode addr_u,
                           const sampler_address_mode addr_v,
                           const sampler_address_mode addr_w)
{
   GLuint id = 0;
   glGenSamplers(1, &id);
   //glBindSampler(0, id); // note: needed?
   glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, gl_sampler_filter[filter]);
   glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, filter == SAMPLER_FILTER_MODE_NEAREST ? GL_NEAREST : GL_LINEAR);
   glSamplerParameteri(id, GL_TEXTURE_WRAP_S, gl_sampler_address[addr_u]);
   glSamplerParameteri(id, GL_TEXTURE_WRAP_T, gl_sampler_address[addr_v]);
   glSamplerParameteri(id, GL_TEXTURE_WRAP_R, gl_sampler_address[addr_w]);

   id_ = id;

   return is_valid();
}

void sampler_state::destroy()
{
   glDeleteSamplers(1, &id_);
   id_ = 0;
}

framebuffer::framebuffer()
   : id_(0)
   , width_(0)
   , height_(0)
   , depth_attachment_{}
   , color_attachments_{}
{
}

bool framebuffer::is_valid() const
{
   return id_ != 0;
}

bool framebuffer::create(const int32 width,
                         const int32 height,
                         const int32 color_attachment_format_count,
                         const framebuffer_format *color_attachment_formats,
                         const bool has_depth_attachment,
                         const framebuffer_format depth_attachment_format)
{
   assert(width > 0);
   assert(height > 0);
   assert(color_attachment_format_count > 0);
   assert(color_attachment_format_count < framebuffer::ATTACHMENT_LIMIT);

   GLuint id = 0;
   glGenFramebuffers(1, &id);
   glBindFramebuffer(GL_FRAMEBUFFER, id);

   GLuint textures[framebuffer::ATTACHMENT_LIMIT] = {};
   glGenTextures(color_attachment_format_count, textures);

   int32 color_attachment_count = 0;
   int32 depth_attachment_count = 0;

   for (int32 attachment_index = 0;
        attachment_index < color_attachment_format_count;
        attachment_index++)
   {
      const framebuffer_format format = color_attachment_formats[attachment_index];

      glBindTexture(GL_TEXTURE_2D, textures[attachment_index]);
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   gl_framebuffer_format_internal[format],
                   width,
                   height,
                   0,
                   gl_framebuffer_format[format],
                   gl_framebuffer_type[format],
                   NULL);

      if (format == FRAMEBUFFER_FORMAT_D32) {
         assert(depth_attachment_count < 1);
         glFramebufferTexture2D(GL_FRAMEBUFFER,
                                GL_DEPTH_STENCIL_ATTACHMENT,
                                GL_TEXTURE_2D,
                                textures[attachment_index],
                                0);
         depth_attachment_count++;
      }
      else {
         glFramebufferTexture2D(GL_FRAMEBUFFER,
                                GL_COLOR_ATTACHMENT0 + color_attachment_count,
                                GL_TEXTURE_2D,
                                textures[attachment_index],
                                0);
         color_attachment_count++;
      }
   }

   GLuint rbo = 0;
   if (has_depth_attachment) {
      glGenRenderbuffers(1, &rbo);
      glBindRenderbuffer(GL_RENDERBUFFER, rbo);

      glRenderbufferStorage(GL_RENDERBUFFER,
                            gl_framebuffer_format_internal[depth_attachment_format],
                            width,
                            height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                GL_DEPTH_STENCIL_ATTACHMENT,
                                GL_RENDERBUFFER,
                                rbo);
   }

   GLenum complete_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if (complete_status != GL_FRAMEBUFFER_COMPLETE) {
      assert(false);
   }

   width_ = width;
   height_ = height;
   id_ = id;
   depth_attachment_ = rbo;
   for (int32 index = 0; index < framebuffer::ATTACHMENT_LIMIT; index++) {
      color_attachments_[index] = textures[index];
   }

   GLenum attachment_indices[framebuffer::ATTACHMENT_LIMIT] = {
      GL_COLOR_ATTACHMENT0 + 0,
      GL_COLOR_ATTACHMENT0 + 1,
      GL_COLOR_ATTACHMENT0 + 2,
      GL_COLOR_ATTACHMENT0 + 3,
   };
   glDrawBuffers(color_attachment_count, attachment_indices);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   return is_valid();
}

void framebuffer::destroy()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glDeleteFramebuffers(1, &id_);
   if (depth_attachment_) {
      glDeleteRenderbuffers(1, &depth_attachment_);
   }

   for (uint32 index = 0; index < framebuffer::ATTACHMENT_LIMIT; index++) {
      if (color_attachments_[index]) {
         glDeleteTextures(1, color_attachments_ + index);
         color_attachments_[index] = 0;
      }
   }

   id_ = 0;
   width_ = 0;
   height_ = 0;
   depth_attachment_ = 0;
}

texture framebuffer::color_attachment_as_texture(const int32 index) const
{
   assert(index >= 0 && index < framebuffer::ATTACHMENT_LIMIT);
   texture result;
   result.id_ = color_attachments_[index];
   return result;
}

vertex_layout::vertex_layout()
   : stride_(0)
   , attribute_count_(0)
   , attributes_{}
{
}

void vertex_layout::add_attribute(const int32 index,
                                  attribute_format format,
                                  const int32 count,
                                  const bool normalized)
{
   assert(attribute_count_ < array_size(attributes_));

   const int32 at = attribute_count_++;
   attributes_[at].command_index_ = index;
   attributes_[at].format_ = format;
   attributes_[at].count_ = count;
   attributes_[at].offset_ = stride_;
   attributes_[at].normalized_ = normalized;

   stride_ += count * gl_attribute_size[format];
}

void vertex_layout::clear()
{
   stride_ = 0;
   attribute_count_ = 0;
   for (int32 index = 0; index < 8; index++) {
      attributes_[index] = {};
   }
}

// note: opengl core context _requires_ a vertex array object to be bound
//       so let's please the opengl gods
static GLuint g_vertex_array_object = 0;

render_backend::render_backend()
{
   if (g_vertex_array_object == 0) {
      glGenVertexArrays(1, &g_vertex_array_object);
      glBindVertexArray(g_vertex_array_object);
   }
}

render_backend::~render_backend()
{
   if (g_vertex_array_object) {
      glBindVertexArray(0);
      glDeleteVertexArrays(1, &g_vertex_array_object);
      g_vertex_array_object = 0;
   }
}

void render_backend::clear(const float red,
                           const float green,
                           const float blue,
                           const float alpha,
                           const float depth)
{
   glDepthMask(GL_TRUE);
   glClearDepth(depth);
   glClearColor(red, green, blue, alpha);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render_backend::set_viewport(const int32 x,
                                  const int32 y,
                                  const int32 width,
                                  const int32 height)
{
   glViewport(x, y, width, height);
}

void render_backend::set_framebuffer(const framebuffer &handle)
{
   glBindFramebuffer(GL_FRAMEBUFFER, handle.id_);
   set_viewport(0, 0, handle.width_, handle.height_);
}

void render_backend::reset_framebuffer()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render_backend::set_shader_program(const shader_program &handle)
{
   glUseProgram(handle.id_);
}

void render_backend::set_shader_uniform(const shader_program &handle,
                                        const uniform_type type,
                                        const char *name,
                                        const int32 count,
                                        const void *value)
{
   GLint location = glGetUniformLocation(handle.id_, name);
   if (location == -1)
      return;

   switch (type) {
      case UNIFORM_TYPE_FLOAT:
      {
         glUniform1fv(location, count, (const GLfloat *)value);
      } break;
      case UNIFORM_TYPE_VEC2:
      {
         glUniform2fv(location, count, (const GLfloat *)value);
      } break;
      case UNIFORM_TYPE_VEC3:
      {
         glUniform3fv(location, count, (const GLfloat *)value);
      } break;
      case UNIFORM_TYPE_VEC4:
      {
         glUniform4fv(location, count, (const GLfloat *)value);
      } break;
      case UNIFORM_TYPE_INT:
      case UNIFORM_TYPE_BOOL:
      case UNIFORM_TYPE_SAMPLER:
      {
         glUniform1iv(location, count, (const GLint *)value);
      } break;
      case UNIFORM_TYPE_MATRIX:
      {
         glUniformMatrix4fv(location, count, GL_FALSE, (const GLfloat *)value);
      } break;
   }
}

void render_backend::set_index_buffer(const index_buffer &handle)
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.id_);
}

void render_backend::set_vertex_buffer(const vertex_buffer &handle)
{
   glBindBuffer(GL_ARRAY_BUFFER, handle.id_);
}

void render_backend::set_vertex_layout(const vertex_layout &layout)
{
   // note: disable all attributes
   for (int32 index = 0; index < array_size(layout.attributes_); index++) {
      glDisableVertexAttribArray(index);
   }

   const int32 vertex_stride = layout.stride_;
   const int32 attribute_count = layout.attribute_count_;
   for (int32 attribute_index = 0;
        attribute_index < attribute_count;
        attribute_index++)
   {
      const auto &attribute = layout.attributes_[attribute_index];
      glEnableVertexAttribArray(attribute.command_index_);
      glVertexAttribPointer(attribute.command_index_,
                            attribute.count_,
                            gl_attribute_type[attribute.format_],
                            attribute.normalized_,
                            vertex_stride,
                            (const void *)(uintptr_t)attribute.offset_);
   }
}

void render_backend::set_texture(const texture &handle,
                                 const int32 unit)
{
   assert(unit >= 0 && unit < 4);
   const GLenum units[] =
   {
      GL_TEXTURE0,
      GL_TEXTURE1,
      GL_TEXTURE2,
      GL_TEXTURE3,
   };

   glActiveTexture(units[unit]);
   glBindTexture(GL_TEXTURE_2D, handle.id_);
}

void render_backend::set_cubemap(const cubemap &handle,
                                 const int32 unit)
{
   glActiveTexture(GL_TEXTURE0 + unit);
   glBindTexture(GL_TEXTURE_CUBE_MAP, handle.id_);
}

void render_backend::set_sampler_state(const sampler_state &handle,
                                       const int32 unit)
{
   glBindSampler(unit, handle.id_);
}

void render_backend::set_blend_state(const bool enabled,
                                     const blend_equation eq_rgb,
                                     const blend_factor src_rgb,
                                     const blend_factor dst_rgb,
                                     const blend_equation eq_alpha,
                                     const blend_factor src_alpha,
                                     const blend_factor dst_alpha)
{
   if (enabled) {
      glEnable(GL_BLEND);
      glBlendFuncSeparate(gl_blend_ft[src_rgb],
                          gl_blend_ft[dst_rgb],
                          gl_blend_ft[src_alpha],
                          gl_blend_ft[dst_alpha]);
      glBlendEquationSeparate(gl_blend_eq[eq_rgb],
                              gl_blend_eq[eq_alpha]);
   }
   else {
      glDisable(GL_BLEND);
   }
}

void render_backend::set_depth_state(const bool testing,
                                     const bool write,
                                     const float range_near,
                                     const float range_far,
                                     const compare_func func)
{
   if (testing) {
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(gl_compare_func[func]);

      if (write)
         glDepthMask(GL_TRUE);
      else
         glDepthMask(GL_FALSE);
   }
   else {
      glDisable(GL_DEPTH_TEST);
   }

   glDepthRange(range_near, range_far);
}

void render_backend::set_rasterizer_state(const cull_mode cull_mode,
                                          const front_face front_face,
                                          const polygon_mode polygon)
{
   if (cull_mode != CULL_MODE_NONE) {
      glEnable(GL_CULL_FACE);
      glCullFace(gl_cull_mode[cull_mode]);
   }
   else {
      glDisable(GL_CULL_FACE);
   }

   glFrontFace(gl_front_face[front_face]);
   glPolygonMode(GL_FRONT_AND_BACK, polygon == POLYGON_MODE_FILL ? GL_FILL : GL_LINE);
}

void render_backend::draw(const primitive_topology topology,
                          const int32 start_index,
                          const int32 primitive_count)
{
   glDrawArrays(gl_primitive_topology[topology],
                start_index,
                primitive_count);
}

void render_backend::draw_indexed(const primitive_topology topology,
                                  const index_type type,
                                  const int32 start_index,
                                  const int32 primitive_count)
{
   // todo: glDrawElementsBaseVertex
   glDrawElements(gl_primitive_topology[topology],
                  primitive_count,
                  gl_index_type[type],
                  (const void *)(uintptr_t)(gl_index_size[type] * start_index));
}
