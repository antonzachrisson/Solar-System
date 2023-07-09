// applicaiton.cpp

#include "spinach.hpp"

#include <GLFW/glfw3.h> // keycodes, ...
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct vertex2d
{
   float x, y;
   float u, v;
};

struct vertex3d
{
   float x, y, z;
   float u, v;
};

application::application(const char* title, int width, int height)
    : m_width(width)
    , m_height(height)
    , m_context(title, width, height, this)
    , m_camera(glm::perspective(3.1415926f * 0.25f, float(width) / float(height), 1.0f, 1000.0f))
    , m_controller(m_camera)
    , m_overlay(&m_program_font, &m_texture_font, &m_sampler_nearest, &m_layout_2d)
    , m_sun(&m_layout_3d)
    , m_mercury(&m_layout_3d)
    , m_venus(&m_layout_3d)
    , m_earth(&m_layout_3d)
    , m_moon(&m_layout_3d)
    , m_mars(&m_layout_3d)
    , m_jupiter(&m_layout_3d)
    , m_saturn(&m_layout_3d)
    , m_uranus(&m_layout_3d)
    , m_neptune(&m_layout_3d)
    , m_sun_position(0.0f, 0.0f, 0.0f)
    , m_mercury_position(0.0f, 0.0f, -20.0f)
    , m_venus_position(0.0f, 0.0f, -30.0f)
    , m_earth_position(0.0f, 0.0f, -40.0f)
    , m_moon_position(0.0f, 0.0f, -48.0f)
    , m_mars_position(0.0f, 0.0f, -53.0f)
    , m_jupiter_position(0.0f, 0.0f, -65.0f)
    , m_saturn_position(0.0f, 0.0f, -75.0f)
    , m_uranus_position(0.0f, 0.0f, -85.0f)
    , m_neptune_position(0.0f, 0.0f, -95.0f)
{
    moonAngle = atan2f(m_moon_position.z - m_earth_position.z, m_moon_position.x - m_earth_position.x);
    moonRadius = 8.0f;
}

glm::vec3 application::orbit(glm::vec3 thisPlanet, glm::vec3 otherPlanet, float rotSpeed, const time& dt)
{
    float angle = atan2f(thisPlanet.z, thisPlanet.x) - atan2f(otherPlanet.z, otherPlanet.x);
    angle += dt.as_seconds() * rotSpeed;

    if (angle > 2 * atanf(1.0f) * 4.0f)
        angle -= 2 * atanf(1.0f) * 4.0f;

    if (angle < 0)
        angle += 2 * atanf(1.0f) * 4.0f;

    float radius = glm::distance(thisPlanet, otherPlanet);
    
    float x = otherPlanet.x + cosf(angle) * radius;
    float z = otherPlanet.z + sinf(angle) * radius;

    return glm::vec3(x, thisPlanet.y, z);
}

glm::vec3 application::orbit(glm::vec3 thisPlanet, glm::vec3 otherPlanet, float rotSpeed, const time& dt, float radius, float &angle)
{
    angle += dt.as_seconds() * rotSpeed;

    if (angle > 2 * atanf(1.0f) * 4.0f)
        angle -= 2 * atanf(1.0f) * 4.0f;

    if (angle < 0)
        angle += 2 * atanf(1.0f) * 4.0f;

    float x = otherPlanet.x + cosf(angle) * radius;
    float z = otherPlanet.z + sinf(angle) * radius;

    return glm::vec3(x, thisPlanet.y, z);
}

void application::run()
{
   m_running = m_context.valid();
   if (!m_running) {
      debug::log("could not initialize context!");
      return;
   }

   if (!create_resources()) {
      return;
   }

   while (m_running && m_context.poll_events()) {
      tick(time::deltatime());
      draw();

      post_frame();
   }
}

void application::on_key(int key, bool state)
{
   m_keyboard.on_key(key, state);
}

void application::on_mouse(int x, int y)
{
   m_mouse.on_move(x, y);
}

void application::on_button(int button, bool state)
{
   m_mouse.on_button(button, state);
}

void application::tick(const time &dt)
{
   if (m_keyboard.key_released(GLFW_KEY_ESCAPE)) {
      m_running = false;
   }

   m_controller.update(m_keyboard, m_mouse, dt);

   m_cube_rotation += dt.as_seconds();

   m_mercury_position = orbit(m_mercury_position, m_sun_position, 1.0f, dt);

   m_venus_position = orbit(m_venus_position, m_sun_position, 1.2f, dt);

   m_earth_position = orbit(m_earth_position, m_sun_position, 1.1f, dt);

   m_moon_position = orbit(m_moon_position, m_earth_position, 4.0f, dt, moonRadius, moonAngle);

   m_mars_position = orbit(m_mars_position, m_sun_position, 1.5f, dt);

   m_jupiter_position = orbit(m_jupiter_position, m_sun_position, 0.7f, dt);

   m_saturn_position = orbit(m_saturn_position, m_sun_position, 0.9f, dt);

   m_uranus_position = orbit(m_uranus_position, m_sun_position, 1.8f, dt);

   m_neptune_position = orbit(m_neptune_position, m_sun_position, 1.3f, dt);


   glm::mat4 sun = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f)), m_sun_position)
       * glm::rotate(glm::mat4(1.0f), m_cube_rotation, glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)));

   glm::mat4 mercury = glm::translate(glm::mat4(1.0f), m_mercury_position)
       * glm::rotate(glm::mat4(1.0f), m_cube_rotation, glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)));

   glm::mat4 venus = glm::translate(glm::mat4(1.0f), m_venus_position)
       * glm::rotate(glm::mat4(1.0f), m_cube_rotation, glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)));

   glm::mat4 earth = glm::translate(glm::mat4(1.0f), m_earth_position)
       * glm::rotate(glm::mat4(1.0f), m_cube_rotation, glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)));

   glm::mat4 moon = glm::translate(glm::mat4(1.0f), m_moon_position)
       * glm::rotate(glm::mat4(1.0f), m_cube_rotation, glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)));

   glm::mat4 mars = glm::translate(glm::mat4(1.0f), m_mars_position)
       * glm::rotate(glm::mat4(1.0f), m_cube_rotation, glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)));

   glm::mat4 jupiter = glm::translate(glm::mat4(1.0f), m_jupiter_position)
       * glm::rotate(glm::mat4(1.0f), m_cube_rotation, glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)));

   glm::mat4 saturn = glm::translate(glm::mat4(1.0f), m_saturn_position)
       * glm::rotate(glm::mat4(1.0f), m_cube_rotation, glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)));

   glm::mat4 uranus = glm::translate(glm::mat4(1.0f), m_uranus_position)
       * glm::rotate(glm::mat4(1.0f), m_cube_rotation, glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)));

   glm::mat4 neptune = glm::translate(glm::mat4(1.0f), m_neptune_position)
       * glm::rotate(glm::mat4(1.0f), m_cube_rotation, glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)));


   m_sun.set_transform(sun);
   m_mercury.set_transform(mercury);
   m_venus.set_transform(venus);
   m_earth.set_transform(earth);
   m_moon.set_transform(moon);
   m_mars.set_transform(mars);
   m_jupiter.set_transform(jupiter);
   m_saturn.set_transform(saturn);
   m_uranus.set_transform(uranus);
   m_neptune.set_transform(neptune);

   const int frames_per_second = int(1.0f / dt.as_seconds());
   const int frame_timing_ms = int(dt.as_milliseconds());

   m_overlay.pre_frame(m_width, m_height);
   m_overlay.push_line("FPS: %d (%dms)", frames_per_second, frame_timing_ms);
}

void application::draw()
{
   draw_world_render_pass();
   draw_framebuffer_render_pass();
   draw_debug_text_render_pass();
   m_context.swap_buffers();
}

bool application::create_resources()
{
   if (!create_framebuffers()) {
      debug::log("could not create framebuffers!");
      return false;
   }
   if (!create_shaders()) {
      debug::log("could not create shader programs!");
      return false;
   }
   if (!create_textures()) {
      debug::log("could not create textures!");
      return false;
   }
   if (!create_samplers()) {
      debug::log("could not create sampler states!");
      return false;
   }
   if (!create_buffers()) {
      debug::log("could not create vertex buffers!");
      return false;
   }
   if (!create_layouts()) {
      debug::log("could not create vertex layouts!");
      return false;
   }
   if (!create_skybox()) {
      debug::log("could not create skybox!");
      return false;
   }
   if (!create_models()) {
      debug::log("could not create models!");
      return false;
   }
   if (!create_misc()) {
      debug::log("could not create misc resources!");
      return false;
   }

   return true;
}

bool application::create_framebuffers()
{
   const framebuffer_format formats[] = { FRAMEBUFFER_FORMAT_RGBA8, FRAMEBUFFER_FORMAT_D32 };
   if (!m_rendertarget.create(320, 180, 2, formats)) {
      return false;
   }

   return true;
}

bool application::create_shaders()
{
   if (!utility::create_shader_program_from_files(m_program_world, "data/world.vs.glsl", "data/world.fs.glsl")) {
      return false;
   }

   if (!utility::create_shader_program_from_files(m_program_final, "data/final.vs.glsl", "data/final.fs.glsl")) {
      return false;
   }

   if (!utility::create_shader_program_from_files(m_program_font, "data/font.vs.glsl", "data/font.fs.glsl")) {
      return false;
   }

   return true;
}

bool application::create_textures()
{
   if (!utility::create_texture_from_file(m_texture_font, "data/font8x8.png")) {
      return false;
   }

   if (!utility::create_texture_from_file(m_texture_sun, "data/sun.png")) {
       return false;
   }

   if (!utility::create_texture_from_file(m_texture_mercury, "data/mercury.png")) {
       return false;
   }

   if (!utility::create_texture_from_file(m_texture_venus, "data/venus.png")) {
       return false;
   }

   if (!utility::create_texture_from_file(m_texture_earth, "data/earth.png")) {
       return false;
   }

   if (!utility::create_texture_from_file(m_texture_moon, "data/moon.png")) {
       return false;
   }

   if (!utility::create_texture_from_file(m_texture_mars, "data/mars.png")) {
       return false;
   }

   if (!utility::create_texture_from_file(m_texture_jupiter, "data/jupiter.png")) {
       return false;
   }

   if (!utility::create_texture_from_file(m_texture_saturn, "data/saturn.png")) {
       return false;
   }

   if (!utility::create_texture_from_file(m_texture_uranus, "data/uranus.png")) {
       return false;
   }

   if (!utility::create_texture_from_file(m_texture_neptune, "data/neptune.png")) {
       return false;
   }

   return true;
}

bool application::create_samplers()
{
   if (!m_sampler_nearest.create(SAMPLER_FILTER_MODE_NEAREST, SAMPLER_ADDRESS_MODE_CLAMP, SAMPLER_ADDRESS_MODE_CLAMP)) {
      return false;
   }

   if (!m_sampler_linear.create(SAMPLER_FILTER_MODE_LINEAR, SAMPLER_ADDRESS_MODE_CLAMP, SAMPLER_ADDRESS_MODE_CLAMP)) {
      return false;
   }

   return true;
}

bool application::create_buffers()
{
   const vertex2d screen_quad_data[] =
   {
      { -1.0f,  1.0f,          0.0f, 1.0f },
      {  1.0f,  1.0f,          1.0f, 1.0f },
      {  1.0f, -1.0f,          1.0f, 0.0f },
      {  1.0f, -1.0f,          1.0f, 0.0f },
      { -1.0f, -1.0f,          0.0f, 0.0f },
      { -1.0f,  1.0f,          0.0f, 1.0f },
   };

   if (!m_buffer_screen_quad.create(sizeof(screen_quad_data), screen_quad_data)) {
      return false;
   }

   return true;
}

bool application::create_layouts()
{
   m_layout_3d.add_attribute(0, vertex_layout::ATTRIBUTE_FORMAT_FLOAT, 3, false);
   m_layout_3d.add_attribute(1, vertex_layout::ATTRIBUTE_FORMAT_FLOAT, 2, false);

   m_layout_2d.add_attribute(0, vertex_layout::ATTRIBUTE_FORMAT_FLOAT, 2, false);
   m_layout_2d.add_attribute(1, vertex_layout::ATTRIBUTE_FORMAT_FLOAT, 2, false);

   return true;
}

bool application::create_skybox()
{
   if (!m_skybox.create("data/skybox")) {
      return false;
   }

   return true;
}

bool application::create_models()
{
   const vertex3d cube_data[] =
   {
      // front
      { -1.0f,  1.0f,  1.0f,   0.0f, 1.0f, },
      {  1.0f,  1.0f,  1.0f,   1.0f, 1.0f, },
      {  1.0f, -1.0f,  1.0f,   1.0f, 0.0f, },
      {  1.0f, -1.0f,  1.0f,   1.0f, 0.0f, },
      { -1.0f, -1.0f,  1.0f,   0.0f, 0.0f, },
      { -1.0f,  1.0f,  1.0f,   0.0f, 1.0f, },

      // right                             
      {  1.0f,  1.0f,  1.0f,   0.0f, 1.0f, },
      {  1.0f,  1.0f, -1.0f,   1.0f, 1.0f, },
      {  1.0f, -1.0f, -1.0f,   1.0f, 0.0f, },
      {  1.0f, -1.0f, -1.0f,   1.0f, 0.0f, },
      {  1.0f, -1.0f,  1.0f,   0.0f, 0.0f, },
      {  1.0f,  1.0f,  1.0f,   0.0f, 1.0f, },

      // back                              
      {  1.0f,  1.0f, -1.0f,   0.0f, 1.0f, },
      { -1.0f,  1.0f, -1.0f,   1.0f, 1.0f, },
      { -1.0f, -1.0f, -1.0f,   1.0f, 0.0f, },
      { -1.0f, -1.0f, -1.0f,   1.0f, 0.0f, },
      {  1.0f, -1.0f, -1.0f,   0.0f, 0.0f, },
      {  1.0f,  1.0f, -1.0f,   0.0f, 1.0f, },

      // left                              
      { -1.0f,  1.0f, -1.0f,   0.0f, 1.0f, },
      { -1.0f,  1.0f,  1.0f,   1.0f, 1.0f, },
      { -1.0f, -1.0f,  1.0f,   1.0f, 0.0f, },
      { -1.0f, -1.0f,  1.0f,   1.0f, 0.0f, },
      { -1.0f, -1.0f, -1.0f,   0.0f, 0.0f, },
      { -1.0f,  1.0f, -1.0f,   0.0f, 1.0f, },

      // top                               
      {  1.0f,  1.0f,  1.0f,   0.0f, 1.0f, },
      { -1.0f,  1.0f,  1.0f,   1.0f, 1.0f, },
      { -1.0f,  1.0f, -1.0f,   1.0f, 0.0f, },
      { -1.0f,  1.0f, -1.0f,   1.0f, 0.0f, },
      {  1.0f,  1.0f, -1.0f,   0.0f, 0.0f, },
      {  1.0f,  1.0f,  1.0f,   0.0f, 1.0f, },

      // bottom
      { -1.0f, -1.0f,  1.0f,   0.0f, 1.0f, },
      {  1.0f, -1.0f,  1.0f,   1.0f, 1.0f, },
      {  1.0f, -1.0f, -1.0f,   1.0f, 0.0f, },
      {  1.0f, -1.0f, -1.0f,   1.0f, 0.0f, },
      { -1.0f, -1.0f, -1.0f,   0.0f, 0.0f, },
      { -1.0f, -1.0f,  1.0f,   0.0f, 1.0f, },
   };

   m_sun.set_transform(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f)));
   m_sun.m_material.set_shader_program(&m_program_world);
   m_sun.m_material.set_texture(&m_texture_sun);
   m_sun.m_material.set_sampler_state(&m_sampler_linear);
   if (!m_sun.create(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sizeof(vertex3d), sizeof(cube_data) / sizeof(cube_data[0]), cube_data)) {
       return false;
   }

   m_mercury.set_transform(glm::mat4(1.0f));
   m_mercury.m_material.set_shader_program(&m_program_world);
   m_mercury.m_material.set_texture(&m_texture_mercury);
   m_mercury.m_material.set_sampler_state(&m_sampler_linear);
   if (!m_mercury.create(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sizeof(vertex3d), sizeof(cube_data) / sizeof(cube_data[0]), cube_data)) {
       return false;
   }

   m_venus.set_transform(glm::mat4(1.0f));
   m_venus.m_material.set_shader_program(&m_program_world);
   m_venus.m_material.set_texture(&m_texture_venus);
   m_venus.m_material.set_sampler_state(&m_sampler_linear);
   if (!m_venus.create(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sizeof(vertex3d), sizeof(cube_data) / sizeof(cube_data[0]), cube_data)) {
       return false;
   }

   m_earth.set_transform(glm::mat4(1.0f));
   m_earth.m_material.set_shader_program(&m_program_world);
   m_earth.m_material.set_texture(&m_texture_earth);
   m_earth.m_material.set_sampler_state(&m_sampler_linear);
   if (!m_earth.create(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sizeof(vertex3d), sizeof(cube_data) / sizeof(cube_data[0]), cube_data)) {
       return false;
   }

   m_moon.set_transform(glm::mat4(1.0f));
   m_moon.m_material.set_shader_program(&m_program_world);
   m_moon.m_material.set_texture(&m_texture_moon);
   m_moon.m_material.set_sampler_state(&m_sampler_linear);
   if (!m_moon.create(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sizeof(vertex3d), sizeof(cube_data) / sizeof(cube_data[0]), cube_data)) {
       return false;
   }

   m_mars.set_transform(glm::mat4(1.0f));
   m_mars.m_material.set_shader_program(&m_program_world);
   m_mars.m_material.set_texture(&m_texture_mars);
   m_mars.m_material.set_sampler_state(&m_sampler_linear);
   if (!m_mars.create(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sizeof(vertex3d), sizeof(cube_data) / sizeof(cube_data[0]), cube_data)) {
       return false;
   }

   m_jupiter.set_transform(glm::mat4(1.0f));
   m_jupiter.m_material.set_shader_program(&m_program_world);
   m_jupiter.m_material.set_texture(&m_texture_jupiter);
   m_jupiter.m_material.set_sampler_state(&m_sampler_linear);
   if (!m_jupiter.create(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sizeof(vertex3d), sizeof(cube_data) / sizeof(cube_data[0]), cube_data)) {
       return false;
   }

   m_saturn.set_transform(glm::mat4(1.0f));
   m_saturn.m_material.set_shader_program(&m_program_world);
   m_saturn.m_material.set_texture(&m_texture_saturn);
   m_saturn.m_material.set_sampler_state(&m_sampler_linear);
   if (!m_saturn.create(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sizeof(vertex3d), sizeof(cube_data) / sizeof(cube_data[0]), cube_data)) {
       return false;
   }


   m_uranus.set_transform(glm::mat4(1.0f));
   m_uranus.m_material.set_shader_program(&m_program_world);
   m_uranus.m_material.set_texture(&m_texture_uranus);
   m_uranus.m_material.set_sampler_state(&m_sampler_linear);
   if (!m_uranus.create(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sizeof(vertex3d), sizeof(cube_data) / sizeof(cube_data[0]), cube_data)) {
       return false;
   }

   m_neptune.set_transform(glm::mat4(1.0f));
   m_neptune.m_material.set_shader_program(&m_program_world);
   m_neptune.m_material.set_texture(&m_texture_neptune);
   m_neptune.m_material.set_sampler_state(&m_sampler_linear);
   if (!m_neptune.create(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sizeof(vertex3d), sizeof(cube_data) / sizeof(cube_data[0]), cube_data)) {
       return false;
   }

   // note: model list
   m_meshes.push_back(&m_sun);
   m_meshes.push_back(&m_mercury);
   m_meshes.push_back(&m_venus);
   m_meshes.push_back(&m_earth);
   m_meshes.push_back(&m_moon);
   m_meshes.push_back(&m_mars);
   m_meshes.push_back(&m_jupiter);
   m_meshes.push_back(&m_saturn);
   m_meshes.push_back(&m_uranus);
   m_meshes.push_back(&m_neptune);


   return true;
}

bool application::create_misc()
{
   return true;
}

void application::draw_world_render_pass()
{
   m_backend.set_framebuffer(m_rendertarget);
   m_backend.clear(0.0f, 0.0f, 0.0f, 1.0f);

   m_skybox.draw(m_backend, m_camera);

   m_camera.bind(m_backend, m_program_world);
   for (const auto &mesh : m_meshes) {
      mesh->draw(m_backend);
   }
}

void application::draw_framebuffer_render_pass()
{
   auto screen_texture = m_rendertarget.color_attachment_as_texture(0);

   m_backend.reset_framebuffer();
   m_backend.clear(0.0f, 0.0f, 0.0f, 1.0f);
   m_backend.set_viewport(0, 0, m_width, m_height);
   m_backend.set_shader_program(m_program_final);
   m_backend.set_vertex_buffer(m_buffer_screen_quad);
   m_backend.set_vertex_layout(m_layout_2d);
   m_backend.set_texture(screen_texture);
   m_backend.set_sampler_state(m_sampler_nearest);
   m_backend.set_blend_state(false);
   m_backend.set_rasterizer_state(CULL_MODE_NONE, FRONT_FACE_CW);
   m_backend.draw(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, 6);
}

void application::draw_debug_text_render_pass()
{
   m_overlay.draw(m_backend);
}

void application::post_frame()
{
   m_mouse.update();
   m_keyboard.update();
}
