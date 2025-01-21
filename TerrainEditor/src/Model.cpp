#include "Model.h"
#include <iostream>
#include <fstream>

Model::Model(const std::string& name) :
    modelName(name),
    VAO(0), VBO(0), EBO(0),
    diffuseMapData(), specularMapData(), emissionMapData(), normalMapData(), // ��ʼ��Ϊ��
    diffuseMap(0), specularMap(0), emissionMap(0), normalMap(0),
    Kd(glm::vec3(1.0f)), Ks(glm::vec3(0.8f)), Ke(glm::vec3(0.0f)), Ka(glm::vec3(0.01f)),
    Ns(1.0f), d(1.0f),
    modelMatrix(glm::mat4(1.0f)), scaleMatrix(glm::mat4(1.0f))
{
    // ��ʼ��OpenGL����
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

    // ʹ�� tinyobj::ObjReader �� ObjReaderConfig ��ȡ .obj �ļ�
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = directory.string();  // ���ò����ļ�����·��

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

    // �����㡢���ߡ���������������
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
                    1.0 - attrib.texcoords[2 * index.texcoord_index + 1]  // ע�⣺�������������Ҫ��תY��
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

            // ������СY����
            minY = std::min(minY, vertex.position.y);
        }
    }


    // ��ȡ .mtl �ļ��������������
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

        // ��ȡ����
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

    // ���ɻ����OpenGL����
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

        // ��ͼ�����ݴ洢��textureData��
        textureData.assign(data, data + width * height * nrChannels);

        // �����������
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // ���������װ����
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // ����������˲���
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // �ϴ��������ݵ�OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // �ͷ�stbi���ص�����
        stbi_image_free(data);
    }
    else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
}

void Model::generateOrUpdateOpenGLObjects()
{
    if (VAO == 0 || VBO == 0 || EBO == 0) {
        // ����OpenGL����
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    glBindVertexArray(VAO);

    // ��VBO���ϴ���������
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // ��EBO���ϴ���������
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // ���ö�������ָ��
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);

    // ���VAO
    glBindVertexArray(0);
}

//*
void Model::render(const Shader& shader) const {
    // ʹ��ָ������ɫ��
    shader.use();

    // ��ģ�;��󴫵ݸ���ɫ��
    shader.setMat4("model", modelMatrix);

    // ���ݲ�������
    shader.setVec3("Kd", Kd);
    shader.setVec3("Ks", Ks);
    shader.setVec3("Ke", Ke);
    shader.setVec3("Ka", Ka);
    shader.setFloat("Ns", Ns);
    shader.setFloat("d", d);

    // �����Ƿ�����Ӧ����ı�־
    shader.setBool("hasDiffuseMap", !diffuseMapData.empty());
    shader.setBool("hasSpecularMap", !specularMapData.empty());
    shader.setBool("hasEmissionMap", !emissionMapData.empty());
    shader.setBool("hasNormalMap", !normalMapData.empty());

    // �������������
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

    // ��VAO
    glBindVertexArray(VAO);

    // ����ģ��
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // ���VAO
    glBindVertexArray(0);
}
/**/

/* ��debug�İ汾��
void Model::render(const Shader& shader) const {
    // ʹ��ָ������ɫ��
    shader.use();

    // ��ģ�;��󴫵ݸ���ɫ��
    shader.setMat4("model", modelMatrix);

    // ���ݲ�������
    shader.setVec3("Kd", Kd);
    shader.setVec3("Ks", Ks);
    shader.setVec3("Ke", Ke);
    shader.setVec3("Ka", Ka);
    shader.setFloat("Ns", Ns);
    shader.setFloat("d", d);

    // ��ӡ��������
    std::cout << "Material Properties:" << std::endl;
    std::cout << "  Kd: (" << Kd.r << ", " << Kd.g << ", " << Kd.b << ")" << std::endl;
    std::cout << "  Ks: (" << Ks.r << ", " << Ks.g << ", " << Ks.b << ")" << std::endl;
    std::cout << "  Ke: (" << Ke.r << ", " << Ke.g << ", " << Ke.b << ")" << std::endl;
    std::cout << "  Ka: (" << Ka.r << ", " << Ka.g << ", " << Ka.b << ")" << std::endl;
    std::cout << "  Ns: " << Ns << std::endl;
    std::cout << "  d: " << d << std::endl;

    // �����Ƿ�����Ӧ����ı�־
    shader.setBool("hasDiffuseMap", !diffuseMapData.empty());
    shader.setBool("hasSpecularMap", !specularMapData.empty());
    shader.setBool("hasEmissionMap", !emissionMapData.empty());
    shader.setBool("hasNormalMap", !normalMapData.empty());

    // ��ӡ����״̬
    std::cout << "Texture States:" << std::endl;
    std::cout << "  hasDiffuseMap: " << (!diffuseMapData.empty() ? "true" : "false") << std::endl;
    std::cout << "  hasSpecularMap: " << (!specularMapData.empty() ? "true" : "false") << std::endl;
    std::cout << "  hasEmissionMap: " << (!emissionMapData.empty() ? "true" : "false") << std::endl;
    std::cout << "  hasNormalMap: " << (!normalMapData.empty() ? "true" : "false") << std::endl;

    // �������������
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

    // ��VAO
    glBindVertexArray(VAO);
    std::cout << "VAO bound, ID: " << VAO << std::endl;

    // ����ģ��
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    std::cout << "Model drawn with " << indices.size() << " indices." << std::endl;

    // ���VAO
    glBindVertexArray(0);
    std::cout << "VAO unbound." << std::endl;

    // ���OpenGL����
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

// �������ž���
void Model::setScaleMatrix(const glm::mat4& scaleMatrix) {
    this->scaleMatrix = scaleMatrix;
}

// ��ȡ���ž���
const glm::mat4& Model::getScaleMatrix() const {
    return scaleMatrix;
}

