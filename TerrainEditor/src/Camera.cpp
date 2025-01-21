#include "Camera.h"
#include <iostream>

// 构造函数（使用向量）
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) :
    Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVITY),
    Zoom(ZOOM)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// 构造函数（使用标量值）
Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) :
    Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVITY),
    Zoom(ZOOM)
{
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// 返回视图矩阵
glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

glm::vec3 Camera::GetPosition() const
{
    return Position;
}


// 计算水平前向向量
glm::vec3 Camera::getHorizontalFront() const {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(0.0f)); // 使用0作为俯仰角
    front.y = 0.0f; // 水平移动不考虑Y轴
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(0.0f));
    return glm::normalize(front);
}

// 处理键盘输入
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;

    // 前后左右移动跟随摄像机的偏航角度
    glm::vec3 horizontalFront = getHorizontalFront();
    if (direction == FORWARD)
        Position += horizontalFront * velocity;
    if (direction == BACKWARD)
        Position -= horizontalFront * velocity;
    if (direction == LEFT)
        Position -= glm::normalize(glm::cross(horizontalFront, WorldUp)) * velocity;
    if (direction == RIGHT)
        Position += glm::normalize(glm::cross(horizontalFront, WorldUp)) * velocity;

    // 上下移动为绝对移动（沿世界坐标系的Y轴）
    if (direction == UP)
        Position += WorldUp * velocity; // 使用世界坐标系的上向向量
    if (direction == DOWN)
        Position -= WorldUp * velocity;
}

// 处理鼠标移动
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // 确保俯仰角在合理范围内
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // 更新前向、右向和上向向量
    updateCameraVectors();
}

// 处理鼠标滚轮
void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < ZOOM_min)
        Zoom = ZOOM_min;
    if (Zoom > ZOOM_Max)
        Zoom = ZOOM_Max;
}

// 更新摄像机向量
void Camera::updateCameraVectors()
{
    // 计算新的前向向量
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // 重新计算右向和上向向量
    Right = glm::normalize(glm::cross(Front, WorldUp));  // 归一化向量，因为当向上或向下看时，向量长度会接近0，导致移动变慢。
    Up = glm::normalize(glm::cross(Right, Front));
}

