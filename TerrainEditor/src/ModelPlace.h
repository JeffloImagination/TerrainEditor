#ifndef MODELPLACE_H
#define MODELPLACE_H
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include <include/glm/glm.hpp>
#include <include/glm/gtc/matrix_transform.hpp>
#include <include/glm/gtc/type_ptr.hpp>
#include <include/glad/glad.h>
#include <include/GLFW/glfw3.h>
#include <include/tiny_obj_loader.h>
#include <include/stb_image.h>

#include "Model.h"

class ModelPlace {
public:
    // 使用智能指针
    ModelPlace(std::shared_ptr<Model> model) : model(model) {}

    // 添加一个位置矩阵到列表中
    void addModelMatrix(const glm::mat4& positionMatrix);

    // 渲染所有位置上的模型
    void renderAll(const Shader& shader) const;

    // 获取和设置模型名称
    const std::string getName() const;
    void setName(const std::string& name);

    // 获取模型
    std::shared_ptr<Model> getModel() const;

    // 删除上一个添加的位置矩阵
    void removeLastModelMatrix();

    // 增加或减少模型的缩放
    void increaseScale(float scaleSpeed);
    void decreaseScale(float scaleSpeed);

protected:
    // 模型
    std::shared_ptr<Model> model;

    // 位置矩阵列表
    std::vector<glm::mat4> positionMatrices;
};



#endif // MODELPLACE_H