#version 330 core

// 输入属性
layout(location = 0) in vec3 aPos;       // 顶点位置
layout(location = 1) in vec2 aTexCoord;  // 顶点纹理坐标
layout(location = 2) in vec3 aNormal;    // 顶点法线

// 输出到片段着色器的变量
out vec3 FragPos;    // 片段的世界空间位置
out vec3 Normal;     // 法线
out vec2 TexCoord;   // 纹理坐标


// Uniform变量
uniform mat4 model;  // 模型矩阵
uniform mat4 view;   // 视图矩阵
uniform mat4 projection;  // 投影矩阵

void main()
{
    // 计算世界空间中的顶点位置
    FragPos = vec3(model * vec4(aPos, 1.0));

    // 将法线从模型空间转换到世界空间
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // 直接传递纹理坐标
    TexCoord = aTexCoord;

    // 计算裁剪空间下的顶点位置
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

