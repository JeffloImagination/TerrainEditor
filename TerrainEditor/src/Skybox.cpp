#include "Skybox.h"
#include <include/glm/glm.hpp>
#include <include/glm/gtc/matrix_transform.hpp>
#include <include/glm/gtc/type_ptr.hpp>
#include <iostream>
#include <include/stb_image.h>

Skybox::Skybox() : VAO(0), VBO(0), cubemapTexture(0), skyboxShader(nullptr), modelMatrix(glm::mat4(1.0f)) {}

Skybox::~Skybox() {
    if (cubemapTexture) glDeleteTextures(1, &cubemapTexture);
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    delete skyboxShader;
}

void Skybox::init(const std::vector<std::string>& faces, const Shader& shader) {
    cubemapTexture = load_cubemap(faces);
    skyboxShader = new Shader(shader); // 深拷贝着色器，或使用引用/指针根据需求调整
    setupGeometry();
}

unsigned int Skybox::load_cubemap(const std::vector<std::string>& faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void Skybox::setupGeometry() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void Skybox::setModelMatrix(const glm::mat4& modelMatrix) {
    this->modelMatrix = modelMatrix;
}

const glm::mat4& Skybox::getModelMatrix() const {
    return modelMatrix;
}

void Skybox::render(const glm::mat4& projection, const glm::mat4& view) const {
    glDepthFunc(GL_LEQUAL);  // 更改深度函数以确保天空盒在所有物体后面
    glDepthMask(GL_FALSE);
    skyboxShader->use();

    // 应用模型矩阵到视图矩阵，移除平移部分
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
    glm::mat4 skyboxModelView = skyboxView * modelMatrix;

    skyboxShader->setMat4("view", skyboxModelView);
    skyboxShader->setMat4("projection", projection);

    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS); // 恢复默认深度函数
}