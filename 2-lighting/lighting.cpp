#include <glad/glad.h>
// glad must be included before GLFW
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <string>
using std::cout, std::endl;

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

// container vertex shader
const char *containerVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normalMatrix;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  FragPos = vec3(model * vec4(aPos, 1.0));
  Normal = mat3(normalMatrix) * aNormal; 
  TexCoord = aTexCoord;
})";

const char *containerFragmentShaderSource = R"(
#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

out vec4 FragColor;

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shinness;
};

struct DirLight {
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct PointLight {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

struct SpotLight {
  vec3 position;
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;

  float cutOff;
  float outerCutOff;
};

uniform Material material;

uniform DirLight dirLight;
#define NR_POINT_LIGHT 4
uniform PointLight pointLight[NR_POINT_LIGHT];
uniform SpotLight spotLight;

uniform vec3 viewPos;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 fragPos);

void main() {
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 normal = normalize(Normal);

  vec3 result = vec3(0.0f);
  //result += CalcDirLight(dirLight, normal, viewDir);
  
  for (int i = 0; i < NR_POINT_LIGHT; ++i) {
    //result += CalcPointLight(pointLight[i], normal, viewDir, FragPos);
  }

  result += CalcSpotLight(spotLight, normal, viewDir, FragPos);

  FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
  vec3 sampleDiffuse = vec3(texture(material.diffuse, TexCoord));

  vec3 ambient = sampleDiffuse * light.ambient;

  vec3 lightDir = normalize(-light.direction);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = sampleDiffuse * diff * light.diffuse;

  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shinness);
  vec3 specular = vec3(texture(material.specular, TexCoord)) * spec * light.specular;

  return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos) {
  vec3 sampleDiffuse = vec3(texture(material.diffuse, TexCoord));

  vec3 ambient = sampleDiffuse * light.ambient;

  vec3 lightDir = normalize(light.position - fragPos);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = sampleDiffuse * diff * light.diffuse;

  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shinness);
  vec3 specular = texture(material.specular, TexCoord).rgb * spec * light.specular;

  float distance = length(light.position - fragPos);
  float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;

  return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 fragPos) {
  vec3 sampleDiffuse = texture(material.diffuse, TexCoord).rgb;
  vec3 ambient = sampleDiffuse * light.ambient;

  vec3 lightDir = normalize(light.position - fragPos);

  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = sampleDiffuse * diff * light.diffuse;

  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shinness);
  vec3 specular = texture(material.specular, TexCoord).rgb * spec * light.specular;

  float distance = length(light.position - fragPos);
  float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;

  float theta = dot(lightDir, normalize(-light.direction));
  float epsilon = light.cutOff - light.outerCutOff;
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

  diffuse *= intensity;
  specular *= intensity;

  return (ambient + diffuse + specular);
}

)";

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
uniform vec3 lightColor;

void main() {
  FragColor = vec4(lightColor, 1.0);
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
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

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
  unsigned textures[2];
  glGenTextures(2, textures);

  // texture unit 0
  glBindTexture(GL_TEXTURE_2D, textures[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, nrChannels;
#ifdef __APPLE__
  unsigned char *data = stbi_load(
      (std::string(PROJECT_SOURCE_DIR) + "/resources/container2.png").c_str(),
      &width, &height, &nrChannels, 0);
#else
  unsigned char *data = stbi_load(
      (std::string(PROJECT_SOURCE_DIR) + "\\resources\\container2.png").c_str(),
      &width, &height, &nrChannels, 0);
#endif
  if (data) {
    // copy data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    cout << "Failed to load texture" << endl;
  }

  // texture unit 1
  glBindTexture(GL_TEXTURE_2D, textures[1]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef __APPLE__
  data = stbi_load(
      (std::string(PROJECT_SOURCE_DIR) + "/resources/container2_specular.png")
          .c_str(),
      &width, &height, &nrChannels, 0);
#else
  data = stbi_load(
      (std::string(PROJECT_SOURCE_DIR) + "\\resources\\container2_specular.png")
          .c_str(),
      &width, &height, &nrChannels, 0);
#endif
  if (data) {
    // copy data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    cout << "Failed to load texture" << endl;
  }

  stbi_image_free(data);

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

  glUseProgram(containerShaderProgram);
  glUniform1i(glGetUniformLocation(containerShaderProgram, "material.diffuse"),
              0);
  glUniform1i(glGetUniformLocation(containerShaderProgram, "material.specular"),
              1);

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

    glUniform3fv(glGetUniformLocation(containerShaderProgram,
                                      pointLightPosition.c_str()),
                 1, glm::value_ptr(pointLightPositions[i]));
    glUniform1f(glGetUniformLocation(containerShaderProgram,
                                     pointLightConstant.c_str()),
                1.0f);
    glUniform1f(
        glGetUniformLocation(containerShaderProgram, pointLightLinear.c_str()),
        0.09f);
    glUniform1f(glGetUniformLocation(containerShaderProgram,
                                     pointLightQuadratic.c_str()),
                0.032f);

    glUniform3fv(
        glGetUniformLocation(containerShaderProgram, pointLightAmbient.c_str()),
        1, glm::value_ptr(ambientColor));
    glUniform3fv(
        glGetUniformLocation(containerShaderProgram, pointLightDiffuse.c_str()),
        1, glm::value_ptr(diffuseColor));
    glUniform3f(glGetUniformLocation(containerShaderProgram,
                                     pointLightSpecular.c_str()),
                1.0f, 1.0f, 1.0f);
  }

  glUniform3fv(glGetUniformLocation(containerShaderProgram, "viewPos"), 1,
               glm::value_ptr(cameraPos));
  glUniform3f(glGetUniformLocation(containerShaderProgram, "material.ambient"),
              1.0f, 0.5f, 0.31f);
  glUniform3f(glGetUniformLocation(containerShaderProgram, "material.diffuse"),
              1.0f, 0.5f, 0.31f);
  glUniform3f(glGetUniformLocation(containerShaderProgram, "material.specular"),
              0.5f, 0.5f, 0.5f);
  glUniform1f(glGetUniformLocation(containerShaderProgram, "material.shinness"),
              32.0f);
  // float lightColor[] = {1.0f, sin((float)glfwGetTime()), 1.0f};
  glUniform3fv(
      glGetUniformLocation(containerShaderProgram, "spotLight.position"), 1,
      glm::value_ptr(cameraPos));
  glUniform3fv(
      glGetUniformLocation(containerShaderProgram, "spotLight.direction"), 1,
      glm::value_ptr(cameraFront));

  glUniform1f(
      glGetUniformLocation(containerShaderProgram, "spotLight.constant"), 1.0f);
  glUniform1f(glGetUniformLocation(containerShaderProgram, "spotLight.linear"),
              0.09f);
  glUniform1f(
      glGetUniformLocation(containerShaderProgram, "spotLight.quadratic"),
      0.032f);

  glUniform1f(glGetUniformLocation(containerShaderProgram, "spotLight.cutOff"),
              glm::cos(glm::radians(12.5f)));
  glUniform1f(
      glGetUniformLocation(containerShaderProgram, "spotLight.outerCutOff"),
      glm::cos(glm::radians(17.5f)));

  glm::vec3 spotLightColor = glm::vec3(0.1f, 0.f, 0.2f);
  glUniform3fv(
      glGetUniformLocation(containerShaderProgram, "spotLight.ambient"), 1,
      glm::value_ptr(ambientColor));
  glUniform3fv(
      glGetUniformLocation(containerShaderProgram, "spotLight.diffuse"), 1,
      glm::value_ptr(diffuseColor));
  glUniform3f(
      glGetUniformLocation(containerShaderProgram, "spotLight.specular"), 1.0f,
      1.0f, 1.0f);

  glUniform3f(
      glGetUniformLocation(containerShaderProgram, "dirLight.direction"), -0.2f,
      -1.0f, -0.3f);
  glUniform3fv(glGetUniformLocation(containerShaderProgram, "dirLight.ambient"),
               1, glm::value_ptr(ambientColor));
  glUniform3fv(glGetUniformLocation(containerShaderProgram, "dirLight.diffuse"),
               1, glm::value_ptr(diffuseColor));
  glUniform3f(glGetUniformLocation(containerShaderProgram, "dirLight.specular"),
              1.0f, 1.0f, 1.0f);

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

    glUseProgram(containerShaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(containerShaderProgram, "view"), 1,
                       GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(
        glGetUniformLocation(containerShaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));
    glUniformMatrix4fv(
        glGetUniformLocation(containerShaderProgram, "normalMatrix"), 1,
        GL_FALSE, glm::value_ptr(normalMatrix));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);

    for (int i = 0; i < cubeNum; ++i) {
      model = glm::mat4(1.0f);
      // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f),
      //                     glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::translate(model, cubePositions[i]);
      glUniformMatrix4fv(glGetUniformLocation(containerShaderProgram, "model"),
                         1, GL_FALSE, glm::value_ptr(model));

      glBindVertexArray(containerVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);

    glUseProgram(lightcubeShaderProgram);
    glUniform3fv(glGetUniformLocation(lightcubeShaderProgram, "lightColor"), 1,
                 glm::value_ptr(lightColor));

    glUniformMatrix4fv(glGetUniformLocation(lightcubeShaderProgram, "view"), 1,
                       GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(
        glGetUniformLocation(lightcubeShaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));

    for (int i = 0; i < 4; ++i) {

      model = glm::mat4(1.0f);
      model = glm::translate(model, pointLightPositions[i]);
      model = glm::scale(model, glm::vec3(0.2f));

      glUniformMatrix4fv(glGetUniformLocation(lightcubeShaderProgram, "model"),
                         1, GL_FALSE, glm::value_ptr(model));

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
  glDeleteProgram(containerShaderProgram);
  glDeleteProgram(lightcubeShaderProgram);

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
