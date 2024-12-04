#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
// glad must be included before GLFW
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

class Shader {
public:
    unsigned ID;
    Shader(const std::string &vsPath, const std::string &fsPath) {
        std::string vsCode = getContentFromFile(vsPath);
        std::string fsCode = getContentFromFile(fsPath);

        if (vsCode.empty() || fsCode.empty()) {
            throw std::runtime_error("Failed to load shader files.");
        }

        unsigned vertex = glCreateShader(GL_VERTEX_SHADER);
        const char* vsCodeCStr = vsCode.c_str();
        glShaderSource(vertex, 1, &vsCodeCStr, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        unsigned fragment = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fsCodeCStr = fsCode.c_str();
        glShaderSource(fragment, 1, &fsCodeCStr, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    } 

    ~Shader() { glDeleteProgram(ID); }

    void use() { glUseProgram(ID); }

    void setBool(const char *uniform, bool value) {
        glUniform1i(getUniformLocation(uniform), (int)value);
    }

    void setInt(const char *uniform, int value) {
        glUniform1i(getUniformLocation(uniform), value);
    }

    void setFloat(const char *uniform, float value) {
        glUniform1f(getUniformLocation(uniform), value);
    }

    void setVec2(const char *uniform, glm::vec2 &value) {
        glUniform2fv(getUniformLocation(uniform), 1, &value[0]);
    }

    void setVec2(const char *uniform, float x, float y) {
        glUniform2f(getUniformLocation(uniform), x, y);
    }

    void setVec3(const char *uniform, glm::vec3 &value) {
        glUniform3fv(getUniformLocation(uniform), 1, &value[0]);
    }

    void setVec3(const char *uniform, float x, float y, float z) {
        glUniform3f(getUniformLocation(uniform), x, y, z);
    }

    void setVec4(const char *uniform, glm::vec4 &value) {
        glUniform4fv(getUniformLocation(uniform), 1, &value[0]);
    }

    void setVec4(const char *uniform, float x, float y, float z, float w) {
        glUniform4f(getUniformLocation(uniform), x, y, z, w);
    }

    void setMat2(const char *uniform, glm::mat2 &value) {
        glUniformMatrix2fv(getUniformLocation(uniform), 1, GL_FALSE, &value[0][0]);
    }

    void setMat3(const char *uniform, glm::mat3 &value) {
        glUniformMatrix3fv(getUniformLocation(uniform), 1, GL_FALSE, &value[0][0]);
    }

    void setMat4(const char *uniform, glm::mat4 &value) {
        glUniformMatrix4fv(getUniformLocation(uniform), 1, GL_FALSE, &value[0][0]);
    }

private:
    int getUniformLocation(const char *uniform) {
        int loc = glGetUniformLocation(ID, uniform);
        if (loc == -1) {
            std::cerr << "ERROR: Failed to get location of uniform " << uniform << std::endl;
            return -1;
        }
        return loc;
    }

std::string getContentFromFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "ERROR: Failed to open " << filePath << std::endl;
        return ""; // Return an empty string on failure
    }

    std::ostringstream sstream;
    sstream << file.rdbuf();
    return sstream.str(); // Return the full content as a string
}

    void checkCompileErrors(unsigned ID, const char *type_cstr)  {
        int success;
        const unsigned logSize = 1024;
        char infoLog[logSize];

        std::string type(type_cstr);

        if (type == "PROGRAM") {
            glGetProgramiv(ID, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(ID, logSize, NULL, infoLog);
                std::cerr << "ERROR::SHADER::" << type << "::LINKING_FAILED\n" << infoLog << std::endl;
            }
        } else {
            glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(ID, logSize, NULL, infoLog);
                std::cerr << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
            }
        }
    }
};

#endif
