#include "Terrain.h"
#include <stdexcept>


Terrain::Terrain()
    : gridWidth(0), gridHeight(0), terrainAltitude(0.0f), terrainTextureScale(1.0f),
    heightMapWidth(0), heightMapHeight(0), heightMapData(nullptr), Model()
{
}

Terrain::~Terrain()
{
    if (heightMapData) {
        stbi_image_free(heightMapData);
    }
}

void Terrain::load(const std::string& heightmapPath, int width, int height, float altitude, float texture_scale, const std::string& texturePath)
{
    // 储存地形网格尺寸和海拔
    gridWidth = width;
    gridHeight = height;
    terrainAltitude = altitude;
    terrainTextureScale = texture_scale;

    // 生成地形网格
    generateTerrain(heightmapPath, width, height, altitude, texture_scale);

    // 生成索引数据
    generateIndices(width, height);

    // 生成法线数据
    calculateNormals(width, height);

    // 初始化OpenGL对象
    generateOrUpdateOpenGLObjects();

    // 添加纹理
    loadTexture(texturePath, diffuseMap, diffuseMapData);
}

void Terrain::generateTerrain(const std::string& heightmapPath, int width, int height, float altitude, float texture_scale) {
    int channels;
    heightMapData = stbi_load(heightmapPath.c_str(), &heightMapWidth, &heightMapHeight, &channels, 1);
    if (!heightMapData) {
        throw std::runtime_error("Failed to load heightmap: " + heightmapPath);
    }

    vertices.clear();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // 计算纹理坐标
            float u = static_cast<float>(x) / (width - 1);
            float v = static_cast<float>(y) / (height - 1);

            // 使用双线性插值获取高度值
            float imgX = u * (heightMapWidth - 1);
            float imgY = v * (heightMapHeight - 1);
            int imgX0 = static_cast<int>(imgX);
            int imgY0 = static_cast<int>(imgY);
            int imgX1 = imgX0 + 1;
            int imgY1 = imgY0 + 1;

            // 添加边界检查，确保相邻像素索引在图像范围内
            if (imgX1 >= heightMapWidth) {
                imgX1 = heightMapWidth - 1;
            }
            if (imgY1 >= heightMapHeight) {
                imgY1 = heightMapHeight - 1;
            }

            // 处理边界像素的特殊情况
            if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
                imgX0 = std::clamp(imgX0, 0, heightMapWidth - 1);
                imgX1 = std::clamp(imgX1, 0, heightMapWidth - 1);
                imgY0 = std::clamp(imgY0, 0, heightMapHeight - 1);
                imgY1 = std::clamp(imgY1, 0, heightMapHeight - 1);
            }

            float dx = imgX - imgX0;
            float dy = imgY - imgY0;
            float h00 = static_cast<float>(heightMapData[imgY0 * heightMapWidth + imgX0]) / 255.0f;
            float h01 = static_cast<float>(heightMapData[imgY0 * heightMapWidth + imgX1]) / 255.0f;
            float h10 = static_cast<float>(heightMapData[imgY1 * heightMapWidth + imgX0]) / 255.0f;
            float h11 = static_cast<float>(heightMapData[imgY1 * heightMapWidth + imgX1]) / 255.0f;
            float heightValue = (1.0f - dx) * (1.0f - dy) * h00 +
                dx * (1.0f - dy) * h01 +
                (1.0f - dx) * dy * h10 +
                dx * dy * h11;

            // 计算顶点位置
            float posX = (x - (width / 2.0f));
            float posY = heightValue * altitude;
            float posZ = (y - (height / 2.0f));
            vertices.push_back({ {posX, posY, posZ}, {u * texture_scale, v * texture_scale} });
        }
    }
}

void Terrain::generateIndices(int width, int height)
{
    indices.clear();
    for (int y = 0; y < height - 1; ++y)
    {
        for (int x = 0; x < width - 1; ++x)
        {
            unsigned int topLeft = y * width + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (y + 1) * width + x;
            unsigned int bottomRight = bottomLeft + 1;

            // 生成两个三角形
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}


void Terrain::calculateNormals(int width, int height)
{
    // 清除现有的法线数据
    for (auto& vertex : vertices)
    {
        vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    // 计算每个三角形的法线，并累加到其顶点上
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        glm::vec3 v1 = vertices[i1].position - vertices[i0].position;
        glm::vec3 v2 = vertices[i2].position - vertices[i0].position;

        glm::vec3 faceNormal = glm::normalize(glm::cross(v1, v2));

        vertices[i0].normal += faceNormal;
        vertices[i1].normal += faceNormal;
        vertices[i2].normal += faceNormal;
    }

    // 对每个顶点的法线进行归一化，并打印非边缘顶点的法线
    const size_t PRINT_EVERY_NTH_VERTEX = 1; 

    for (size_t i = 0; i < vertices.size(); ++i)
    {

        

        auto& vertex = vertices[i];
        vertex.normal = glm::normalize(vertex.normal);

        /*
        // 计算当前顶点在网格中的x, y位置
        int x = i % width;
        int y = i / width;

        // 检查是否为边缘顶点
        bool isEdgeVertex = (x == 0 || x == width - 1 || y == 0 || y == height - 1);

        // 打印法线信息及xy位置，但仅限于特定间隔的顶点，并且不是边缘顶点
        if (!isEdgeVertex && (i % PRINT_EVERY_NTH_VERTEX == 0)) {
            std::cout << "Vertex[" << i << "] at (" << x << ", " << y << ") Normal: ("
                << vertex.normal.x << ", "
                << vertex.normal.y << ", "
                << vertex.normal.z << ")" << std::endl;
        }
        /**/
    }
}


float Terrain::getHeightAtWorldPosition(const glm::vec3& worldPos) const {
    // 将世界坐标转换为相对于地形网格的坐标
    float x = worldPos.x + (gridWidth / 2.0f);
    float z = worldPos.z + (gridHeight / 2.0f);

    // 检查是否在地形范围内
    if (x < 0 || x >= gridWidth || z < 0 || z >= gridHeight) {
        return -1.0f; // 返回无效高度值表示超出范围
    }

    // 计算纹理坐标
    float u = static_cast<float>(x) / (gridWidth - 1);
    float v = static_cast<float>(z) / (gridHeight - 1);

    // 使用双线性插值获取高度值
    float imgX = u * (heightMapWidth - 1);
    float imgZ = v * (heightMapHeight - 1);
    int imgX0 = static_cast<int>(imgX);
    int imgZ0 = static_cast<int>(imgZ);
    int imgX1 = imgX0 + 1;
    int imgZ1 = imgZ0 + 1;

    // 添加边界检查，确保相邻像素索引在图像范围内
    if (imgX1 >= heightMapWidth) {
        imgX1 = heightMapWidth - 1;
    }
    if (imgZ1 >= heightMapHeight) {
        imgZ1 = heightMapHeight - 1;
    }

    float dx = imgX - imgX0;
    float dz = imgZ - imgZ0;
    float h00 = static_cast<float>(heightMapData[imgZ0 * heightMapWidth + imgX0]) / 255.0f;
    float h01 = static_cast<float>(heightMapData[imgZ0 * heightMapWidth + imgX1]) / 255.0f;
    float h10 = static_cast<float>(heightMapData[imgZ1 * heightMapWidth + imgX0]) / 255.0f;
    float h11 = static_cast<float>(heightMapData[imgZ1 * heightMapWidth + imgX1]) / 255.0f;
    float heightValue = (1.0f - dx) * (1.0f - dz) * h00 +
        dx * (1.0f - dz) * h01 +
        (1.0f - dx) * dz * h10 +
        dx * dz * h11;

    // 返回海拔缩放后的高度值
    return heightValue * terrainAltitude;
}



glm::vec3 getRayDirection(float mouseX, float mouseY, const glm::mat4& view, const glm::mat4& projection, int width, int height) {
    // 将鼠标位置归一化到[-1, 1]范围
    float ndcX = (2.0f * mouseX / width) - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY / height);

    // 构建投影矩阵的逆矩阵
    glm::mat4 invProjection = glm::inverse(projection);
    glm::mat4 invView = glm::inverse(view);

    // 将NDC坐标转换到裁剪空间
    glm::vec4 clipSpacePos = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);

    // 转换到视图空间
    glm::vec4 viewSpacePos = invProjection * clipSpacePos;
    viewSpacePos = glm::vec4(viewSpacePos.x, viewSpacePos.y, -1.0f, 0.0f); // 设置为方向向量

    // 转换到世界空间
    glm::vec4 worldSpaceDir = invView * viewSpacePos;

    // 归一化得到射线方向
    return glm::normalize(glm::vec3(worldSpaceDir));
}


glm::vec3 raycastTerrainIntersection(const glm::vec3& origin, const glm::vec3& direction, Terrain& terrain) {
    float tMin = 0.0f;
    float tMax = 1000.0f; // 假设最大距离为1000个单位
    float epsilon = 0.01f;

    while (tMax - tMin > epsilon) {
        float tMid = (tMin + tMax) / 2.0f;
        glm::vec3 point = origin + tMid * direction;

        if (terrain.getHeightAtWorldPosition(point) >= point.y) {
            tMax = tMid;
        }
        else {
            tMin = tMid;
        }
    }

    // 返回交点位置
    return origin + tMin * direction;
}