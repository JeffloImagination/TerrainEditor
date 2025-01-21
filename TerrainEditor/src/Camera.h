#ifndef CAMERA_H
#define CAMERA_H

#include <include/glm/glm.hpp>
#include <include/glm/gtc/matrix_transform.hpp>
#include <include/GLFW/glfw3.h>

// 摄像机移动方向枚举
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// 默认摄像机参数
const float YAW = 45.0f; // 偏航角
const float PITCH = -30.0f; // 俯仰角
const float SPEED = 35.0f; // 移动速度
const float SENSITIVITY = 0.05f; // 视角灵敏度
const float ZOOM = 45.0f;

const float ZOOM_min = 5.0f;
const float ZOOM_Max = 70.0f;

class Camera
{
public:
    // 摄像机属性
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // 欧拉角
    float Yaw;
    float Pitch;
    // 摄像机选项
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // 世界坐标系中的方向向量
    static constexpr glm::vec3 WorldForward = glm::vec3(0.0f, 0.0f, -1.0f);
    static constexpr glm::vec3 WorldRight = glm::vec3(1.0f, 0.0f, 0.0f);
    static constexpr glm::vec3 WorldUpVector = glm::vec3(0.0f, 1.0f, 0.0f);

    // 构造函数（使用向量）
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    // 构造函数（使用标量值）
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    // 返回视图矩阵
    glm::mat4 GetViewMatrix();

    // 返回摄像机位置
    glm::vec3 GetPosition() const;

    // 处理键盘输入
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    // 处理鼠标移动
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // 处理鼠标滚轮
    void ProcessMouseScroll(float yoffset);

    // 计算水平前向向量
    glm::vec3 getHorizontalFront() const;

private:
    // 更新摄像机向量
    void updateCameraVectors();
};

#endif // CAMERA_H#pragma once
