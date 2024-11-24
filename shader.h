#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <ifstream>
#include <string>
using std::string;

class Shader {
public:
  unsigned ID;

  Shader(const char *vertexPath, const char *fragmentPath)
      : vertexPath(vertexPath), fragmentPath(fragmentPath) {}

  void use() { glUseProgram(ID); }

  int setBool(const std::string &name, bool value) const;
  int setInt(const std::string &name, int value) const;
  int setFloat(const std::string &name, float value) const;


  const char *vertexPath, *fragmentPath;
}

#endif // SHADER_H
