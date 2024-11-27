#include <glad/glad.h>
// glad must be included before GLFW
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
using std::cout, std::endl;

constexpr unsigned SCR_WIDTH = 800;
constexpr unsigned SCR_HEIGHT = 800;

void processInput(GLFWwindow *window);

// container vertex shader
const char *containerVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
})";

const char *containerFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main() {
  FragColor = vec4(lightColor * objectColor, 1.0);
})";

// lightcube vertex shader
const char *lightcubeVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
})";

const char *lightcubeFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main() {
  FragColor = vec4(1.f, 1.f, 0.f, 1.0f);
})";

int main() {
  // bubu
  printf("💡🌟\n");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lighting", NULL, NULL);
  if (!window) {
    cout << "Failed to create GLFW window" << endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window,
                                 [](GLFWwindow *window, int width, int height) {
                                   glViewport(0, 0, width, height);
                                 });

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    cout << "Failed to initialize GLAD" << endl;
    return -1;
  }

  unsigned containerVertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(containerVertexShader, 1, &containerVertexShaderSource, NULL);
  glCompileShader(containerVertexShader);
  int success;
  char infoLog[512];
  glGetShaderiv(containerVertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(containerVertexShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
  }

  unsigned containerFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(containerFragmentShader, 1, &containerFragmentShaderSource,
                 NULL);
  glCompileShader(containerFragmentShader);
  glGetShaderiv(containerFragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(containerFragmentShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
  }

  unsigned containerShaderProgram = glCreateProgram();
  glAttachShader(containerShaderProgram, containerVertexShader);
  glAttachShader(containerShaderProgram, containerFragmentShader);
  glLinkProgram(containerShaderProgram);
  glGetProgramiv(containerShaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(containerShaderProgram, 512, NULL, infoLog);
    cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
  }

  glDeleteShader(containerVertexShader);
  glDeleteShader(containerFragmentShader);

  unsigned lightcubeVertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(lightcubeVertexShader, 1, &lightcubeVertexShaderSource, NULL);
  glCompileShader(lightcubeVertexShader);
  glGetShaderiv(lightcubeVertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(lightcubeVertexShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
  }

  unsigned lightcubeFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(lightcubeFragmentShader, 1, &lightcubeFragmentShaderSource,
                 NULL);
  glCompileShader(lightcubeFragmentShader);
  glGetShaderiv(lightcubeFragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(lightcubeFragmentShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
  }

  unsigned lightcubeShaderProgram = glCreateProgram();
  glAttachShader(lightcubeShaderProgram, lightcubeVertexShader);
  glAttachShader(lightcubeShaderProgram, lightcubeFragmentShader);
  glLinkProgram(lightcubeShaderProgram);
  glGetProgramiv(lightcubeShaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(lightcubeShaderProgram, 512, NULL, infoLog);
    cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
  }

  glDeleteShader(lightcubeVertexShader);
  glDeleteShader(lightcubeFragmentShader);

  // set up vertes attributes
  float vertices[] = {
      // positions
      -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f,
      0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,

      -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,
      0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,

      -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,
      -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,

      0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f,
      0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,

      -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,
      0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f,

      -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,
      0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f};

  unsigned VBO, containerVAO, lightcubeVAO;
  glGenVertexArrays(1, &containerVAO);
  glGenVertexArrays(1, &lightcubeVAO);
  glGenBuffers(1, &VBO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(containerVAO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  glBindVertexArray(lightcubeVAO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  glEnable(GL_DEPTH_TEST);

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f),
                        glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

    glm::mat4 projection =
        glm::perspective(glm::radians(45.0f),
                         (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    glUseProgram(containerShaderProgram);
    glUniform3f(glGetUniformLocation(containerShaderProgram, "objectColor"),
                1.0f, 0.5f, 0.31f);
    glUniform3f(glGetUniformLocation(containerShaderProgram, "lightColor"),
                1.0f, 1.0f, 1.0f);

    glUniformMatrix4fv(glGetUniformLocation(containerShaderProgram, "model"), 1,
                       GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(containerShaderProgram, "view"), 1,
                       GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(
        glGetUniformLocation(containerShaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));

    glBindVertexArray(containerVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glUseProgram(lightcubeShaderProgram);
    glm::vec3 lightPos = glm::vec3(1.2f, 1.0f, 0.0f);
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f));

    glUniformMatrix4fv(glGetUniformLocation(lightcubeShaderProgram, "model"), 1,
                       GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(lightcubeShaderProgram, "view"), 1,
                       GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(
        glGetUniformLocation(lightcubeShaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));

    glBindVertexArray(lightcubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &containerVAO);
  glDeleteVertexArrays(1, &lightcubeVAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(containerShaderProgram);
  glDeleteProgram(lightcubeShaderProgram);

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

// ligh 嘟好
