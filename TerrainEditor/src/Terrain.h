#ifndef TERRAIN_H
#define TERRAIN_H

#include "Model.h"
#include <string>
#include <include/stb_image.h>

// ��Model��̳�
class Terrain : public Model {
public:
    Terrain();
    ~Terrain();

    // ���ص�������
    void load(const std::string& heightmapPath, int width, int height, float altitude, float texture_scale, const std::string& texturePath);

    // ��ȡ�����������괦�ĵ��θ߶�
    float getHeightAtWorldPosition(const glm::vec3& worldPos) const;

private:
    // ���ɵ�������
    void generateTerrain(const std::string& heightmapPath, int width, int height, float altitude, float texture_scale);

    // ������������
    void generateIndices(int width, int height);

    // ���㷨��
    void calculateNormals(int width, int height);

    // ��������ߴ�
    int gridWidth;
    int gridHeight;

    // ���κ�����������
    float terrainAltitude;

    // ������������
    float terrainTextureScale;

    // �߶�ͼͼ���Ⱥ͸߶�
    int heightMapWidth;
    int heightMapHeight;

    // �߶�ͼͼ������
    unsigned char* heightMapData;
};


// �����������
glm::vec3 getRayDirection(float mouseX, float mouseY, const glm::mat4& view, const glm::mat4& projection, int width, int height);

// ������������εĽ���
glm::vec3 raycastTerrainIntersection(const glm::vec3& origin, const glm::vec3& direction, Terrain& terrain);



#endif // TERRAIN_H