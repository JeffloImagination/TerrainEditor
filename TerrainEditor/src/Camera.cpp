#include "Camera.h"
#include <iostream>

// ���캯����ʹ��������
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

// ���캯����ʹ�ñ���ֵ��
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

// ������ͼ����
glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

glm::vec3 Camera::GetPosition() const
{
    return Position;
}


// ����ˮƽǰ������
glm::vec3 Camera::getHorizontalFront() const {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(0.0f)); // ʹ��0��Ϊ������
    front.y = 0.0f; // ˮƽ�ƶ�������Y��
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(0.0f));
    return glm::normalize(front);
}

// �����������
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;

    // ǰ�������ƶ������������ƫ���Ƕ�
    glm::vec3 horizontalFront = getHorizontalFront();
    if (direction == FORWARD)
        Position += horizontalFront * velocity;
    if (direction == BACKWARD)
        Position -= horizontalFront * velocity;
    if (direction == LEFT)
        Position -= glm::normalize(glm::cross(horizontalFront, WorldUp)) * velocity;
    if (direction == RIGHT)
        Position += glm::normalize(glm::cross(horizontalFront, WorldUp)) * velocity;

    // �����ƶ�Ϊ�����ƶ�������������ϵ��Y�ᣩ
    if (direction == UP)
        Position += WorldUp * velocity; // ʹ����������ϵ����������
    if (direction == DOWN)
        Position -= WorldUp * velocity;
}

// ��������ƶ�
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // ȷ���������ں���Χ��
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // ����ǰ���������������
    updateCameraVectors();
}

// ����������
void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < ZOOM_min)
        Zoom = ZOOM_min;
    if (Zoom > ZOOM_Max)
        Zoom = ZOOM_Max;
}

// �������������
void Camera::updateCameraVectors()
{
    // �����µ�ǰ������
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // ���¼����������������
    Right = glm::normalize(glm::cross(Front, WorldUp));  // ��һ����������Ϊ�����ϻ����¿�ʱ���������Ȼ�ӽ�0�������ƶ�������
    Up = glm::normalize(glm::cross(Right, Front));
}

