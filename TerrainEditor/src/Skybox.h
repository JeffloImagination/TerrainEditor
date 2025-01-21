#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <string>
#include <include/glad/glad.h>
#include <include/GLFW/glfw3.h>
#include <include/glm/glm.hpp>  

#include "Shader.h"

class Skybox {
public:
    Skybox();
    ~Skybox();

    // 初始化
    void init(const std::vector<std::string>& faces, const Shader& shader);

    // 渲染天空盒
    void render(const glm::mat4& projection, const glm::mat4& view) const;

    // 设置和获取模型矩阵
    void setModelMatrix(const glm::mat4& modelMatrix);
    const glm::mat4& getModelMatrix() const;

private:
    unsigned int VAO, VBO;
    unsigned int cubemapTexture;
    Shader* skyboxShader;

    // 天空盒顶点数据
    float skybox_vertices[108] = {   //天空盒顶点数组
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    // 模型矩阵
    glm::mat4 modelMatrix;

    // 功能函数：加载空间盒纹理
    unsigned int load_cubemap(const std::vector<std::string>& faces);

    // 初始化VAO和VBO
    void setupGeometry();
};

#endif // SKYBOX_H