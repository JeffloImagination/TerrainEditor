#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include <include/glm/glm.hpp>
#include <include/glad/glad.h>
#include <include/GLFW/glfw3.h>
#include <include/tiny_obj_loader.h>
#include <include/stb_image.h>

#include "Shader.h"


// ����ṹ
struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;  // ����
};

class Model {
public:
    Model(const std::string& name = "");
    virtual ~Model();

    // ��Ⱦģ��
    void render(const Shader& shader) const;

    // ��ȡ������ģ��λ�þ���
    void setModelMatrix(const glm::mat4& modelMatrix);
    const glm::mat4& getModelMatrix() const;

    // ����ģ�ͣ������ļ��У�
    bool load(const std::string& dir_path);

    // ��ȡ����������
    const std::string& getName() const;
    void setName(const std::string& name);

    // ��ȡ���Y����
    float getMinY() const;

    // ��ȡ������ģ�����ž���
    void setScaleMatrix(const glm::mat4& scaleMatrix);
    const glm::mat4& getScaleMatrix() const;

protected:
    // ���ɻ����OpenGL����
    void generateOrUpdateOpenGLObjects();

    // ��ȡ��ͼ���ݲ���ӵ�OpenGL
    void loadTexture(const std::string& path, GLuint& textureID, std::vector<unsigned char>& textureData);

    std::vector<Vertex> vertices; // ��������
    std::vector<unsigned int> indices; // ��������
    glm::mat4 modelMatrix; // ģ��λ�þ���
    glm::mat4 scaleMatrix; // ���ž���
  
    // OpenGL����
    GLuint VAO, VBO, EBO; 

    // ��ͼ�������䣬���淴�䣬�Է��⣬��͹��ͼ��
    std::vector<unsigned char> diffuseMapData, specularMapData, emissionMapData, normalMapData; // ����ʵ������
    GLuint diffuseMap, specularMap, emissionMap, normalMap; // GL����ID

    // ��������
    glm::vec3 Kd;  // ��������ɫ
    glm::vec3 Ks;  // ���淴����ɫ
    glm::vec3 Ke;  // �Է�����ɫ
    glm::vec3 Ka;  // ��������ɫ
    float Ns;      // ����ָ��
    float d;       // ��͸����

private:
    std::string modelName; // ģ������
    float minY = 0.0f; // ģ�����Y����
};

#endif // MODEL_H