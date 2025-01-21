#include "ModelPlace.h"
#include <stdexcept>


// ���һ��ģ��λ�þ����б���
void ModelPlace::addModelMatrix(const glm::mat4& positionMatrix) {
    positionMatrices.push_back(positionMatrix);
}

// ��Ⱦ����λ���ϵ�ģ��
void ModelPlace::renderAll(const Shader& shader) const {
    for (const auto& matrix : positionMatrices) {
        // ����ģ�;���
        model->setModelMatrix(matrix);
        // ��Ⱦģ��
        model->render(shader);
    }
}

// ��ȡģ�����ƣ�ͨ��Model�ࣩ
const std::string ModelPlace::getName() const {
    std::string modelName;
    modelName = model->getName();
    return modelName;
}

// ����ģ�����ƣ�ͨ��Model�ࣩ
void ModelPlace::setName(const std::string& name) {
    model->setName(name);
}

// ��ȡģ��
std::shared_ptr<Model> ModelPlace::getModel() const {
    return model; 
}


// ɾ����һ����ӵ�λ�þ���
void ModelPlace::removeLastModelMatrix() {
    if (!positionMatrices.empty()) {
        positionMatrices.pop_back();  // ɾ�����һ������
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