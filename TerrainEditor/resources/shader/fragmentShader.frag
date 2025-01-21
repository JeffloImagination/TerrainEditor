#version 330 core
in vec3 FragPos;    // 从顶点着色器接收的片段位置
in vec3 Normal;     // 从顶点着色器接收的法线
in vec2 TexCoord;   // 从顶点着色器接收的纹理坐标

out vec4 FragColor; // 最终输出的颜色

uniform sampler2D diffuseMap;   // 漫反射贴图
uniform sampler2D specularMap;  // 镜面反射贴图
uniform sampler2D emissionMap;  // 自发光贴图
uniform sampler2D normalMap;    // 凹凸贴图

uniform bool hasDiffuseMap;     // 是否有漫反射贴图
uniform bool hasSpecularMap;    // 是否有镜面反射贴图
uniform bool hasEmissionMap;    // 是否有自发光贴图
uniform bool hasNormalMap;      // 是否有凹凸贴图

uniform vec3 Kd;                // 漫反射颜色
uniform vec3 Ks;                // 镜面反射颜色
uniform vec3 Ke;                // 自发光颜色

uniform float Ns;               // 镜面指数
uniform vec3 Ka;                // 环境光颜色
uniform float d;                // 不透明度

uniform vec3 viewPos;           // 视点位置
uniform vec3 lightPos;          // 光源位置
uniform vec3 lightColor;        // 光源颜色
uniform float Kc;               // 常数衰减系数
uniform float Kl;               // 线性衰减系数
uniform float Kq;               // 二次衰减系数

void main()
{
    // 采样漫反射贴图
    vec3 diffuse = hasDiffuseMap ? texture(diffuseMap, TexCoord).rgb : Kd;

    // 采样镜面反射贴图
    vec3 specular = hasSpecularMap ? texture(specularMap, TexCoord).rgb : Ks;

    // 采样自发光贴图
    vec3 emission = hasEmissionMap ? texture(emissionMap, TexCoord).rgb : Ke;

    // 采样凹凸贴图，并转换为法线
    vec3 normalSample = hasNormalMap ? texture(normalMap, TexCoord).rgb : vec3(0.0, 0.0, 1.0);
    normalSample = normalize(normalSample * 2.0 - 1.0); // 将[0,1]范围映射到[-1,1]
    vec3 normal = normalize(Normal + normalSample);

    // 计算光源到片段的方向
    vec3 lightDir = normalize(lightPos - FragPos);

    // 计算光源到片段的距离
    float distance = length(lightPos - FragPos);

    // 计算视角向量
    vec3 viewDir = normalize(viewPos - FragPos);

    // 计算半角向量
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // 计算衰减因子
    float attenuation = 1.0 / (Kc + Kl * distance + Kq * distance * distance);

    // 计算各个光照成分
    vec3 ambient = Ka * diffuse;
    vec3 diffuseLight = max(dot(normal, lightDir), 0.0) * diffuse * lightColor; // 暂时取消衰减
    vec3 specularLight = pow(max(dot(normal, halfwayDir), 0.0), Ns) * specular * lightColor;

    // 组合所有光照成分
    vec3 result = (ambient + diffuseLight + specularLight + emission) * d;

    // 暂时关闭光照渲染
    // result = (ambient + diffuse + specularLight + emission) * d;

    // 输出最终颜色
    FragColor = vec4(result, 1.0);
}