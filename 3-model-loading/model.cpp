#include <glad/glad.h>
// glad must be included before GLFW
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "model.h"

#include <filesystem>
#include <iostream>
#include <string>
using std::endl, std::string;
namespace fs = std::filesystem;

constexpr unsigned SCR_WIDTH = 800;
constexpr unsigned SCR_HEIGHT = 800;

glm::vec3 cameraPos = glm::vec3(0.f, 0.f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.f, 0.f, -2.0f);
glm::vec3 cameraUp = glm::vec3(0.f, 1.f, 0.f);

float deltaTime = 0.f;
float lastFrame = 0.f;

float LastX = SCR_WIDTH / 2;
float LastY = SCR_HEIGHT / 2;
bool firstMouse = true;

float yaw = -90.0f;
float pitch = 0.0f;
glm::vec3 direction;

float fov = 45.0f;

void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
unsigned loadTexture(const string &imagePath);

int main() {
  // bubu
  printf("🎒🎸\n");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Model Loading", NULL, NULL);
  if (!window) {
    cerr << "Failed to create GLFW window" << endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    cerr << "Failed to initialize GLAD" << endl;
    return -1;
  }

  stbi_set_flip_vertically_on_load(true);
  glEnable(GL_DEPTH_TEST);

  const std::string shaderPath = std::string(SUBPROJECT_SOURCE_DIR) + "/shaders";
  const std::string modelVertex = getPath(shaderPath + "/model_loading.vs");
  const std::string modelFragment = getPath(shaderPath + "/model_loading.fs");

  Shader modelShader(modelVertex, modelFragment);
  const string path = getPath(std::string(SUBPROJECT_SOURCE_DIR) + "/backpack/backpack.obj");

  Model ourModel = (getPath(std::string(SUBPROJECT_SOURCE_DIR) + "/backpack/backpack.obj"));


  glm::vec3 lightColor;
  lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
  glm::vec3 diffuseColor = lightColor * 0.5f;
  glm::vec3 ambientColor = diffuseColor * 0.2f;

  modelShader.use();
  modelShader.setFloat("spotLight.constant", 1.0f);
  modelShader.setFloat("spotLight.linear", 0.09f);
  modelShader.setFloat("spotLight.quadratic", 0.032f);

  modelShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(17.5f)));
  modelShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(22.5f)));

  modelShader.setVec3("spotLight.ambient", ambientColor);
  modelShader.setVec3("spotLight.diffuse", diffuseColor);
  modelShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);

  modelShader.setFloat("shinness", 32.0f);

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(
        glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    modelShader.use();
    modelShader.setMat4("view", view);
    modelShader.setMat4("projection", projection);

    modelShader.setVec3("viewPos", cameraPos);
    modelShader.setVec3("spotLight.direction", cameraFront);
    modelShader.setVec3("spotLight.position", cameraPos);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.f, 0.f, 0.f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    modelShader.setMat4("model", model);

    glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));
    modelShader.setMat4("normalMatrix", normalMatrix);

    ourModel.Draw(modelShader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  float cameraSpeed = deltaTime * 1.5f;
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  } else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    cameraPos += cameraSpeed * cameraFront;
  } else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    cameraPos -=
        cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
  } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    cameraPos -= cameraSpeed * cameraFront;
  } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    cameraPos +=
        cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
  } else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    cameraPos += cameraSpeed * cameraUp;
  } else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    cameraPos -= cameraSpeed * cameraUp;
  }
}

// light 嘟好

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    xpos = LastX;
    ypos = LastY;
    firstMouse = false;
  }

  float xOffset = xpos - LastX;
  float yOffset =
      -(ypos - LastY); // reversed since y-coordinate go from bottom to top
  LastX = xpos;
  LastY = ypos;

  float sensitivity = 0.01f;
  xOffset *= sensitivity;
  yOffset *= sensitivity;

  yaw += xOffset;
  pitch += yOffset;

  if (pitch > 89.0f) {
    pitch = 89.0f;
  } else if (pitch < -89.0f) {
    pitch = -89.0f;
  }

  direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.y = sin(glm::radians(pitch));
  direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(direction);
}

void scroll_callback(GLFWwindow *window, double xpos, double ypos) {
  fov -= ypos;
  if (fov < 1.0f) {
    fov = 1.0f;
  } else if (fov > 45.0f) {
    fov = 45.0f;
  }
}

unsigned loadTexture(const std::string &imagePath) {
  unsigned textureID;
  glGenTextures(1, &textureID);


  int width, height, nrChannels;
  unsigned char *data = stbi_load(imagePath.c_str(), &width, &height, &nrChannels, 0);

  if (data) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    // copy data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

  } else {
    cerr << "ERROR: Failed to load texture: " << imagePath << endl;
  }

  stbi_image_free(data);

  return textureID;
}
