#include <glad/glad.h>
// glad must be included before GLFW
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// shader class
#include "shader.h"

#include <filesystem>
#include <iostream>
#include <string>
using std::cout, std::endl, std::string;
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

std::string getPath(const std::string& path);

void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
unsigned loadTexture(const string &imagePath);

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
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    cout << "Failed to initialize GLAD" << endl;
    return -1;
  }

  const std::string shaderPath = std::string(SUBPROJECT_SOURCE_DIR) + "/shaders";
  const std::string lightingVertex = getPath(shaderPath + "/multiple_lights.vs");
  const std::string lightingFragment = getPath(shaderPath + "/multiple_lights.fs");

  const std::string lightCubeVertex = getPath(shaderPath + "/light_cube.vs");
  const std::string lightCubeFragment = getPath(shaderPath + "/light_cube.fs");

  Shader lighting(lightingVertex, lightingFragment);
  Shader lightCube(lightCubeVertex, lightCubeFragment);

  // set up vertes attributes
  float vertices[] = {
      // positions          // normals           // texture coords
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.5f,  -0.5f,
      -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,
      0.0f,  -1.0f, 1.0f,  1.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
      1.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,

      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.5f,  -0.5f,
      0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,
      0.0f,  1.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
      1.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,
      -0.5f, -1.0f, 0.0f,  0.0f,  1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f,
      0.0f,  0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
      0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f,  0.0f,
      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,

      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,
      -0.5f, 1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,
      0.0f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
      0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.5f,  -0.5f,
      -0.5f, 0.0f,  -1.0f, 0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,
      -1.0f, 0.0f,  1.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
      1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,
      -0.5f, 0.0f,  1.0f,  0.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,
      1.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f};

  unsigned VBO, containerVAO, lightcubeVAO;
  glGenVertexArrays(1, &containerVAO);
  glGenVertexArrays(1, &lightcubeVAO);
  glGenBuffers(1, &VBO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(containerVAO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glBindVertexArray(0);

  glBindVertexArray(lightcubeVAO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  glEnable(GL_DEPTH_TEST);

  // texture
  unsigned diffuseMap = loadTexture(getPath((std::string(PROJECT_SOURCE_DIR) + "/resources/container2.png")));
  unsigned specularMap = loadTexture(getPath((std::string(PROJECT_SOURCE_DIR) + "/resources/container2_specular.png")));

  const unsigned cubeNum = 10;
  glm::vec3 cubePositions[] = {
      glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
      glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
      glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};

  glm::vec3 pointLightPositions[] = {
      glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f),
      glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(0.0f, 0.0f, -3.0f)};

  glm::vec3 lightColor;
  lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
  // lightColor.x = sin(currentFrame * 2.0f);
  // lightColor.y = sin(currentFrame * 0.7f);
  // lightColor.z = sin(currentFrame * 1.3f);

  glm::vec3 diffuseColor = lightColor * 0.5f;
  glm::vec3 ambientColor = diffuseColor * 0.2f;

  glm::vec3 pinkLight = glm::vec3(0.8f, 0.5f, 0.2f);

  lighting.use();
  lighting.setInt("material.diffuse", 0);
  lighting.setInt("material.specular", 1);
  lighting.setFloat("material.shinness", 32.0f);

  for (int i = 0; i < 4; ++i) {
    std::string index = std::to_string(i); // Convert i to string properly
    std::string pointLight =
        "pointLight[" + index + "]"; // Construct the base string

    std::string pointLightPosition = pointLight + ".position";
    std::string pointLightConstant = pointLight + ".constant";
    std::string pointLightLinear = pointLight + ".linear";
    std::string pointLightQuadratic = pointLight + ".quadratic";

    std::string pointLightAmbient = pointLight + ".ambient";
    std::string pointLightDiffuse = pointLight + ".diffuse";
    std::string pointLightSpecular = pointLight + ".specular";

    lighting.setVec3(pointLightPosition.c_str(), pointLightPositions[i]);
    lighting.setFloat(pointLightConstant.c_str(), 1.0f);
    lighting.setFloat(pointLightLinear.c_str(), 0.09f);
    lighting.setFloat(pointLightQuadratic.c_str(), 0.032f);

    lighting.setVec3(pointLightAmbient.c_str(), ambientColor);
    lighting.setVec3(pointLightDiffuse.c_str(), ambientColor);
    lighting.setVec3(pointLightSpecular.c_str(), 1.0f, 1.0f, 1.0f);
  }


  lighting.setFloat("spotLight.constant", 1.0f);
  lighting.setFloat("spotLight.linear", 0.09f);
  lighting.setFloat("spotLight.quadratic", 0.032f);

  lighting.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
  lighting.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

  lighting.setVec3("spotLight.ambient", ambientColor);
  lighting.setVec3("spotLight.diffuse", diffuseColor);
  lighting.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);

  lighting.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
  lighting.setVec3("dirLight.ambient", ambientColor);
  lighting.setVec3("dirLight.diffuse", diffuseColor);
  lighting.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    float radius = 2.0f;
    float lightX = sin((float)glfwGetTime()) * radius;
    float lightZ = cos((float)glfwGetTime()) * radius;
    glm::vec3 lightPos = glm::vec3(0.f, -0.8f, 0.8f);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(
        glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));

    lighting.use();
    lighting.setVec3("viewPos", cameraPos);
    lighting.setVec3("spotLight.position", cameraPos);
    lighting.setVec3("spotLight.direction", cameraFront);

    lighting.setMat4("view", view);
    lighting.setMat4("projection", projection);
    lighting.setMat4("normalMatrix", normalMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);

    for (int i = 0; i < cubeNum; ++i) {
      model = glm::mat4(1.0f);
      // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f),
      //                     glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::translate(model, cubePositions[i]);
      lighting.setMat4("model", model);

      glBindVertexArray(containerVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);

    lightCube.use();
    lightCube.setVec3("lightColor", lightColor);
    lightCube.setMat4("view", view);
    lightCube.setMat4("projection", projection);

    for (int i = 0; i < 4; ++i) {

      model = glm::mat4(1.0f);
      model = glm::translate(model, pointLightPositions[i]);
      model = glm::scale(model, glm::vec3(0.2f));

      lightCube.setMat4("model", model);

      glBindVertexArray(lightcubeVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &containerVAO);
  glDeleteVertexArrays(1, &lightcubeVAO);
  glDeleteBuffers(1, &VBO);

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
  }
}

// ligh 嘟好

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

  float sensitivity = 0.02f;
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
    cout << "ERROR: Failed to load texture: " << imagePath << endl;
  }

  stbi_image_free(data);

  return textureID;
}

std::string getPath(const std::string& path) {
    fs::path fsPath(path);
    return fsPath.make_preferred().string();
}
