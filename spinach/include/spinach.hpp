// spinach.hpp

#pragma once

#include <string>
#include <vector>
#include <render.hpp>
#include <glm/glm.hpp> // vecN,matN,quat

namespace utility
{
   // note: milliseconds since process start
   int64 get_current_tick();

   bool create_shader_program_from_files(shader_program &program,
                                         const char *vertex_filename,
                                         const char *fragment_filename);

   bool create_texture_from_file(texture &texture,
                                 const char *filename);

   bool create_cubemap_from_files(cubemap &cubemap,
                                  const int count,
                                  const char **filenames);
} // !utility

namespace debug
{
   void log(const char *format, ...);
} // !debug

struct time {
   static time now();
   static time deltatime();

   constexpr time() = default;
   constexpr time(const int64 duration)
      : m_duration(duration)
   {
   }

   time &operator+=(const time &rhs);
   time &operator-=(const time &rhs);
   time operator+(const time &rhs) const;
   time operator-(const time &rhs) const;

   float as_seconds() const;
   float as_milliseconds() const;

   int64 m_duration{};
};

struct mouse {
   mouse() = default;

   int x() const;
   int y() const;
   bool button_down(const int index) const;
   bool button_pressed(const int index) const;
   bool button_released(const int index) const;

   void update();
   void on_move(const int x, const int y);
   void on_button(const int index, bool state);

   int m_x{};
   int m_y{};
   struct {
      bool m_current;
      bool m_previous;
   } m_buttons[8]{};
};

struct keyboard {
   keyboard() = default;

   bool key_down(const int index) const;
   bool key_pressed(const int index) const;
   bool key_released(const int index) const;

   void update();
   void on_key(const int index, bool state);

   struct {
      bool m_current;
      bool m_previous;
   } m_keys[512]{};
};

struct camera {
   camera(const glm::mat4 &projection = glm::mat4(1.0f));

   void update();
   void move_x(const float amount);
   void move_y(const float amount);
   void move_z(const float amount);
   void rotate_x(const float amount);
   void rotate_y(const float amount);
   void rotate_z(const float amount);
   void set_projection(const glm::mat4 &projection);
   void bind(render_backend &backend, const shader_program &program);

   float m_pitch{};
   float m_yaw{};
   float m_roll{};
   glm::vec3 m_right;
   glm::vec3 m_up;
   glm::vec3 m_forward;
   glm::vec3 m_position;
   glm::mat4 m_view;
   glm::mat4 m_projection;
};

struct controller {
   controller(camera &camera);

   void update(const keyboard &keyboard, const mouse &mouse, const time &deltatime);
   void set_camera_speed(const float speed);
   void set_mouse_invert_yaw(const bool invert);
   void set_mouse_invert_pitch(const bool invert);
   void set_mouse_sensitivity(const float sensitivity);

   camera &m_camera;
   float m_speed;
   float m_yaw;
   float m_pitch;
   float m_sensitivity;
   glm::vec2 m_mouse_position;
};

struct skybox {
   skybox() = default;

   bool valid() const;
   bool create(const char *path);
   void destroy();

   void draw(render_backend &backend, const camera &camera);

   shader_program m_program;
   cubemap m_cubemap;
   sampler_state m_sampler;
   vertex_buffer m_buffer;
   vertex_layout m_layout;
   int32 m_primitive_count{};
};

struct material {
   struct parameter {
      parameter() = default;
      parameter(uniform_type type,
                std::string  name,
                const glm::vec4 &data);
      parameter(uniform_type type,
                std::string  name,
                const glm::mat4 &data);

      uniform_type m_type{};
      std::string  m_name;
      union {
         glm::vec4 m_v4;
         glm::mat4 m_m4;
      } m_data;
   };

   material() = default;
   material(const shader_program *program, const texture *texture, const sampler_state *sampler);

   void set_shader_program(const shader_program *program);
   void set_texture(const texture *texture);
   void set_sampler_state(const sampler_state *sampler);
   void set_parameter(const std::string &name, const glm::vec2 &value);
   void set_parameter(const std::string &name, const glm::vec3 &value);
   void set_parameter(const std::string &name, const glm::vec4 &value);
   void set_parameter(const std::string &name, const glm::mat4 &value);
   void bind(render_backend &backend);

   const shader_program *m_program{};
   const texture *m_texture{};
   const sampler_state *m_sampler{};
   std::vector<parameter> m_parameters;
};

struct mesh {
   static bool create_from_file(mesh &model, const char *filename);

   mesh(const vertex_layout *vertex_layout);

   bool valid() const;
   bool create(const primitive_topology topology, const int stride, const int count, const void *data);
   void update(const int stride, const int count, const void *data);
   void destroy();

   void set_transform(const glm::mat4 &transform);
   void draw(render_backend &backend);

   // note: we are cheating a bit, the model does not usually
   //       have a transform matrix. 
   //       something other holds this data, e.g. a node in a
   //       scene graph or something like that.
   glm::mat4 m_transform;

   material m_material;
   vertex_buffer m_buffer;
   const vertex_layout *m_layout{};
   primitive_topology m_topology{};
   int m_primitive_count{};
};

struct debug_overlay {
   debug_overlay(shader_program *program, 
                 texture *texture, 
                 sampler_state *sampler, 
                 vertex_layout *layout);

   void push_line(const char *format, ...);
   void pre_frame(const int width, const int height);
   void draw(render_backend &backend);

   glm::mat4 m_projection;
   material m_material;
   vertex_layout *m_layout{};
   vertex_buffer m_buffer;
   int m_vertex_count{};
   std::vector<std::string> m_lines;
};

struct GLFWwindow;
struct render_context {
   render_context(const char *title, int width, int height, void *userdata);
   ~render_context();

   bool valid() const;
   bool poll_events();
   void swap_buffers();

   GLFWwindow *m_window{};
};

struct application {
   application(const char *title, int width, int height);

   void run();

   void on_key(int key, bool state);
   void on_mouse(int x, int y);
   void on_button(int button, bool state);

private:
   void tick(const time &dt);
   void draw();

   bool create_resources();
   bool create_framebuffers();
   bool create_shaders();
   bool create_textures();
   bool create_samplers();
   bool create_buffers();
   bool create_layouts();
   bool create_skybox();
   bool create_models();
   bool create_misc();

   void draw_world_render_pass();
   void draw_framebuffer_render_pass();
   void draw_debug_text_render_pass();
   void post_frame();

   glm::vec3 orbit(glm::vec3 thisPlanet, glm::vec3 otherPlanet, float rotSpeed, const time& dt);
   glm::vec3 orbit(glm::vec3 thisPlanet, glm::vec3 otherPlanet, float rotSpeed, const time& dt, float radius, float &angle);
   
private:
   bool m_running{};
   int m_width{};
   int m_height{};
   mouse m_mouse;
   keyboard m_keyboard;
   render_context m_context;
   render_backend m_backend;

   framebuffer m_rendertarget;
   shader_program m_program_world;
   shader_program m_program_final;
   shader_program m_program_font;
   texture m_texture_font;
   texture m_texture_sun;
   texture m_texture_mercury;
   texture m_texture_venus;
   texture m_texture_earth;
   texture m_texture_moon;
   texture m_texture_mars;
   texture m_texture_jupiter;
   texture m_texture_saturn;
   texture m_texture_uranus;
   texture m_texture_neptune;
   sampler_state m_sampler_nearest;
   sampler_state m_sampler_linear;
   vertex_buffer m_buffer_screen_quad;
   vertex_layout m_layout_3d;
   vertex_layout m_layout_2d;

   debug_overlay m_overlay;
   camera m_camera;
   controller m_controller;
   skybox m_skybox;

   mesh m_sun;
   mesh m_mercury;
   mesh m_venus;
   mesh m_earth;
   mesh m_moon;
   mesh m_mars;
   mesh m_jupiter;
   mesh m_saturn;
   mesh m_uranus;
   mesh m_neptune;
   std::vector<mesh *> m_meshes;

   float m_cube_rotation{};
   glm::vec3 m_sun_position{};
   glm::vec3 m_mercury_position{};
   glm::vec3 m_venus_position{};
   glm::vec3 m_earth_position{};
   glm::vec3 m_moon_position{};
   glm::vec3 m_mars_position{};
   glm::vec3 m_jupiter_position{};
   glm::vec3 m_saturn_position{};
   glm::vec3 m_uranus_position{};
   glm::vec3 m_neptune_position{};

   float moonAngle;
   float moonRadius;
};
