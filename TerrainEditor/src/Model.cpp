#include "Model.h"
#include <iostream>
#include <fstream>

Model::Model(const std::string& name) :
    modelName(name),
    VAO(0), VBO(0), EBO(0),
    diffuseMapData(), specularMapData(), emissionMapData(), normalMapData(), // 初始化为空
    diffuseMap(0), specularMap(0), emissionMap(0), normalMap(0),
    Kd(glm::vec3(1.0f)), Ks(glm::vec3(0.8f)), Ke(glm::vec3(0.0f)), Ka(glm::vec3(0.01f)),
    Ns(1.0f), d(1.0f),
    modelMatrix(glm::mat4(1.0f)), scaleMatrix(glm::mat4(1.0f))
{
    // 初始化OpenGL对象
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    //std::cout << "newmodelName:" << modelName << std::endl;
}

Model::~Model()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

bool Model::load(const std::string& dir_path) {
    std::filesystem::path directory(dir_path);
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        std::cerr << "Directory does not exist: " << dir_path << std::endl;
        return false;
    }

    std::string folder_name = directory.filename().string();
    std::string obj_file = (directory / (folder_name + ".obj")).string();

    if (!std::filesystem::exists(obj_file)) {
        std::cerr << "OBJ file does not exist: " << obj_file << std::endl;
        return false;
    }

    // 使用 tinyobj::ObjReader 和 ObjReaderConfig 读取 .obj 文件
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = directory.string();  // 设置材质文件搜索路径

    if (!reader.ParseFromFile(obj_file, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error() << std::endl;
        }
        return false;
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning() << std::endl;
    }


    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // 处理顶点、法线、索引和纹理坐标
    minY = std::numeric_limits<float>::max();
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {};

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.texcoord_index >= 0) {
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0 - attrib.texcoords[2 * index.texcoord_index + 1]  // 注意：纹理坐标可能需要翻转Y轴
                };
            }
            else {
                vertex.texCoord = { 0.0f, 0.0f };
            }

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }
            else {
                vertex.normal = { 0.0f, 0.0f, 0.0f };
            }

            vertices.push_back(vertex);
            indices.push_back(static_cast<unsigned int>(vertices.size()) - 1);

            // 更新最小Y坐标
            minY = std::min(minY, vertex.position.y);
        }
    }


    // 读取 .mtl 文件并处理材质属性
    for (const auto& material : materials) {
        if (material.diffuse[0] != 0.0f || material.diffuse[1] != 0.0f || material.diffuse[2] != 0.0f) {
            Kd = glm::vec3(material.diffuse[0], material.diffuse[1], material.diffuse[2]);
        }
        if (material.specular[0] != 0.0f || material.specular[1] != 0.0f || material.specular[2] != 0.0f) {
            Ks = glm::vec3(material.specular[0], material.specular[1], material.specular[2]);
        }
        if (material.emission[0] != 0.0f || material.emission[1] != 0.0f || material.emission[2] != 0.0f) {
            Ke = glm::vec3(material.emission[0], material.emission[1], material.emission[2]);
        }
        if (material.ambient[0] != 0.0f || material.ambient[1] != 0.0f || material.ambient[2] != 0.0f) {
            Ka = glm::vec3(material.ambient[0], material.ambient[1], material.ambient[2]);
        }
        if (material.shininess > 0.0f) {
            Ns = material.shininess;
        }
        if (material.dissolve > 0.0f) {
            d = material.dissolve;
        }

        // 读取纹理
        if (!material.diffuse_texname.empty()) {
            std::string path = (directory / material.diffuse_texname).string();
            loadTexture(path, diffuseMap, diffuseMapData);
        }

        if (!material.specular_texname.empty()) {
            std::string path = (directory / material.specular_texname).string();
            loadTexture(path, specularMap, specularMapData);
        }

        if (!material.emissive_texname.empty()) {
            std::string path = (directory / material.emissive_texname).string();
            loadTexture(path, emissionMap, emissionMapData);
        }

        if (!material.bump_texname.empty()) {
            std::string path = (directory / material.bump_texname).string();
            loadTexture(path, normalMap, normalMapData);
        }

    }

    // 生成或更新OpenGL对象
    generateOrUpdateOpenGLObjects();

    return true;
}

void Model::loadTexture(const std::string& path, GLuint& textureID, std::vector<unsigned char>& textureData) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format;
        if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        // 将图像数据存储到textureData中
        textureData.assign(data, data + width * height * nrChannels);

        // 生成纹理对象
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // 设置纹理包装参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // 设置纹理过滤参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 上传纹理数据到OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // 释放stbi加载的数据
        stbi_image_free(data);
    }
    else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
}

void Model::generateOrUpdateOpenGLObjects()
{
    if (VAO == 0 || VBO == 0 || EBO == 0) {
        // 生成OpenGL对象
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    glBindVertexArray(VAO);

    // 绑定VBO并上传顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // 绑定EBO并上传索引数据
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);

    // 解绑VAO
    glBindVertexArray(0);
}

//*
void Model::render(const Shader& shader) const {
    // 使用指定的着色器
    shader.use();

    // 将模型矩阵传递给着色器
    shader.setMat4("model", modelMatrix);

    // 传递材质属性
    shader.setVec3("Kd", Kd);
    shader.setVec3("Ks", Ks);
    shader.setVec3("Ke", Ke);
    shader.setVec3("Ka", Ka);
    shader.setFloat("Ns", Ns);
    shader.setFloat("d", d);

    // 传递是否有相应纹理的标志
    shader.setBool("hasDiffuseMap", !diffuseMapData.empty());
    shader.setBool("hasSpecularMap", !specularMapData.empty());
    shader.setBool("hasEmissionMap", !emissionMapData.empty());
    shader.setBool("hasNormalMap", !normalMapData.empty());

    // 传递纹理采样器
    if (!diffuseMapData.empty()) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        shader.setInt("diffuseMap", 0);
    }

    if (!specularMapData.empty()) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        shader.setInt("specularMap", 1);
    }

    if (!emissionMapData.empty()) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, emissionMap);
        shader.setInt("emissionMap", 2);
    }

    if (!normalMapData.empty()) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        shader.setInt("normalMap", 3);
    }

    // 绑定VAO
    glBindVertexArray(VAO);

    // 绘制模型
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // 解绑VAO
    glBindVertexArray(0);
}
/**/

/* 带debug的版本：
void Model::render(const Shader& shader) const {
    // 使用指定的着色器
    shader.use();

    // 将模型矩阵传递给着色器
    shader.setMat4("model", modelMatrix);

    // 传递材质属性
    shader.setVec3("Kd", Kd);
    shader.setVec3("Ks", Ks);
    shader.setVec3("Ke", Ke);
    shader.setVec3("Ka", Ka);
    shader.setFloat("Ns", Ns);
    shader.setFloat("d", d);

    // 打印材质属性
    std::cout << "Material Properties:" << std::endl;
    std::cout << "  Kd: (" << Kd.r << ", " << Kd.g << ", " << Kd.b << ")" << std::endl;
    std::cout << "  Ks: (" << Ks.r << ", " << Ks.g << ", " << Ks.b << ")" << std::endl;
    std::cout << "  Ke: (" << Ke.r << ", " << Ke.g << ", " << Ke.b << ")" << std::endl;
    std::cout << "  Ka: (" << Ka.r << ", " << Ka.g << ", " << Ka.b << ")" << std::endl;
    std::cout << "  Ns: " << Ns << std::endl;
    std::cout << "  d: " << d << std::endl;

    // 传递是否有相应纹理的标志
    shader.setBool("hasDiffuseMap", !diffuseMapData.empty());
    shader.setBool("hasSpecularMap", !specularMapData.empty());
    shader.setBool("hasEmissionMap", !emissionMapData.empty());
    shader.setBool("hasNormalMap", !normalMapData.empty());

    // 打印纹理状态
    std::cout << "Texture States:" << std::endl;
    std::cout << "  hasDiffuseMap: " << (!diffuseMapData.empty() ? "true" : "false") << std::endl;
    std::cout << "  hasSpecularMap: " << (!specularMapData.empty() ? "true" : "false") << std::endl;
    std::cout << "  hasEmissionMap: " << (!emissionMapData.empty() ? "true" : "false") << std::endl;
    std::cout << "  hasNormalMap: " << (!normalMapData.empty() ? "true" : "false") << std::endl;

    // 传递纹理采样器
    if (!diffuseMapData.empty()) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        shader.setInt("diffuseMap", 0);
        std::cout << "  Diffuse Texture bound to GL_TEXTURE0, ID: " << diffuseMap << std::endl;
    }

    if (!specularMapData.empty()) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        shader.setInt("specularMap", 1);
        std::cout << "  Specular Texture bound to GL_TEXTURE1, ID: " << specularMap << std::endl;
    }

    if (!emissionMapData.empty()) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, emissionMap);
        shader.setInt("emissionMap", 2);
        std::cout << "  Emission Texture bound to GL_TEXTURE2, ID: " << emissionMap << std::endl;
    }

    if (!normalMapData.empty()) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        shader.setInt("normalMap", 3);
        std::cout << "  Normal Texture bound to GL_TEXTURE3, ID: " << normalMap << std::endl;
    }

    // 绑定VAO
    glBindVertexArray(VAO);
    std::cout << "VAO bound, ID: " << VAO << std::endl;

    // 绘制模型
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    std::cout << "Model drawn with " << indices.size() << " indices." << std::endl;

    // 解绑VAO
    glBindVertexArray(0);
    std::cout << "VAO unbound." << std::endl;

    // 检查OpenGL错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error: " << error << std::endl;
    }
}
/**/

void Model::setModelMatrix(const glm::mat4& modelMatrix)
{
    this->modelMatrix = modelMatrix;
}

const glm::mat4& Model::getModelMatrix() const
{
    return modelMatrix;
}

const std::string& Model::getName() const {
    return modelName;
}

void Model::setName(const std::string& name) {
    modelName = name;
}

float Model::getMinY() const {
    return minY;
}

// 设置缩放矩阵
void Model::setScaleMatrix(const glm::mat4& scaleMatrix) {
    this->scaleMatrix = scaleMatrix;
}

// 获取缩放矩阵
const glm::mat4& Model::getScaleMatrix() const {
    return scaleMatrix;
}

