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

    // ��ʼ��
    void init(const std::vector<std::string>& faces, const Shader& shader);

    // ��Ⱦ��պ�
    void render(const glm::mat4& projection, const glm::mat4& view) const;

    // ���úͻ�ȡģ�;���
    void setModelMatrix(const glm::mat4& modelMatrix);
    const glm::mat4& getModelMatrix() const;

private:
    unsigned int VAO, VBO;
    unsigned int cubemapTexture;
    Shader* skyboxShader;

    // ��պж�������
    float skybox_vertices[108] = {   //��պж�������
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

    // ģ�;���
    glm::mat4 modelMatrix;

    // ���ܺ��������ؿռ������
    unsigned int load_cubemap(const std::vector<std::string>& faces);

    // ��ʼ��VAO��VBO
    void setupGeometry();
};

#endif // SKYBOX_H