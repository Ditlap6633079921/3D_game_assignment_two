// Submission copy.
// To run, place this file inside:
//   LearnOpenGL-master/src/2.lighting/6.multiple_lights_task2_glass/
// and build the target:
//   2.lighting__6.multiple_lights_task2_glass

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void processInput(GLFWwindow* window);

static const unsigned int SCR_WIDTH = 1000;
static const unsigned int SCR_HEIGHT = 700;

static Camera camera(glm::vec3(0.0f, 0.2f, 3.2f));
static float lastX = SCR_WIDTH / 2.0f;
static float lastY = SCR_HEIGHT / 2.0f;
static bool firstMouse = true;

static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

static float gYaw = 0.0f;
static float gPitch = 0.0f;

struct Particle
{
    glm::vec3 p;
    glm::vec3 v;
};

static void appendCylinderSides(std::vector<float>& out, float radius, float height, int segments)
{
    const float halfH = height * 0.5f;
    const float twoPi = 6.28318530718f;
    auto push = [&](const glm::vec3& pos, const glm::vec3& nrm) {
        out.push_back(pos.x); out.push_back(pos.y); out.push_back(pos.z);
        out.push_back(nrm.x); out.push_back(nrm.y); out.push_back(nrm.z);
    };
    for (int i = 0; i < segments; ++i)
    {
        float a0 = (i / (float)segments) * twoPi;
        float a1 = ((i + 1) / (float)segments) * twoPi;
        glm::vec3 n0(std::cos(a0), 0.0f, std::sin(a0));
        glm::vec3 n1(std::cos(a1), 0.0f, std::sin(a1));
        glm::vec3 p0(radius * n0.x, -halfH, radius * n0.z);
        glm::vec3 p1(radius * n1.x, -halfH, radius * n1.z);
        glm::vec3 p2(radius * n1.x, +halfH, radius * n1.z);
        glm::vec3 p3(radius * n0.x, +halfH, radius * n0.z);
        push(p0, n0); push(p1, n1); push(p2, n1);
        push(p0, n0); push(p2, n1); push(p3, n0);
    }
}

static glm::mat3 rotationFromYawPitch(float yaw, float pitch)
{
    glm::mat4 r(1.0f);
    r = glm::rotate(r, yaw, glm::vec3(0.f, 1.f, 0.f));
    r = glm::rotate(r, pitch, glm::vec3(1.f, 0.f, 0.f));
    return glm::mat3(r);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Task2 - Kinetic Glass (multiple lights)", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::string vsPath = FileSystem::getPath("src/2.lighting/6.multiple_lights_task2_glass/task2_glass.vs");
    std::string fsPath = FileSystem::getPath("src/2.lighting/6.multiple_lights_task2_glass/task2_glass.fs");
    std::string lvsPath = FileSystem::getPath("src/2.lighting/6.multiple_lights_task2_glass/task2_light_cube.vs");
    std::string lfsPath = FileSystem::getPath("src/2.lighting/6.multiple_lights_task2_glass/task2_light_cube.fs");

    Shader lightingShader(vsPath.c_str(), fsPath.c_str());
    Shader lightCubeShader(lvsPath.c_str(), lfsPath.c_str());

    float cubeVerts[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    unsigned int cubeVBO = 0, cubeVAO = 0;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int lightVAO = 0;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    const float glassRadius = 0.55f;
    const float glassHeight = 1.40f;
    const int glassSeg = 64;
    std::vector<float> cyl;
    appendCylinderSides(cyl, glassRadius, glassHeight, glassSeg);

    unsigned int cylVBO = 0, cylVAO = 0;
    glGenVertexArrays(1, &cylVAO);
    glGenBuffers(1, &cylVBO);
    glBindVertexArray(cylVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cylVBO);
    glBufferData(GL_ARRAY_BUFFER, cyl.size() * sizeof(float), cyl.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glm::vec3 pointLightPositions[] = {
        glm::vec3( 1.3f,  0.6f,  1.8f),
        glm::vec3(-1.7f, -0.4f,  1.2f),
        glm::vec3(-1.2f,  1.2f, -1.6f),
        glm::vec3( 1.8f,  0.2f, -1.3f)
    };

    const int particleCount = 380;
    const float particleRadius = 0.035f;
    std::vector<Particle> particles;
    particles.reserve(particleCount);
    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> uni(0.0f, 1.0f);
    for (int i = 0; i < particleCount; ++i)
    {
        float a = uni(rng) * 6.2831853f;
        float rr = std::sqrt(uni(rng)) * (glassRadius - particleRadius * 2.0f) * 0.85f;
        float x = std::cos(a) * rr;
        float z = std::sin(a) * rr;
        float y = (-glassHeight * 0.5f + particleRadius) + uni(rng) * (glassHeight * 0.65f);
        Particle p;
        p.p = glm::vec3(x, y, z);
        p.v = glm::vec3((uni(rng) - 0.5f) * 0.2f, (uni(rng) - 0.5f) * 0.2f, (uni(rng) - 0.5f) * 0.2f);
        particles.push_back(p);
    }

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glm::mat3 R = rotationFromYawPitch(gYaw, gPitch);
        glm::mat3 invR = glm::transpose(R);

        glm::vec3 gWorld(0.0f, -9.8f, 0.0f);
        glm::vec3 gLocal = invR * gWorld;

        const float damp = 0.55f;
        const float wallBounce = 0.35f;
        const float bottom = -glassHeight * 0.5f + particleRadius;
        const float top = +glassHeight * 0.5f - particleRadius;
        const float maxR = glassRadius - particleRadius;

        for (auto& p : particles)
        {
            p.v += gLocal * deltaTime;
            p.v *= std::pow(damp, deltaTime);
            p.p += p.v * deltaTime;

            float r = std::sqrt(p.p.x * p.p.x + p.p.z * p.p.z);
            if (r > maxR)
            {
                glm::vec2 n = glm::normalize(glm::vec2(p.p.x, p.p.z));
                p.p.x = n.x * maxR;
                p.p.z = n.y * maxR;
                glm::vec2 v2(p.v.x, p.v.z);
                v2 = v2 - (1.0f + wallBounce) * glm::dot(v2, n) * n;
                p.v.x = v2.x;
                p.v.z = v2.y;
            }

            if (p.p.y < bottom)
            {
                p.p.y = bottom;
                p.v.y = -p.v.y * 0.25f;
            }
            if (p.p.y > top)
            {
                p.p.y = top;
                p.v.y = -p.v.y * 0.25f;
            }
        }

        glClearColor(0.05f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        lightingShader.use();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setVec3("viewPos", camera.Position);

        lightingShader.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        lightingShader.setVec3("dirLight.ambient",   glm::vec3(0.04f));
        lightingShader.setVec3("dirLight.diffuse",   glm::vec3(0.10f));
        lightingShader.setVec3("dirLight.specular",  glm::vec3(0.12f));

        for (int i = 0; i < 4; i++)
        {
            std::string idx = "pointLights[" + std::to_string(i) + "].";
            lightingShader.setVec3(idx + "position", pointLightPositions[i]);
            lightingShader.setFloat(idx + "constant", 1.0f);
            lightingShader.setFloat(idx + "linear", 0.09f);
            lightingShader.setFloat(idx + "quadratic", 0.032f);
            lightingShader.setVec3(idx + "ambient", glm::vec3(0.03f));
            lightingShader.setVec3(idx + "diffuse", glm::vec3(0.7f));
            lightingShader.setVec3(idx + "specular", glm::vec3(1.0f));
        }

        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09f);
        lightingShader.setFloat("spotLight.quadratic", 0.032f);
        lightingShader.setVec3("spotLight.ambient", glm::vec3(0.0f));
        lightingShader.setVec3("spotLight.diffuse", glm::vec3(0.6f));
        lightingShader.setVec3("spotLight.specular", glm::vec3(0.9f));

        glm::mat4 glassModel(1.0f);
        glassModel = glm::rotate(glassModel, gYaw, glm::vec3(0.f, 1.f, 0.f));
        glassModel = glm::rotate(glassModel, gPitch, glm::vec3(1.f, 0.f, 0.f));

        glBindVertexArray(cubeVAO);
        lightingShader.setFloat("material.shininess", 32.0f);
        lightingShader.setVec3("material.ambient", glm::vec3(0.02f, 0.05f, 0.08f));
        lightingShader.setVec3("material.diffuse", glm::vec3(0.10f, 0.35f, 0.75f));
        lightingShader.setVec3("material.specular", glm::vec3(0.2f));
        lightingShader.setFloat("material.alpha", 0.85f);

        for (const auto& p : particles)
        {
            glm::vec3 worldP = R * p.p;
            glm::mat4 m = glm::translate(glm::mat4(1.0f), worldP);
            m = glm::scale(m, glm::vec3(particleRadius));
            lightingShader.setMat4("model", m);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindVertexArray(cylVAO);
        lightingShader.setFloat("material.shininess", 128.0f);
        lightingShader.setVec3("material.ambient", glm::vec3(0.02f));
        lightingShader.setVec3("material.diffuse", glm::vec3(0.18f, 0.22f, 0.26f));
        lightingShader.setVec3("material.specular", glm::vec3(1.0f));
        lightingShader.setFloat("material.alpha", 0.22f);
        lightingShader.setMat4("model", glassModel);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(cyl.size() / 6));

        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        glBindVertexArray(lightVAO);
        for (int i = 0; i < 4; i++)
        {
            glm::mat4 m(1.0f);
            m = glm::translate(m, pointLightPositions[i]);
            m = glm::scale(m, glm::vec3(0.08f));
            lightCubeShader.setMat4("model", m);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &cylVAO);
    glDeleteBuffers(1, &cylVBO);

    glfwTerminate();
    return 0;
}

static void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    const float rotSpeed = 1.6f;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) gYaw += rotSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) gYaw -= rotSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) gPitch += rotSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) gPitch -= rotSpeed * deltaTime;
    gPitch = std::clamp(gPitch, -1.2f, 1.2f);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

