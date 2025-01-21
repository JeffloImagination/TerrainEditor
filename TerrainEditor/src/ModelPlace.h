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
    // ʹ������ָ��
    ModelPlace(std::shared_ptr<Model> model) : model(model) {}

    // ���һ��λ�þ����б���
    void addModelMatrix(const glm::mat4& positionMatrix);

    // ��Ⱦ����λ���ϵ�ģ��
    void renderAll(const Shader& shader) const;

    // ��ȡ������ģ������
    const std::string getName() const;
    void setName(const std::string& name);

    // ��ȡģ��
    std::shared_ptr<Model> getModel() const;

    // ɾ����һ����ӵ�λ�þ���
    void removeLastModelMatrix();

    // ���ӻ����ģ�͵�����
    void increaseScale(float scaleSpeed);
    void decreaseScale(float scaleSpeed);

protected:
    // ģ��
    std::shared_ptr<Model> model;

    // λ�þ����б�
    std::vector<glm::mat4> positionMatrices;
};



#endif // MODELPLACE_H