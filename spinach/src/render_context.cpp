// render_context.cpp

#include "spinach.hpp"
#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

static void 
key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
   application *app = (application *)glfwGetWindowUserPointer(window);
   if (app == nullptr) {
      return;
   }

   app->on_key(key, action != GLFW_RELEASE);
}

static void 
mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
   application *app = (application *)glfwGetWindowUserPointer(window);
   if (app == nullptr) {
      return;
   }

   app->on_mouse(int(xpos), int(ypos));
}

static void 
button_callback(GLFWwindow *window, int button, int action, int mods)
{
   application *app = (application *)glfwGetWindowUserPointer(window);
   if (app == nullptr) {
      return;
   }

   app->on_button(button, action == GLFW_PRESS);
}

render_context::render_context(const char *title, int width, int height, void *userdata)
   : m_window(nullptr)
{
   glfwSetErrorCallback([](int code, const char *message) {
      printf("!!! %s (%d)\n", message, code);
   });

   if (!glfwInit()) {
      return;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
   GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
   if (window == nullptr) {
      return;
   }

   glfwMakeContextCurrent(window);
   glfwSwapInterval(1);
   if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
      glfwDestroyWindow(window);
      return;
   }

   glfwSetKeyCallback(window, key_callback);
   glfwSetCursorPosCallback(window, mouse_callback);
   glfwSetMouseButtonCallback(window, button_callback);
   glfwSetWindowUserPointer(window, userdata);

   m_window = window;
}

render_context::~render_context()
{
   glfwDestroyWindow(m_window);
   glfwTerminate();
}

bool render_context::valid() const
{
   return m_window != nullptr;
}

bool render_context::poll_events()
{
   if (!valid()) {
      return false;
   }

   glfwPollEvents();
   return !glfwWindowShouldClose(m_window);
}

void render_context::swap_buffers()
{
   if (!valid()) {
      return;
   }

   glfwSwapBuffers(m_window);
}
