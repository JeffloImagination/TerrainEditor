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


// 顶点结构
struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;  // 法线
};

class Model {
public:
    Model(const std::string& name = "");
    virtual ~Model();

    // 渲染模型
    void render(const Shader& shader) const;

    // 获取和设置模型位置矩阵
    void setModelMatrix(const glm::mat4& modelMatrix);
    const glm::mat4& getModelMatrix() const;

    // 加载模型（传入文件夹）
    bool load(const std::string& dir_path);

    // 获取和设置名称
    const std::string& getName() const;
    void setName(const std::string& name);

    // 获取最低Y坐标
    float getMinY() const;

    // 获取和设置模型缩放矩阵
    void setScaleMatrix(const glm::mat4& scaleMatrix);
    const glm::mat4& getScaleMatrix() const;

protected:
    // 生成或更新OpenGL对象
    void generateOrUpdateOpenGLObjects();

    // 读取贴图数据并添加到OpenGL
    void loadTexture(const std::string& path, GLuint& textureID, std::vector<unsigned char>& textureData);

    std::vector<Vertex> vertices; // 顶点数据
    std::vector<unsigned int> indices; // 索引数据
    glm::mat4 modelMatrix; // 模型位置矩阵
    glm::mat4 scaleMatrix; // 缩放矩阵
  
    // OpenGL对象
    GLuint VAO, VBO, EBO; 

    // 贴图（漫反射，镜面反射，自发光，凹凸贴图）
    std::vector<unsigned char> diffuseMapData, specularMapData, emissionMapData, normalMapData; // 纹理实际数据
    GLuint diffuseMap, specularMap, emissionMap, normalMap; // GL纹理ID

    // 材质属性
    glm::vec3 Kd;  // 漫反射颜色
    glm::vec3 Ks;  // 镜面反射颜色
    glm::vec3 Ke;  // 自发光颜色
    glm::vec3 Ka;  // 环境光颜色
    float Ns;      // 镜面指数
    float d;       // 不透明度

private:
    std::string modelName; // 模型名称
    float minY = 0.0f; // 模型最低Y坐标
};

#endif // MODEL_H