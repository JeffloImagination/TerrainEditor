#include <include/glad/glad.h>
#include <include/GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <include/glm/glm.hpp>
#include <include/glm/gtc/matrix_transform.hpp>
#include <include/glm/gtc/type_ptr.hpp>

#include <include/stb_image.h>
#include "Shader.h"
#include "Camera.h"  
#include "Utils.h" 
#include "Model.h"
#include "Terrain.h"
#include "ModelPlace.h"
#include "Skybox.h"

#include <vector>
#include <algorithm>

#include <iomanip> // 用于打印debug信息


// 全局变量
// --------------------
// 声明回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height); // 帧缓冲大小改变回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // 鼠标位置变化回调函数
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // 鼠标滚轮滚动回调函数
void processInput(GLFWwindow* window); // 处理输入的函数

// 设置
const unsigned int SCR_WIDTH = 2000;
const unsigned int SCR_HEIGHT = 1500; // 窗口宽高设置

// 摄像机
Camera camera(glm::vec3(-55.0f, 55.0f, -55.0f)); // 初始化摄像机位置
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f; // 记录上次鼠标位置
bool firstMouse = true; // 标记是否是第一次获取鼠标位置
int scrWidth, scrHeight; // 屏幕宽度和高度
bool rightMouseButtonPressed = false; // 鼠标右键是否按下

// 光照参数
glm::vec3 lightPos = glm::vec3(-371.514, -1.69874, 100);
glm::vec3 lightColor = glm::vec3(0.944001, 0.928998, 0.498997);
float Kc = 1.0f;
float Kl = 9e-06;
float Kq = 3.2e-06;

// 定时
float deltaTime = 0.0f;	// 当前帧与上一帧之间的时间差
float lastFrame = 0.0f; // 上一帧的时间

// 用于设置模型矩阵
glm::mat4 translationMatrix;
glm::mat4 rotationMatrix;
glm::mat4 scaleMatrix;
glm::mat4 modelMatrix;

// 放置坐标的模型矩阵和旋转矩阵
glm::mat4 placeMatrix = glm::mat4(1.0f);
glm::mat4 placeRotationMatrix = glm::mat4(1.0f);
const float place_rotation_speed = 90.0f; // 旋转速度
const float place_scale_speed = 0.3f; // 缩放速度

// 放置列表
std::vector<ModelPlace> place_list;
int place_list_index = -1; // 当前列表索引，为-1表示不选择任何模型（仅显示鼠标）

// 鼠标左键和部分按键的最小触发间隔
double lastLeftMousePressTime = 0.0;
bool leftMouseButtonPressed = false;
const double leftMousePressInterval = 0.2; // 鼠标间隔时间，单位为秒

double lastKeyPressTime = 0.0;
bool keyPressActive = false;
const double keyPressInterval = 0.2; // 按键间隔时间，单位为秒


// 主函数
// --------------------
int main()
{
    // 初始化并配置glfw
    // --------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 使用核心模式

    // 创建glfw窗口
    scrWidth = SCR_WIDTH;
    scrHeight = SCR_HEIGHT;
    GLFWwindow* window = glfwCreateWindow(scrWidth, scrHeight, "OpenGL_final_TerrainEditor", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "创建GLFW窗口失败" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // 将窗口的上下文设为当前线程的主上下文

    // 注册回调
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // 帧缓冲大小改变回调
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 设置英文输入法
    SetEnglishInputLanguage();

    // 加载所有OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "初始化GLAD失败" << std::endl;
        return -1;
    }

    // 配置全局OpenGL状态
    glEnable(GL_DEPTH_TEST); // 启用深度测试

    // 构建模型着色器
    // --------------------
    std::string vertexShaderSource = ReadFile("resources/shader/vertexShader.vert");
    std::string fragmentShaderSource = ReadFile("resources/shader/fragmentShader.frag");

    Shader ourShader(vertexShaderSource.c_str(), fragmentShaderSource.c_str());

    // 初始化天空盒 
    // ------------------
    Skybox skybox;

    std::vector<std::string> faces{
        "resources/image/skybox/right.jpg",
        "resources/image/skybox/left.jpg",
        "resources/image/skybox/top.jpg",
        "resources/image/skybox/bottom.jpg",
        "resources/image/skybox/back.jpg",
        "resources/image/skybox/front.jpg"
    };

    std::string skyVertexSource = ReadFile("resources/shader/skybox.vert");
    std::string skyFragmentSource = ReadFile("resources/shader/skybox.frag");

    Shader skyboxShader(skyVertexSource.c_str(), skyFragmentSource.c_str());
    skybox.init(faces, skyboxShader);

    rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    skybox.setModelMatrix(rotationMatrix);

    // 生成地形网格
    // --------------------

    std::wstring defaultFilePath = GetAbsolutePathFromRelative(L"resources\\image\\height_map");
    std::wstring heightMapPath = BrowseForFile(NULL, defaultFilePath);
    if (heightMapPath.empty()) {
        std::cerr << "No height map selected." << std::endl;
    }

    // 获取纹理文件路径
    std::wstring texturePath = GetAbsolutePathFromRelative(L"resources\\image\\Grass_02.png"); // 改为使用默认草地贴图
    /*
    defaultFilePath = GetAbsolutePathFromRelative(L"resources\\image");
    std::wstring texturePath = BrowseForFile(NULL, defaultFilePath);
    if (texturePath.empty()) {
        std::cerr << "No texture selected." << std::endl;
    }
    */

    // 使用std::filesystem::path将宽字符路径转换为普通字符串
    std::filesystem::path heightMap(heightMapPath);
    std::filesystem::path tex(texturePath);

    std::cout << tex.string() << std::endl;

    // 加载地形
    Terrain terrain;
    terrain.load(heightMap.string(), 80, 80, 40, 10, tex.string());



    // 渲染循环
    // --------------------
    while (!glfwWindowShouldClose(window))
    {
        // 每帧时间逻辑
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 键盘输入处理
        processInput(window);

        // 渲染
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 设置清除颜色
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色缓冲区和深度缓冲区

        // 激活着色器
        ourShader.use();


        // 将投影矩阵传递给着色器
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        ourShader.setMat4("projection", projection);

        // 相机/视图变换
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        // 设置视点位置
        glm::vec3 viewPos = camera.GetPosition();  // 获取相机的位置
        ourShader.setVec3("viewPos", viewPos);


        // 设置光源的位置和颜色
        ourShader.setVec3("lightPos", lightPos);
        ourShader.setVec3("lightColor", lightColor);

        // 设置光源衰减系数
        ourShader.setFloat("Kc", Kc);
        ourShader.setFloat("Kl", Kl);
        ourShader.setFloat("Kq", Kq);

        // 渲染地形
        terrain.render(ourShader);

        // 渲染天空盒
        skybox.render(projection, camera.GetViewMatrix());

        // 获取鼠标位置
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // 计算射线
        glm::vec3 rayDirection = getRayDirection(static_cast<float>(xpos), static_cast<float>(ypos), camera.GetViewMatrix(), projection, SCR_WIDTH, SCR_HEIGHT);

        // 渲染跟随鼠标的模型
        if (place_list_index != -1 && place_list_index < static_cast<int>(place_list.size())) {
            ModelPlace& currentModelPlace = place_list[place_list_index];
            glm::vec3 intersectionPoint = raycastTerrainIntersection(camera.GetPosition(), rayDirection, terrain);

            // 获取模型并准备渲染
            std::shared_ptr<Model> currentModel = currentModelPlace.getModel();
            if (currentModel) {
                // 计算模型底部对齐地面的偏移量
                float liftAmount = -currentModel->getMinY(); // 获取模型的最低Y坐标，并取负值作为提升量

                // 获取模型的缩放矩阵
                glm::mat4 scaleMatrix = currentModel->getScaleMatrix();

                // 使用 Y 轴的缩放因子等比例调整 liftAmount
                float scaleY = glm::length(glm::vec3(scaleMatrix[1]));
                liftAmount *= scaleY;

                // 构建变换矩阵，先应用缩放，再将模型移动到交点位置，然后提升模型使其底部位于地面上，最后应用旋转
                glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), intersectionPoint);
                modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, liftAmount, 0.0f));
                modelMatrix = modelMatrix * placeRotationMatrix;
                placeMatrix = modelMatrix * scaleMatrix; // 应用缩放

                // 设置模型的变换矩阵
                currentModel->setModelMatrix(placeMatrix);

                // 渲染模型
                currentModel->render(ourShader);
            }
        }

        // 渲染所有放置的模型实例
        for (const auto& place : place_list) {
            place.renderAll(ourShader);
        }

        // glfw: 交换缓冲区并轮询IO事件（按键按下/释放，鼠标移动等）
        glfwSwapBuffers(window);
        glfwPollEvents();

        // debug
        /*
        // 打印光源和摄像机坐标
        static float printInterval = 1.0f; // 打印间隔时间（秒）
        static float lastPrintTime = 0.0f;
        if (currentFrame - lastPrintTime >= printInterval)
        {
            // 打印光源和摄像机坐标
            std::cout << "Light Position: ("
                << std::fixed << std::setprecision(2) << lightPos.x << ", "
                << std::fixed << std::setprecision(2) << lightPos.y << ", "
                << std::fixed << std::setprecision(2) << lightPos.z << ")" << std::endl;
            std::cout << "Camera Position: ("
                << std::fixed << std::setprecision(2) << viewPos.x << ", "
                << std::fixed << std::setprecision(2) << viewPos.y << ", "
                << std::fixed << std::setprecision(2) << viewPos.z << ")" << std::endl;

            // 更新上次打印时间
            lastPrintTime = currentFrame;
        }
        /* */
    }


    glfwTerminate();
    return 0;
}

// 回调和输入处理
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    // 摄像机移动
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime); // 如果按下W键，向前移动
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime); // 如果按下S键，向后移动
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime); // 如果按下A键，向左移动
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime); // 如果按下D键，向右移动
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime); // 如果按下Q键，向上移动
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime); // 如果按下E键，向下移动

    // 获取当前时间
    double currentTime = glfwGetTime();

    if (!keyPressActive && (currentTime - lastKeyPressTime >= keyPressInterval))
    {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            // 切换到下一个模型
            ++place_list_index;
            if (place_list_index >= static_cast<int>(place_list.size())) {
                place_list_index = -1; // 越界从-1开始
            }

            // 打印当前选择的模型名称
            if (place_list_index >= 0 && place_list_index < static_cast<int>(place_list.size())) {
                std::cout << "Current model: " << place_list[place_list_index].getName() << std::endl;
            }
            else {
                std::cout << "No model selected." << std::endl;
            }

            // 更新按键状态和时间
            lastKeyPressTime = currentTime;
            keyPressActive = true;
            placeRotationMatrix = glm::mat4(1.0f); // 重置放置角度
        }

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            // 切换到上一个模型
            --place_list_index;
            if (place_list_index < -1) {
                place_list_index = static_cast<int>(place_list.size()) - 1; // 越界从最后一个元素开始
            }

            // 打印当前选择的模型名称
            if (place_list_index >= 0 && place_list_index < static_cast<int>(place_list.size())) {
                std::cout << "Current model: " << place_list[place_list_index].getName() << std::endl;
            }
            else {
                std::cout << "No model selected." << std::endl;
            }

            // 更新按键状态和时间
            lastKeyPressTime = currentTime;
            keyPressActive = true;
            placeRotationMatrix = glm::mat4(1.0f); // 重置放置角度
        }

        // 按Z撤回上一个放置的模型
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            if (place_list_index >= 0 && place_list_index < static_cast<int>(place_list.size())) {
                place_list[place_list_index].removeLastModelMatrix();  // 撤回上一个模型
                std::cout << "Last model removed." << std::endl;
            }
            lastKeyPressTime = currentTime;
            keyPressActive = true;  // 防止重复触发
        }

        // 按M导入单个模型
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            std::wstring defaultFolderPath = GetAbsolutePathFromRelative(L"resources\\model");
            std::wstring folderPath = BrowseForFolder(NULL, defaultFolderPath);
            if (!folderPath.empty())
            {
                // 使用std::filesystem::path将宽字符路径转换为普通字符串
                std::filesystem::path selectedPath(folderPath);
                std::string dir_path = selectedPath.string();

                // 提取文件夹名称
                std::string model_name = selectedPath.filename().string();

                // 加载模型
                std::shared_ptr<Model> newModel = std::make_shared<Model>(model_name);
                if (newModel->load(dir_path)) {
                    // 创建新的 ModelPlace 并添加到列表
                    ModelPlace newPlace(newModel);
                    place_list.push_back(newPlace);

                    std::cout << "Model loaded from: " << model_name << std::endl;

                }
                else {
                    std::cerr << "Failed to load model from: " << model_name << std::endl;
                }
            }
        }

        // 按N导入所有模型
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            // 获取资源路径
            std::wstring defaultFolderPath = GetAbsolutePathFromRelative(L"resources\\model");

            // 遍历默认路径下的所有子目录（即模型文件夹）
            for (const auto& entry : std::filesystem::directory_iterator(defaultFolderPath)) {
                if (entry.is_directory()) {
                    // 使用 std::filesystem::path 将宽字符路径转换为普通字符串
                    std::filesystem::path selectedPath(entry.path());
                    std::string dir_path = selectedPath.string();

                    // 提取文件夹名称作为模型名称
                    std::string model_name = selectedPath.filename().string();

                    // 加载模型
                    std::shared_ptr<Model> newModel = std::make_shared<Model>(model_name);
                    if (newModel->load(dir_path)) {
                        // 创建新的 ModelPlace 并添加到列表
                        ModelPlace newPlace(newModel);
                        place_list.push_back(newPlace);

                        std::cout << "Model loaded from: " << model_name << std::endl;
                    }
                    else {
                        std::cerr << "Failed to load model from: " << model_name << std::endl;
                    }
                }
            }

            // 确保只在第一次按下 N 键时执行上述操作
            keyPressActive = true;
        }



    }

    // 检查是否已经过了按键间隔时间，重置按键状态
    if (keyPressActive && (currentTime - lastKeyPressTime >= keyPressInterval)) {
        keyPressActive = false;
    }

    // 按F和G旋转模型
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        float angle = place_rotation_speed * deltaTime;
        placeRotationMatrix = glm::rotate(placeRotationMatrix, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        keyPressActive = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        float angle = -place_rotation_speed * deltaTime;
        placeRotationMatrix = glm::rotate(placeRotationMatrix, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        keyPressActive = true;
    }


    // 按C和V缩放模型
    if (place_list_index != -1 && place_list_index < static_cast<int>(place_list.size())) {
        ModelPlace& currentModelPlace = place_list[place_list_index];

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !keyPressActive) {
            // 调整缩放
            currentModelPlace.increaseScale(place_scale_speed* deltaTime); 
            keyPressActive = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !keyPressActive) {
            // 调整缩放
            currentModelPlace.decreaseScale(place_scale_speed* deltaTime); 
            keyPressActive = true;
        }
    }

    // debug
    // 调整光照位置
    const float lightPosStep = 0.1f;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)//  按下 I 键，增加光照在 y 轴方向上的位置，即向上移动光源，每次增加 lightPosStep（定义为 1.0f）。
        lightPos.y += lightPosStep;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)//  按下 K 键，减少光照在 y 轴方向上的位置，使光源向下移动，每次减少 lightPosStep
        lightPos.y -= lightPosStep;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)//  按下 J 键，减少光照在 x 轴方向上的位置，使光源向左移动，每次减少 lightPosStep
        lightPos.x -= lightPosStep;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)//  按下 L 键，增加光照在 x 轴方向上的位置，使光源向右移动，每次增加 lightPosStep
        lightPos.x += lightPosStep;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)//  按下 U 键，增加光照在 z 轴方向上的位置，使光源向前移动，每次增加 lightPosStep
        lightPos.z += lightPosStep;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)//  按下 O 键，减少光照在 z 轴方向上的位置，使光源向后移动，每次减少 lightPosStep
        lightPos.z -= lightPosStep;

    // 调整光照颜色
    const float colorStep = 0.001f;
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)//  按下 1 键，增加光照颜色的红色分量，每次增加 colorStep（定义为 0.1f），使光照颜色更偏向红色。
        lightColor.r += colorStep;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)//  按下 2 键，减少光照颜色的红色分量，使光照颜色减少红色成分，每次减少 colorStep
        lightColor.r -= colorStep;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)//  按下 3 键，增加光照颜色的绿色分量，每次增加 colorStep，使光照颜色更偏向绿色
        lightColor.g += colorStep;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)//  按下 4 键，减少光照颜色的绿色分量，使光照颜色减少绿色成分，每次减少 colorStep
        lightColor.g -= colorStep;
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)//  按下 5 键，增加光照颜色的蓝色分量，每次增加 colorStep，使光照颜色更偏向蓝色
        lightColor.b += colorStep;
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)//  按下 6 键，减少光照颜色的蓝色分量，使光照颜色减少蓝色成分，每次减少 colorStep
        lightColor.b -= colorStep;

    // 调整光照衰减系数
    const float attenuationStep = 0.0001f;
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)//  按下 7 键，增加常数衰减系数 Kc，每次增加 attenuationStep（定义为 0.0001f），影响光照随距离衰减的程度。
        Kc += attenuationStep;
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)//  按下 8 键，减少常数衰减系数 Kc，使光照衰减程度降低，每次减少 attenuationStep
        Kc -= attenuationStep;

    float Kl_speed = 0.1;
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)//  按下 9 键，增加线性衰减系数 Kl，每次增加 attenuationStep，进一步调整光照随距离衰减的线性部分
        Kl += attenuationStep * Kl_speed;
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)//  按下 0 键，减少线性衰减系数 Kl，降低光照线性衰减效果，每次减少 attenuationStep
        Kl -= attenuationStep * Kl_speed;

    float Kq_speed = 0.0001;
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)//  按下 - 键，增加二次衰减系数 Kq，每次增加 attenuationStep，影响光照随距离衰减的二次方部分
        Kq += attenuationStep * Kq_speed;
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)//  按下 = 键，减少二次衰减系数 Kq，减弱光照二次衰减效果，每次减少 attenuationStep
        Kq -= attenuationStep * Kq_speed;

    // 打印光照参数
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        std::cout << "Light Position: (" << lightPos.x << ", " << lightPos.y << ", " << lightPos.z << ")\n";
        std::cout << "Light Color: (" << lightColor.r << ", " << lightColor.g << ", " << lightColor.b << ")\n";
        std::cout << "Attenuation Coefficients - Kc: " << Kc << ", Kl: " << Kl << ", Kq: " << Kq << "\n";

    }
}


void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (rightMouseButtonPressed) // 右键按下时才能旋转视角
    {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false; // 第一次获取鼠标位置后，设置为false
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // 因为y坐标是从底部到顶部，所以这里取反

        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset); // 处理鼠标移动
    }
}

// 鼠标按键回调函数
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        rightMouseButtonPressed = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        rightMouseButtonPressed = false;
        firstMouse = true; // 释放右键时重置firstMouse
    }

    // 处理鼠标左键点击
    double currentTime = glfwGetTime();
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !leftMouseButtonPressed &&
        (currentTime - lastLeftMousePressTime >= leftMousePressInterval)) {
        leftMouseButtonPressed = true;
        lastLeftMousePressTime = currentTime;

        // 如果有选择的模型，则保存其当前位置矩阵
        if (place_list_index != -1 && place_list_index < static_cast<int>(place_list.size())) {
            ModelPlace& currentModelPlace = place_list[place_list_index];
            currentModelPlace.addModelMatrix(placeMatrix); // 将当前placeMatrix添加到ModelPlace的位置矩阵数组中

            std::cout << "Model placed" << std::endl;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        leftMouseButtonPressed = false;
    }
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(2 * static_cast<float>(yoffset)); // 处理鼠标滚轮滚动
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // 防止宽高为0
    if (width > 0 && height > 0) {
        scrWidth = width;
        scrHeight = height;
        glViewport(0, 0, width, height); // 更新视口大小
    }
}


