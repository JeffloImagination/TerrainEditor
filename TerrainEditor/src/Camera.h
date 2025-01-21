#ifndef CAMERA_H
#define CAMERA_H

#include <include/glm/glm.hpp>
#include <include/glm/gtc/matrix_transform.hpp>
#include <include/GLFW/glfw3.h>

// ������ƶ�����ö��
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Ĭ�����������
const float YAW = 45.0f; // ƫ����
const float PITCH = -30.0f; // ������
const float SPEED = 35.0f; // �ƶ��ٶ�
const float SENSITIVITY = 0.05f; // �ӽ�������
const float ZOOM = 45.0f;

const float ZOOM_min = 5.0f;
const float ZOOM_Max = 70.0f;

class Camera
{
public:
    // ���������
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // ŷ����
    float Yaw;
    float Pitch;
    // �����ѡ��
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // ��������ϵ�еķ�������
    static constexpr glm::vec3 WorldForward = glm::vec3(0.0f, 0.0f, -1.0f);
    static constexpr glm::vec3 WorldRight = glm::vec3(1.0f, 0.0f, 0.0f);
    static constexpr glm::vec3 WorldUpVector = glm::vec3(0.0f, 1.0f, 0.0f);

    // ���캯����ʹ��������
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    // ���캯����ʹ�ñ���ֵ��
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    // ������ͼ����
    glm::mat4 GetViewMatrix();

    // ���������λ��
    glm::vec3 GetPosition() const;

    // �����������
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    // ��������ƶ�
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // ����������
    void ProcessMouseScroll(float yoffset);

    // ����ˮƽǰ������
    glm::vec3 getHorizontalFront() const;

private:
    // �������������
    void updateCameraVectors();
};

#endif // CAMERA_H#pragma once
