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

#include <iomanip> // ���ڴ�ӡdebug��Ϣ


// ȫ�ֱ���
// --------------------
// �����ص�����
void framebuffer_size_callback(GLFWwindow* window, int width, int height); // ֡�����С�ı�ص�����
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // ���λ�ñ仯�ص�����
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // �����ֹ����ص�����
void processInput(GLFWwindow* window); // ��������ĺ���

// ����
const unsigned int SCR_WIDTH = 2000;
const unsigned int SCR_HEIGHT = 1500; // ���ڿ������

// �����
Camera camera(glm::vec3(-55.0f, 55.0f, -55.0f)); // ��ʼ�������λ��
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f; // ��¼�ϴ����λ��
bool firstMouse = true; // ����Ƿ��ǵ�һ�λ�ȡ���λ��
int scrWidth, scrHeight; // ��Ļ��Ⱥ͸߶�
bool rightMouseButtonPressed = false; // ����Ҽ��Ƿ���

// ���ղ���
glm::vec3 lightPos = glm::vec3(-371.514, -1.69874, 100);
glm::vec3 lightColor = glm::vec3(0.944001, 0.928998, 0.498997);
float Kc = 1.0f;
float Kl = 9e-06;
float Kq = 3.2e-06;

// ��ʱ
float deltaTime = 0.0f;	// ��ǰ֡����һ֮֡���ʱ���
float lastFrame = 0.0f; // ��һ֡��ʱ��

// ��������ģ�;���
glm::mat4 translationMatrix;
glm::mat4 rotationMatrix;
glm::mat4 scaleMatrix;
glm::mat4 modelMatrix;

// ���������ģ�;������ת����
glm::mat4 placeMatrix = glm::mat4(1.0f);
glm::mat4 placeRotationMatrix = glm::mat4(1.0f);
const float place_rotation_speed = 90.0f; // ��ת�ٶ�
const float place_scale_speed = 0.3f; // �����ٶ�

// �����б�
std::vector<ModelPlace> place_list;
int place_list_index = -1; // ��ǰ�б�������Ϊ-1��ʾ��ѡ���κ�ģ�ͣ�����ʾ��꣩

// �������Ͳ��ְ�������С�������
double lastLeftMousePressTime = 0.0;
bool leftMouseButtonPressed = false;
const double leftMousePressInterval = 0.2; // �����ʱ�䣬��λΪ��

double lastKeyPressTime = 0.0;
bool keyPressActive = false;
const double keyPressInterval = 0.2; // �������ʱ�䣬��λΪ��


// ������
// --------------------
int main()
{
    // ��ʼ��������glfw
    // --------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // ʹ�ú���ģʽ

    // ����glfw����
    scrWidth = SCR_WIDTH;
    scrHeight = SCR_HEIGHT;
    GLFWwindow* window = glfwCreateWindow(scrWidth, scrHeight, "OpenGL_final_TerrainEditor", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "����GLFW����ʧ��" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // �����ڵ���������Ϊ��ǰ�̵߳���������

    // ע��ص�
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // ֡�����С�ı�ص�
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // ����Ӣ�����뷨
    SetEnglishInputLanguage();

    // ��������OpenGL����ָ��
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "��ʼ��GLADʧ��" << std::endl;
        return -1;
    }

    // ����ȫ��OpenGL״̬
    glEnable(GL_DEPTH_TEST); // ������Ȳ���

    // ����ģ����ɫ��
    // --------------------
    std::string vertexShaderSource = ReadFile("resources/shader/vertexShader.vert");
    std::string fragmentShaderSource = ReadFile("resources/shader/fragmentShader.frag");

    Shader ourShader(vertexShaderSource.c_str(), fragmentShaderSource.c_str());

    // ��ʼ����պ� 
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

    // ���ɵ�������
    // --------------------

    std::wstring defaultFilePath = GetAbsolutePathFromRelative(L"resources\\image\\height_map");
    std::wstring heightMapPath = BrowseForFile(NULL, defaultFilePath);
    if (heightMapPath.empty()) {
        std::cerr << "No height map selected." << std::endl;
    }

    // ��ȡ�����ļ�·��
    std::wstring texturePath = GetAbsolutePathFromRelative(L"resources\\image\\Grass_02.png"); // ��Ϊʹ��Ĭ�ϲݵ���ͼ
    /*
    defaultFilePath = GetAbsolutePathFromRelative(L"resources\\image");
    std::wstring texturePath = BrowseForFile(NULL, defaultFilePath);
    if (texturePath.empty()) {
        std::cerr << "No texture selected." << std::endl;
    }
    */

    // ʹ��std::filesystem::path�����ַ�·��ת��Ϊ��ͨ�ַ���
    std::filesystem::path heightMap(heightMapPath);
    std::filesystem::path tex(texturePath);

    std::cout << tex.string() << std::endl;

    // ���ص���
    Terrain terrain;
    terrain.load(heightMap.string(), 80, 80, 40, 10, tex.string());



    // ��Ⱦѭ��
    // --------------------
    while (!glfwWindowShouldClose(window))
    {
        // ÿ֡ʱ���߼�
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // �������봦��
        processInput(window);

        // ��Ⱦ
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // ���������ɫ
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // �����ɫ����������Ȼ�����

        // ������ɫ��
        ourShader.use();


        // ��ͶӰ���󴫵ݸ���ɫ��
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        ourShader.setMat4("projection", projection);

        // ���/��ͼ�任
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        // �����ӵ�λ��
        glm::vec3 viewPos = camera.GetPosition();  // ��ȡ�����λ��
        ourShader.setVec3("viewPos", viewPos);


        // ���ù�Դ��λ�ú���ɫ
        ourShader.setVec3("lightPos", lightPos);
        ourShader.setVec3("lightColor", lightColor);

        // ���ù�Դ˥��ϵ��
        ourShader.setFloat("Kc", Kc);
        ourShader.setFloat("Kl", Kl);
        ourShader.setFloat("Kq", Kq);

        // ��Ⱦ����
        terrain.render(ourShader);

        // ��Ⱦ��պ�
        skybox.render(projection, camera.GetViewMatrix());

        // ��ȡ���λ��
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // ��������
        glm::vec3 rayDirection = getRayDirection(static_cast<float>(xpos), static_cast<float>(ypos), camera.GetViewMatrix(), projection, SCR_WIDTH, SCR_HEIGHT);

        // ��Ⱦ��������ģ��
        if (place_list_index != -1 && place_list_index < static_cast<int>(place_list.size())) {
            ModelPlace& currentModelPlace = place_list[place_list_index];
            glm::vec3 intersectionPoint = raycastTerrainIntersection(camera.GetPosition(), rayDirection, terrain);

            // ��ȡģ�Ͳ�׼����Ⱦ
            std::shared_ptr<Model> currentModel = currentModelPlace.getModel();
            if (currentModel) {
                // ����ģ�͵ײ���������ƫ����
                float liftAmount = -currentModel->getMinY(); // ��ȡģ�͵����Y���꣬��ȡ��ֵ��Ϊ������

                // ��ȡģ�͵����ž���
                glm::mat4 scaleMatrix = currentModel->getScaleMatrix();

                // ʹ�� Y ����������ӵȱ������� liftAmount
                float scaleY = glm::length(glm::vec3(scaleMatrix[1]));
                liftAmount *= scaleY;

                // �����任������Ӧ�����ţ��ٽ�ģ���ƶ�������λ�ã�Ȼ������ģ��ʹ��ײ�λ�ڵ����ϣ����Ӧ����ת
                glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), intersectionPoint);
                modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, liftAmount, 0.0f));
                modelMatrix = modelMatrix * placeRotationMatrix;
                placeMatrix = modelMatrix * scaleMatrix; // Ӧ������

                // ����ģ�͵ı任����
                currentModel->setModelMatrix(placeMatrix);

                // ��Ⱦģ��
                currentModel->render(ourShader);
            }
        }

        // ��Ⱦ���з��õ�ģ��ʵ��
        for (const auto& place : place_list) {
            place.renderAll(ourShader);
        }

        // glfw: ��������������ѯIO�¼�����������/�ͷţ�����ƶ��ȣ�
        glfwSwapBuffers(window);
        glfwPollEvents();

        // debug
        /*
        // ��ӡ��Դ�����������
        static float printInterval = 1.0f; // ��ӡ���ʱ�䣨�룩
        static float lastPrintTime = 0.0f;
        if (currentFrame - lastPrintTime >= printInterval)
        {
            // ��ӡ��Դ�����������
            std::cout << "Light Position: ("
                << std::fixed << std::setprecision(2) << lightPos.x << ", "
                << std::fixed << std::setprecision(2) << lightPos.y << ", "
                << std::fixed << std::setprecision(2) << lightPos.z << ")" << std::endl;
            std::cout << "Camera Position: ("
                << std::fixed << std::setprecision(2) << viewPos.x << ", "
                << std::fixed << std::setprecision(2) << viewPos.y << ", "
                << std::fixed << std::setprecision(2) << viewPos.z << ")" << std::endl;

            // �����ϴδ�ӡʱ��
            lastPrintTime = currentFrame;
        }
        /* */
    }


    glfwTerminate();
    return 0;
}

// �ص������봦��
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    // ������ƶ�
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime); // �������W������ǰ�ƶ�
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime); // �������S��������ƶ�
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime); // �������A���������ƶ�
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime); // �������D���������ƶ�
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime); // �������Q���������ƶ�
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime); // �������E���������ƶ�

    // ��ȡ��ǰʱ��
    double currentTime = glfwGetTime();

    if (!keyPressActive && (currentTime - lastKeyPressTime >= keyPressInterval))
    {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            // �л�����һ��ģ��
            ++place_list_index;
            if (place_list_index >= static_cast<int>(place_list.size())) {
                place_list_index = -1; // Խ���-1��ʼ
            }

            // ��ӡ��ǰѡ���ģ������
            if (place_list_index >= 0 && place_list_index < static_cast<int>(place_list.size())) {
                std::cout << "Current model: " << place_list[place_list_index].getName() << std::endl;
            }
            else {
                std::cout << "No model selected." << std::endl;
            }

            // ���°���״̬��ʱ��
            lastKeyPressTime = currentTime;
            keyPressActive = true;
            placeRotationMatrix = glm::mat4(1.0f); // ���÷��ýǶ�
        }

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            // �л�����һ��ģ��
            --place_list_index;
            if (place_list_index < -1) {
                place_list_index = static_cast<int>(place_list.size()) - 1; // Խ������һ��Ԫ�ؿ�ʼ
            }

            // ��ӡ��ǰѡ���ģ������
            if (place_list_index >= 0 && place_list_index < static_cast<int>(place_list.size())) {
                std::cout << "Current model: " << place_list[place_list_index].getName() << std::endl;
            }
            else {
                std::cout << "No model selected." << std::endl;
            }

            // ���°���״̬��ʱ��
            lastKeyPressTime = currentTime;
            keyPressActive = true;
            placeRotationMatrix = glm::mat4(1.0f); // ���÷��ýǶ�
        }

        // ��Z������һ�����õ�ģ��
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            if (place_list_index >= 0 && place_list_index < static_cast<int>(place_list.size())) {
                place_list[place_list_index].removeLastModelMatrix();  // ������һ��ģ��
                std::cout << "Last model removed." << std::endl;
            }
            lastKeyPressTime = currentTime;
            keyPressActive = true;  // ��ֹ�ظ�����
        }

        // ��M���뵥��ģ��
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            std::wstring defaultFolderPath = GetAbsolutePathFromRelative(L"resources\\model");
            std::wstring folderPath = BrowseForFolder(NULL, defaultFolderPath);
            if (!folderPath.empty())
            {
                // ʹ��std::filesystem::path�����ַ�·��ת��Ϊ��ͨ�ַ���
                std::filesystem::path selectedPath(folderPath);
                std::string dir_path = selectedPath.string();

                // ��ȡ�ļ�������
                std::string model_name = selectedPath.filename().string();

                // ����ģ��
                std::shared_ptr<Model> newModel = std::make_shared<Model>(model_name);
                if (newModel->load(dir_path)) {
                    // �����µ� ModelPlace ����ӵ��б�
                    ModelPlace newPlace(newModel);
                    place_list.push_back(newPlace);

                    std::cout << "Model loaded from: " << model_name << std::endl;

                }
                else {
                    std::cerr << "Failed to load model from: " << model_name << std::endl;
                }
            }
        }

        // ��N��������ģ��
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            // ��ȡ��Դ·��
            std::wstring defaultFolderPath = GetAbsolutePathFromRelative(L"resources\\model");

            // ����Ĭ��·���µ�������Ŀ¼����ģ���ļ��У�
            for (const auto& entry : std::filesystem::directory_iterator(defaultFolderPath)) {
                if (entry.is_directory()) {
                    // ʹ�� std::filesystem::path �����ַ�·��ת��Ϊ��ͨ�ַ���
                    std::filesystem::path selectedPath(entry.path());
                    std::string dir_path = selectedPath.string();

                    // ��ȡ�ļ���������Ϊģ������
                    std::string model_name = selectedPath.filename().string();

                    // ����ģ��
                    std::shared_ptr<Model> newModel = std::make_shared<Model>(model_name);
                    if (newModel->load(dir_path)) {
                        // �����µ� ModelPlace ����ӵ��б�
                        ModelPlace newPlace(newModel);
                        place_list.push_back(newPlace);

                        std::cout << "Model loaded from: " << model_name << std::endl;
                    }
                    else {
                        std::cerr << "Failed to load model from: " << model_name << std::endl;
                    }
                }
            }

            // ȷ��ֻ�ڵ�һ�ΰ��� N ��ʱִ����������
            keyPressActive = true;
        }



    }

    // ����Ƿ��Ѿ����˰������ʱ�䣬���ð���״̬
    if (keyPressActive && (currentTime - lastKeyPressTime >= keyPressInterval)) {
        keyPressActive = false;
    }

    // ��F��G��תģ��
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


    // ��C��V����ģ��
    if (place_list_index != -1 && place_list_index < static_cast<int>(place_list.size())) {
        ModelPlace& currentModelPlace = place_list[place_list_index];

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !keyPressActive) {
            // ��������
            currentModelPlace.increaseScale(place_scale_speed* deltaTime); 
            keyPressActive = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !keyPressActive) {
            // ��������
            currentModelPlace.decreaseScale(place_scale_speed* deltaTime); 
            keyPressActive = true;
        }
    }

    // debug
    // ��������λ��
    const float lightPosStep = 0.1f;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)//  ���� I �������ӹ����� y �᷽���ϵ�λ�ã��������ƶ���Դ��ÿ������ lightPosStep������Ϊ 1.0f����
        lightPos.y += lightPosStep;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)//  ���� K �������ٹ����� y �᷽���ϵ�λ�ã�ʹ��Դ�����ƶ���ÿ�μ��� lightPosStep
        lightPos.y -= lightPosStep;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)//  ���� J �������ٹ����� x �᷽���ϵ�λ�ã�ʹ��Դ�����ƶ���ÿ�μ��� lightPosStep
        lightPos.x -= lightPosStep;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)//  ���� L �������ӹ����� x �᷽���ϵ�λ�ã�ʹ��Դ�����ƶ���ÿ������ lightPosStep
        lightPos.x += lightPosStep;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)//  ���� U �������ӹ����� z �᷽���ϵ�λ�ã�ʹ��Դ��ǰ�ƶ���ÿ������ lightPosStep
        lightPos.z += lightPosStep;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)//  ���� O �������ٹ����� z �᷽���ϵ�λ�ã�ʹ��Դ����ƶ���ÿ�μ��� lightPosStep
        lightPos.z -= lightPosStep;

    // ����������ɫ
    const float colorStep = 0.001f;
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)//  ���� 1 �������ӹ�����ɫ�ĺ�ɫ������ÿ������ colorStep������Ϊ 0.1f����ʹ������ɫ��ƫ���ɫ��
        lightColor.r += colorStep;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)//  ���� 2 �������ٹ�����ɫ�ĺ�ɫ������ʹ������ɫ���ٺ�ɫ�ɷ֣�ÿ�μ��� colorStep
        lightColor.r -= colorStep;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)//  ���� 3 �������ӹ�����ɫ����ɫ������ÿ������ colorStep��ʹ������ɫ��ƫ����ɫ
        lightColor.g += colorStep;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)//  ���� 4 �������ٹ�����ɫ����ɫ������ʹ������ɫ������ɫ�ɷ֣�ÿ�μ��� colorStep
        lightColor.g -= colorStep;
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)//  ���� 5 �������ӹ�����ɫ����ɫ������ÿ������ colorStep��ʹ������ɫ��ƫ����ɫ
        lightColor.b += colorStep;
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)//  ���� 6 �������ٹ�����ɫ����ɫ������ʹ������ɫ������ɫ�ɷ֣�ÿ�μ��� colorStep
        lightColor.b -= colorStep;

    // ��������˥��ϵ��
    const float attenuationStep = 0.0001f;
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)//  ���� 7 �������ӳ���˥��ϵ�� Kc��ÿ������ attenuationStep������Ϊ 0.0001f����Ӱ����������˥���ĳ̶ȡ�
        Kc += attenuationStep;
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)//  ���� 8 �������ٳ���˥��ϵ�� Kc��ʹ����˥���̶Ƚ��ͣ�ÿ�μ��� attenuationStep
        Kc -= attenuationStep;

    float Kl_speed = 0.1;
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)//  ���� 9 ������������˥��ϵ�� Kl��ÿ������ attenuationStep����һ���������������˥�������Բ���
        Kl += attenuationStep * Kl_speed;
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)//  ���� 0 ������������˥��ϵ�� Kl�����͹�������˥��Ч����ÿ�μ��� attenuationStep
        Kl -= attenuationStep * Kl_speed;

    float Kq_speed = 0.0001;
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)//  ���� - �������Ӷ���˥��ϵ�� Kq��ÿ������ attenuationStep��Ӱ����������˥���Ķ��η�����
        Kq += attenuationStep * Kq_speed;
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)//  ���� = �������ٶ���˥��ϵ�� Kq���������ն���˥��Ч����ÿ�μ��� attenuationStep
        Kq -= attenuationStep * Kq_speed;

    // ��ӡ���ղ���
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

    if (rightMouseButtonPressed) // �Ҽ�����ʱ������ת�ӽ�
    {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false; // ��һ�λ�ȡ���λ�ú�����Ϊfalse
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // ��Ϊy�����Ǵӵײ�����������������ȡ��

        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset); // ��������ƶ�
    }
}

// ��갴���ص�����
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        rightMouseButtonPressed = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        rightMouseButtonPressed = false;
        firstMouse = true; // �ͷ��Ҽ�ʱ����firstMouse
    }

    // �������������
    double currentTime = glfwGetTime();
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !leftMouseButtonPressed &&
        (currentTime - lastLeftMousePressTime >= leftMousePressInterval)) {
        leftMouseButtonPressed = true;
        lastLeftMousePressTime = currentTime;

        // �����ѡ���ģ�ͣ��򱣴��䵱ǰλ�þ���
        if (place_list_index != -1 && place_list_index < static_cast<int>(place_list.size())) {
            ModelPlace& currentModelPlace = place_list[place_list_index];
            currentModelPlace.addModelMatrix(placeMatrix); // ����ǰplaceMatrix��ӵ�ModelPlace��λ�þ���������

            std::cout << "Model placed" << std::endl;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        leftMouseButtonPressed = false;
    }
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(2 * static_cast<float>(yoffset)); // ���������ֹ���
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // ��ֹ���Ϊ0
    if (width > 0 && height > 0) {
        scrWidth = width;
        scrHeight = height;
        glViewport(0, 0, width, height); // �����ӿڴ�С
    }
}


