#version 330 core
in vec3 FragPos;    // �Ӷ�����ɫ�����յ�Ƭ��λ��
in vec3 Normal;     // �Ӷ�����ɫ�����յķ���
in vec2 TexCoord;   // �Ӷ�����ɫ�����յ���������

out vec4 FragColor; // �����������ɫ

uniform sampler2D diffuseMap;   // ��������ͼ
uniform sampler2D specularMap;  // ���淴����ͼ
uniform sampler2D emissionMap;  // �Է�����ͼ
uniform sampler2D normalMap;    // ��͹��ͼ

uniform bool hasDiffuseMap;     // �Ƿ�����������ͼ
uniform bool hasSpecularMap;    // �Ƿ��о��淴����ͼ
uniform bool hasEmissionMap;    // �Ƿ����Է�����ͼ
uniform bool hasNormalMap;      // �Ƿ��а�͹��ͼ

uniform vec3 Kd;                // ��������ɫ
uniform vec3 Ks;                // ���淴����ɫ
uniform vec3 Ke;                // �Է�����ɫ

uniform float Ns;               // ����ָ��
uniform vec3 Ka;                // ��������ɫ
uniform float d;                // ��͸����

uniform vec3 viewPos;           // �ӵ�λ��
uniform vec3 lightPos;          // ��Դλ��
uniform vec3 lightColor;        // ��Դ��ɫ
uniform float Kc;               // ����˥��ϵ��
uniform float Kl;               // ����˥��ϵ��
uniform float Kq;               // ����˥��ϵ��

void main()
{
    // ������������ͼ
    vec3 diffuse = hasDiffuseMap ? texture(diffuseMap, TexCoord).rgb : Kd;

    // �������淴����ͼ
    vec3 specular = hasSpecularMap ? texture(specularMap, TexCoord).rgb : Ks;

    // �����Է�����ͼ
    vec3 emission = hasEmissionMap ? texture(emissionMap, TexCoord).rgb : Ke;

    // ������͹��ͼ����ת��Ϊ����
    vec3 normalSample = hasNormalMap ? texture(normalMap, TexCoord).rgb : vec3(0.0, 0.0, 1.0);
    normalSample = normalize(normalSample * 2.0 - 1.0); // ��[0,1]��Χӳ�䵽[-1,1]
    vec3 normal = normalize(Normal + normalSample);

    // �����Դ��Ƭ�εķ���
    vec3 lightDir = normalize(lightPos - FragPos);

    // �����Դ��Ƭ�εľ���
    float distance = length(lightPos - FragPos);

    // �����ӽ�����
    vec3 viewDir = normalize(viewPos - FragPos);

    // ����������
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // ����˥������
    float attenuation = 1.0 / (Kc + Kl * distance + Kq * distance * distance);

    // ����������ճɷ�
    vec3 ambient = Ka * diffuse;
    vec3 diffuseLight = max(dot(normal, lightDir), 0.0) * diffuse * lightColor; // ��ʱȡ��˥��
    vec3 specularLight = pow(max(dot(normal, halfwayDir), 0.0), Ns) * specular * lightColor;

    // ������й��ճɷ�
    vec3 result = (ambient + diffuseLight + specularLight + emission) * d;

    // ��ʱ�رչ�����Ⱦ
    // result = (ambient + diffuse + specularLight + emission) * d;

    // ���������ɫ
    FragColor = vec4(result, 1.0);
}