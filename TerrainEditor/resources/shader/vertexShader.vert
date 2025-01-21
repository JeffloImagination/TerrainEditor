#version 330 core

// ��������
layout(location = 0) in vec3 aPos;       // ����λ��
layout(location = 1) in vec2 aTexCoord;  // ������������
layout(location = 2) in vec3 aNormal;    // ���㷨��

// �����Ƭ����ɫ���ı���
out vec3 FragPos;    // Ƭ�ε�����ռ�λ��
out vec3 Normal;     // ����
out vec2 TexCoord;   // ��������


// Uniform����
uniform mat4 model;  // ģ�;���
uniform mat4 view;   // ��ͼ����
uniform mat4 projection;  // ͶӰ����

void main()
{
    // ��������ռ��еĶ���λ��
    FragPos = vec3(model * vec4(aPos, 1.0));

    // �����ߴ�ģ�Ϳռ�ת��������ռ�
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // ֱ�Ӵ�����������
    TexCoord = aTexCoord;

    // ����ü��ռ��µĶ���λ��
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

