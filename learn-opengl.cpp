// $ clang++ -o # @ -std=c++17 -stdlib=libc++ -lglfw -framework CoreVideo
// -framework OpenGL -framework IOKit -framework Cocoa -framework Carbon

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
using std::cout, std::endl;

constexpr unsigned WIDTH = 800;
constexpr unsigned HEIGHT = 600;

int main() {
  // init and configure
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // create window
  GLFWwindow *window =
      glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", NULL, NULL);
  if (!window) {
    cout << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
   cout << "Failed to initialize GLAD\n";
    return -1;
  }

  return 0;
}
