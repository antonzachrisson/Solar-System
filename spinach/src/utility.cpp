// utility.cpp

#include "spinach.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cstdio>
#include <string>
#include <chrono>

template <typename F>
struct defer {
   F m_f;
   defer(F &&f) : m_f(f) {}
   ~defer() { m_f(); }
};

namespace utility
{
   bool create_shader_program_from_files(shader_program &program,
                                         const char *vertex_filename,
                                         const char *fragment_filename)
   {
      std::size_t size = 0;
      FILE *fin = nullptr;

      fopen_s(&fin, vertex_filename, "r");
      if (fin == nullptr) {
         return false;
      }

      fseek(fin, 0, SEEK_END);
      size = ftell(fin);
      fseek(fin, 0, SEEK_SET);
      std::string vertex_source;
      vertex_source.resize(size + 1);
      fread(vertex_source.data(), 1, size, fin);
      fclose(fin);
      fin = nullptr;

      fopen_s(&fin, fragment_filename, "r");
      if (fin == nullptr) {
         return false;
      }

      fseek(fin, 0, SEEK_END);
      size = ftell(fin);
      fseek(fin, 0, SEEK_SET);
      std::string fragment_source;
      fragment_source.resize(size + 1);
      fread(fragment_source.data(), 1, size, fin);
      fclose(fin);
      fin = nullptr;

      return program.create(vertex_source.c_str(), fragment_source.c_str());
   }

   bool create_texture_from_file(texture &texture,
                                 const char *filename)
   {
      int width = 0, height = 0, components = 0;
      auto data = stbi_load(filename, &width, &height, &components, STBI_default);
      assert(data);
      assert(components == 3 || components == 4);

      defer release([&]() {
         stbi_image_free(data);
      });

      texture_format format = components == 3 ? TEXTURE_FORMAT_RGB8 : TEXTURE_FORMAT_RGBA8;

      return texture.create(format, width, height, data);
   }

   bool create_cubemap_from_files(cubemap &cubemap,
                                  const int count,
                                  const char **filenames)
   {
      assert(count == 6);

      int width = 0;
      int height = 0;
      int components = 0;
      texture_format format = TEXTURE_FORMAT_UNKNOWN;
      const void *data[6] = {};

      // note: load image data
      for (int index = 0; index < count; index++) {
         const char *filename = filenames[index];
         int w = 0, h = 0, c = 0;
         auto d = stbi_load(filename, &w, &h, &c, STBI_default);
         assert(d);
         assert(c == 3 || c == 4);

         texture_format f = c == 3 ? TEXTURE_FORMAT_RGB8 : TEXTURE_FORMAT_RGBA8;
         if (width == 0) {
            width = w;
         }
         if (width != w) {
            assert(false);
         }
         
         if (height == 0) {
            height = h;
         }
         if (height != h) {
            assert(false);
         }

         if (format == TEXTURE_FORMAT_UNKNOWN) {
            format = f;
         }
         if (format != f) {
            assert(false);
         }

         data[index] = d;
      }

      // note: create cubemap
      bool success = cubemap.create(format, width, height, data);

      // note: release image data
      for (int index = 0; index < 6; index++) {
         if (data[index]) {
            stbi_image_free((void *)data[index]);
         }
      }

      return success;
   }

   int64 get_current_tick()
   {
      static int64 start = 0;
      if (start == 0) {
         auto ns = std::chrono::high_resolution_clock::now();
         auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(ns);
         start = ms.time_since_epoch().count();
      }

      auto ns = std::chrono::high_resolution_clock::now();
      auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(ns);
      auto now = ms.time_since_epoch().count();
      return now - start;
   }
} // !utility
