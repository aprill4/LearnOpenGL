#include "shader.h"

int Shader::setBool(const std::string &name, bool value) const {
  int loc = glGetUniformLocation(ID, name.c_str());
  if (loc == -1) {
    return -1;
  }

  glUniform1i(loc, (int)value);
  return 0;
}
