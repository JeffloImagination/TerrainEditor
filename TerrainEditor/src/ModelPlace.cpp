#include "ModelPlace.h"
#include <stdexcept>


// 添加一个模型位置矩阵到列表中
void ModelPlace::addModelMatrix(const glm::mat4& positionMatrix) {
    positionMatrices.push_back(positionMatrix);
}

// 渲染所有位置上的模型
void ModelPlace::renderAll(const Shader& shader) const {
    for (const auto& matrix : positionMatrices) {
        // 设置模型矩阵
        model->setModelMatrix(matrix);
        // 渲染模型
        model->render(shader);
    }
}

// 获取模型名称（通过Model类）
const std::string ModelPlace::getName() const {
    std::string modelName;
    modelName = model->getName();
    return modelName;
}

// 设置模型名称（通过Model类）
void ModelPlace::setName(const std::string& name) {
    model->setName(name);
}

// 获取模型
std::shared_ptr<Model> ModelPlace::getModel() const {
    return model; 
}


// 删除上一个添加的位置矩阵
void ModelPlace::removeLastModelMatrix() {
    if (!positionMatrices.empty()) {
        positionMatrices.pop_back();  // 删除最后一个矩阵
    }
}

void ModelPlace::increaseScale(float scaleSpeed) {
    glm::mat4 currentScaleMatrix = model->getScaleMatrix();
    float scaleValue = 1.0f + scaleSpeed;
    glm::mat4 newScaleMatrix = glm::scale(currentScaleMatrix, glm::vec3(scaleValue));
    model->setScaleMatrix(newScaleMatrix);

}

void ModelPlace::decreaseScale(float scaleSpeed) {
    glm::mat4 currentScaleMatrix = model->getScaleMatrix();
    float scaleValue = 1.0f - scaleSpeed;
    glm::mat4 newScaleMatrix = glm::scale(currentScaleMatrix, glm::vec3(scaleValue));
    model->setScaleMatrix(newScaleMatrix);
}