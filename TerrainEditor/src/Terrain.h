#ifndef TERRAIN_H
#define TERRAIN_H

#include "Model.h"
#include <string>
#include <include/stb_image.h>

// 从Model类继承
class Terrain : public Model {
public:
    Terrain();
    ~Terrain();

    // 加载地形数据
    void load(const std::string& heightmapPath, int width, int height, float altitude, float texture_scale, const std::string& texturePath);

    // 获取给定世界坐标处的地形高度
    float getHeightAtWorldPosition(const glm::vec3& worldPos) const;

private:
    // 生成地形网格
    void generateTerrain(const std::string& heightmapPath, int width, int height, float altitude, float texture_scale);

    // 生成索引数据
    void generateIndices(int width, int height);

    // 计算法线
    void calculateNormals(int width, int height);

    // 地形网格尺寸
    int gridWidth;
    int gridHeight;

    // 地形海拔缩放因子
    float terrainAltitude;

    // 纹理缩放因子
    float terrainTextureScale;

    // 高度图图像宽度和高度
    int heightMapWidth;
    int heightMapHeight;

    // 高度图图像数据
    unsigned char* heightMapData;
};


// 计算鼠标射线
glm::vec3 getRayDirection(float mouseX, float mouseY, const glm::mat4& view, const glm::mat4& projection, int width, int height);

// 计算射线与地形的交点
glm::vec3 raycastTerrainIntersection(const glm::vec3& origin, const glm::vec3& direction, Terrain& terrain);



#endif // TERRAIN_H