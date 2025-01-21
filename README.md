# 基于Opengl的简单地形编辑器

大三计算机图形学的小组作业，基于Opengl实现

Model.cpp : 模型类，使用TinyObjLoade加载obj和mtl模型
Terrain.cpp : 地形生成类，继承自Model，通过高度图生成地形。计算鼠标与地形的交互
ModelPlace.cpp : 模型放置类，储存一个模型所有放置坐标
Skybox.cpp : 天空盒类，储存天空盒数据

Camera.cpp :摄像机类
Shader.cpp: 着色器类，实现快捷的着色器输入
Utils.cpp: 存放工具函数

resource/shader/vertexShader.vert：顶点着色器
resource/shader/fragmentShader.frag：段着色器
resource/shader/skybox.vert：天空盒顶点着色器
resource/shader/skybox.frag：天空盒段着色器

基本操作逻辑：
1.启动时选择高度图（位于resources\image\height_map文件夹）
2.摄像机：wasd移动，qe上升下降，按住鼠标右键调整视角。
3.按m导入模型，下拉找到resources\model文件夹，选择其中的模型（例如container）
4.按n导入所有模型
5.按r和t调整当前要放置的模型，按f和g旋转，c和v缩放，点击左键放置。按z撤回。
