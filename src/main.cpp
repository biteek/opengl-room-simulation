#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

glm::vec3 cameraPos   = glm::vec3(0.0f, -0.5f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

bool firstMouse = true;
float yaw   = -90.0f;
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0f / 2.0;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool isDoorOpen = false;
bool eKeyPressed = false;
float currentDoorAngle = 0.0f;

int lightMode = 0;
bool tKeyPressed = false;

struct ModelData {
    unsigned int VAO, VBO;
    int vertexCount;
    unsigned int textureID;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 5.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (!eKeyPressed) {
            isDoorOpen = !isDoorOpen;
            eKeyPressed = true;
        }
    } else {
        eKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        if (!tKeyPressed) {
            lightMode = (lightMode + 1) % 4;
            tKeyPressed = true;
        }
    } else {
        tKeyPressed = false;
    }
}

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);
    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        std::cout << "Shader Error: " << infoLog << std::endl;
    }
    return id;
}

unsigned int loadTexture(char const * path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    
    if (data) {
        GLenum format = (nrComponents == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

ModelData loadModel(const char* objPath, const char* texPath) {
    ModelData data = {0, 0, 0, 0};
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objPath)) {
        std::cout << "OBJ Error: " << warn << err << std::endl;
        return data;
    }

    std::vector<float> vertices;
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);

            if (index.normal_index >= 0) {
                vertices.push_back(attrib.normals[3 * index.normal_index + 0]);
                vertices.push_back(attrib.normals[3 * index.normal_index + 1]);
                vertices.push_back(attrib.normals[3 * index.normal_index + 2]);
            } else {
                vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
            }

            if (index.texcoord_index >= 0) {
                vertices.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
                vertices.push_back(attrib.texcoords[2 * index.texcoord_index + 1]);
            } else {
                vertices.push_back(0.0f); vertices.push_back(0.0f);
            }
        }
    }

    glGenVertexArrays(1, &data.VAO);
    glGenBuffers(1, &data.VBO);
    glBindVertexArray(data.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    data.vertexCount = vertices.size() / 8;
    data.textureID = loadTexture(texPath);
    std::cout << "Loaded model: " << objPath << " | Vertices: " << data.vertexCount << std::endl;

    return data;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1920, 1440, "Projekt GK", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);

    const char* vShader = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aNormal;\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "out vec3 FragPos;\n"
        "out vec3 Normal;\n"
        "out vec2 TexCoords;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main() {\n"
        "   FragPos = vec3(model * vec4(aPos, 1.0));\n"
        "   Normal = mat3(transpose(inverse(model))) * aNormal;\n"
        "   TexCoords = aTexCoord;\n"
        "   gl_Position = projection * view * vec4(FragPos, 1.0);\n"
        "}\0";

    const char* fShader = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 FragPos;\n"
        "in vec3 Normal;\n"
        "in vec2 TexCoords;\n"
        "uniform sampler2D texture1;\n"
        "uniform vec3 viewPos;\n"
        "uniform vec3 pointLightPos;\n"
        "uniform vec3 pointLightColor;\n"
        "uniform vec3 spotLightPos;\n"
        "uniform vec3 spotLightDir;\n"
        "uniform vec3 spotLightColor;\n"
        "uniform float spotCutOff;\n"
        "uniform float spotOuterCutOff;\n"
        "void main() {\n"
        "   vec3 norm = normalize(Normal);\n"
        "   vec3 viewDir = normalize(viewPos - FragPos);\n"
        "   if (dot(norm, viewDir) < 0.0) norm = -norm;\n"
        "   vec3 texColor = texture(texture1, TexCoords).rgb;\n"
        
        "   vec3 lightDir1 = normalize(pointLightPos - FragPos);\n"
        "   float diff1 = max(dot(norm, lightDir1), 0.0);\n"
        "   vec3 reflectDir1 = reflect(-lightDir1, norm);\n"
        "   float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), 32.0);\n"
        "   float distance1 = length(pointLightPos - FragPos);\n"
        "   float attenuation1 = 1.0 / (1.0 + 0.045 * distance1 + 0.0075 * (distance1 * distance1));\n"
        "   vec3 ambient1 = 0.35 * pointLightColor * texColor;\n"
        "   vec3 diffuse1 = diff1 * pointLightColor * texColor;\n"
        "   vec3 specular1 = spec1 * pointLightColor;\n"
        "   vec3 pointResult = (ambient1 + diffuse1 + specular1) * attenuation1;\n"

        "   vec3 lightDir2 = normalize(spotLightPos - FragPos);\n"
        "   float diff2 = max(dot(norm, lightDir2), 0.0);\n"
        "   vec3 reflectDir2 = reflect(-lightDir2, norm);\n"
        "   float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), 32.0);\n"
        "   float theta = dot(lightDir2, normalize(-spotLightDir));\n"
        "   float epsilon = spotCutOff - spotOuterCutOff;\n"
        "   float intensity = clamp((theta - spotOuterCutOff) / epsilon, 0.0, 1.0);\n"
        "   float distance2 = length(spotLightPos - FragPos);\n"
        "   float attenuation2 = 1.0 / (1.0 + 0.09 * distance2 + 0.032 * (distance2 * distance2));\n"
        "   vec3 diffuse2 = diff2 * spotLightColor * texColor;\n"
        "   vec3 specular2 = spec2 * spotLightColor;\n"
        "   vec3 spotResult = (diffuse2 + specular2) * attenuation2 * intensity;\n"

        "   FragColor = vec4(pointResult + spotResult, 1.0);\n"
        "}\0";

    unsigned int vs = compileShader(GL_VERTEX_SHADER, vShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fShader);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    float roomVertices[] = {
        -5.0f, -2.0f, -5.0f,  0.0f,  1.0f,  0.0f,   0.0f,  0.0f,
         5.0f, -2.0f, -5.0f,  0.0f,  1.0f,  0.0f,  10.0f,  0.0f,
         5.0f, -2.0f,  5.0f,  0.0f,  1.0f,  0.0f,  10.0f, 10.0f,
         5.0f, -2.0f,  5.0f,  0.0f,  1.0f,  0.0f,  10.0f, 10.0f,
        -5.0f, -2.0f,  5.0f,  0.0f,  1.0f,  0.0f,   0.0f, 10.0f,
        -5.0f, -2.0f, -5.0f,  0.0f,  1.0f,  0.0f,   0.0f,  0.0f,

        -5.0f,  3.0f, -5.0f,  0.0f, -1.0f,  0.0f,   0.0f,  0.0f,
         5.0f,  3.0f,  5.0f,  0.0f, -1.0f,  0.0f,  10.0f, 10.0f,
         5.0f,  3.0f, -5.0f,  0.0f, -1.0f,  0.0f,  10.0f,  0.0f,
         5.0f,  3.0f,  5.0f,  0.0f, -1.0f,  0.0f,  10.0f, 10.0f,
        -5.0f,  3.0f, -5.0f,  0.0f, -1.0f,  0.0f,   0.0f,  0.0f,
        -5.0f,  3.0f,  5.0f,  0.0f, -1.0f,  0.0f,   0.0f, 10.0f,

        -5.0f, -2.0f, -5.0f,  0.0f,  0.0f,  1.0f,   0.0f,  0.0f,
        -5.0f,  3.0f, -5.0f,  0.0f,  0.0f,  1.0f,   0.0f,  5.0f,
         5.0f,  3.0f, -5.0f,  0.0f,  0.0f,  1.0f,  10.0f,  5.0f,
         5.0f,  3.0f, -5.0f,  0.0f,  0.0f,  1.0f,  10.0f,  5.0f,
         5.0f, -2.0f, -5.0f,  0.0f,  0.0f,  1.0f,  10.0f,  0.0f,
        -5.0f, -2.0f, -5.0f,  0.0f,  0.0f,  1.0f,   0.0f,  0.0f,
        
        -5.0f, -2.0f, -5.0f,  1.0f,  0.0f,  0.0f,   0.0f,  0.0f,
        -5.0f, -2.0f, -0.85f, 1.0f,  0.0f,  0.0f,   4.15f, 0.0f,
        -5.0f,  3.0f, -0.85f, 1.0f,  0.0f,  0.0f,   4.15f, 5.0f,
        -5.0f,  3.0f, -0.85f, 1.0f,  0.0f,  0.0f,   4.15f, 5.0f,
        -5.0f,  3.0f, -5.0f,  1.0f,  0.0f,  0.0f,   0.0f,  5.0f,
        -5.0f, -2.0f, -5.0f,  1.0f,  0.0f,  0.0f,   0.0f,  0.0f,

        -5.0f, -2.0f,  0.85f, 1.0f,  0.0f,  0.0f,   5.85f, 0.0f,
        -5.0f, -2.0f,  5.0f,  1.0f,  0.0f,  0.0f,   10.0f, 0.0f,
        -5.0f,  3.0f,  5.0f,  1.0f,  0.0f,  0.0f,   10.0f, 5.0f,
        -5.0f,  3.0f,  5.0f,  1.0f,  0.0f,  0.0f,   10.0f, 5.0f,
        -5.0f,  3.0f,  0.85f, 1.0f,  0.0f,  0.0f,   5.85f, 5.0f,
        -5.0f, -2.0f,  0.85f, 1.0f,  0.0f,  0.0f,   5.85f, 0.0f,

        -5.0f,  2.05f, -0.85f, 1.0f,  0.0f,  0.0f,   4.15f, 4.05f,
        -5.0f,  2.05f,  0.85f, 1.0f,  0.0f,  0.0f,   5.85f, 4.05f,
        -5.0f,  3.0f,   0.85f, 1.0f,  0.0f,  0.0f,   5.85f, 5.0f,
        -5.0f,  3.0f,   0.85f, 1.0f,  0.0f,  0.0f,   5.85f, 5.0f,
        -5.0f,  3.0f,  -0.85f, 1.0f,  0.0f,  0.0f,   4.15f, 5.0f,
        -5.0f,  2.05f, -0.85f, 1.0f,  0.0f,  0.0f,   4.15f, 4.05f,
        
         5.0f, -2.0f, -5.0f, -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,
         5.0f,  3.0f,  5.0f, -1.0f,  0.0f,  0.0f,  10.0f,  5.0f,
         5.0f, -2.0f,  5.0f, -1.0f,  0.0f,  0.0f,  10.0f,  0.0f,
         5.0f,  3.0f,  5.0f, -1.0f,  0.0f,  0.0f,  10.0f,  5.0f,
         5.0f, -2.0f, -5.0f, -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,
         5.0f,  3.0f, -5.0f, -1.0f,  0.0f,  0.0f,   0.0f,  5.0f,

        -5.0f, -2.0f,  5.0f,  0.0f,  0.0f, -1.0f,   0.0f,  0.0f,
         5.0f, -2.0f,  5.0f,  0.0f,  0.0f, -1.0f,  10.0f,  0.0f,
         5.0f,  3.0f,  5.0f,  0.0f,  0.0f, -1.0f,  10.0f,  5.0f,
         5.0f,  3.0f,  5.0f,  0.0f,  0.0f, -1.0f,  10.0f,  5.0f,
        -5.0f,  3.0f,  5.0f,  0.0f,  0.0f, -1.0f,   0.0f,  5.0f,
        -5.0f, -2.0f,  5.0f,  0.0f,  0.0f, -1.0f,   0.0f,  0.0f
    };

    unsigned int roomVBO, roomVAO;
    glGenVertexArrays(1, &roomVAO);
    glGenBuffers(1, &roomVBO);
    glBindVertexArray(roomVAO);
    glBindBuffer(GL_ARRAY_BUFFER, roomVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roomVertices), roomVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    std::string basePath = "objects/";

    unsigned int floorTex   = loadTexture((basePath + "room/floor.jpg").c_str());
    unsigned int ceilingTex = loadTexture((basePath + "room/ceiling.jpg").c_str());
    unsigned int wallsTex   = loadTexture((basePath + "room/walls.jpg").c_str());

    ModelData table  = loadModel((basePath + "table/table.obj").c_str(),   (basePath + "table/texture-table.jpg").c_str());
    ModelData lamp   = loadModel((basePath + "lamp/lamp.obj").c_str(),     (basePath + "lamp/texture-lamp.jpg").c_str());
    ModelData chair  = loadModel((basePath + "chair/chair.obj").c_str(),   (basePath + "chair/texture-chair.jpg").c_str());
    ModelData carpet = loadModel((basePath + "carpet/carpet.obj").c_str(), (basePath + "carpet/texture-carpet.jpg").c_str());
    ModelData door   = loadModel((basePath + "door/door.obj").c_str(),     (basePath + "door/texture-door.jpg").c_str());
    ModelData fan    = loadModel((basePath + "fan/fan.obj").c_str(),       (basePath + "fan/texture-fan.jpg").c_str());

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    glUniform3f(glGetUniformLocation(shaderProgram, "pointLightPos"), 0.0f, 1.15f, 0.0f);
    glUniform3f(glGetUniformLocation(shaderProgram, "spotLightPos"), 0.0f, 3.15f, -3.8f);
    glUniform3f(glGetUniformLocation(shaderProgram, "spotLightDir"), 0.0f, -1.0f, 0.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotCutOff"), glm::cos(glm::radians(20.0f)));
    glUniform1f(glGetUniformLocation(shaderProgram, "spotOuterCutOff"), glm::cos(glm::radians(25.0f)));

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        
        glm::vec3 pointColor = (lightMode == 0 || lightMode == 2) ? glm::vec3(1.0f, 0.95f, 0.85f) : glm::vec3(0.0f);
        glm::vec3 spotColor  = (lightMode == 0 || lightMode == 1) ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(0.0f);

        glUniform3fv(glGetUniformLocation(shaderProgram, "pointLightColor"), 1, glm::value_ptr(pointColor));
        glUniform3fv(glGetUniformLocation(shaderProgram, "spotLightColor"), 1, glm::value_ptr(spotColor));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1440.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

        glm::mat4 modelRoom = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelRoom));
        
        glBindVertexArray(roomVAO);
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D, floorTex);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindTexture(GL_TEXTURE_2D, ceilingTex);
        glDrawArrays(GL_TRIANGLES, 6, 6);

        glBindTexture(GL_TEXTURE_2D, wallsTex);
        glDrawArrays(GL_TRIANGLES, 12, 36); 

        if (carpet.vertexCount > 0) {
            glm::mat4 modelObj = glm::mat4(1.0f);
            modelObj = glm::translate(modelObj, glm::vec3(0.0f, -2.00f, 0.0f)); 
            modelObj = glm::rotate(modelObj, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            modelObj = glm::scale(modelObj, glm::vec3(0.04f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObj));
            glBindTexture(GL_TEXTURE_2D, carpet.textureID);
            glBindVertexArray(carpet.VAO);
            glDrawArrays(GL_TRIANGLES, 0, carpet.vertexCount);
        }

        if (table.vertexCount > 0) {
            glm::mat4 modelObj = glm::mat4(1.0f);
            modelObj = glm::translate(modelObj, glm::vec3(0.0f, -1.60f, -3.8f)); 
            modelObj = glm::rotate(modelObj, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelObj = glm::scale(modelObj, glm::vec3(2.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObj));
            glBindTexture(GL_TEXTURE_2D, table.textureID);
            glBindVertexArray(table.VAO);
            glDrawArrays(GL_TRIANGLES, 0, table.vertexCount);
        }

        if (chair.vertexCount > 0) {
            glm::mat4 modelObj = glm::mat4(1.0f);
            modelObj = glm::translate(modelObj, glm::vec3(-2.0f, -1.98f, -3.8f)); 
            modelObj = glm::rotate(modelObj, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelObj = glm::scale(modelObj, glm::vec3(2.6f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObj));
            glBindTexture(GL_TEXTURE_2D, chair.textureID);
            glBindVertexArray(chair.VAO);
            glDrawArrays(GL_TRIANGLES, 0, chair.vertexCount);
        }

        if (lamp.vertexCount > 0) {
            glm::mat4 modelObj = glm::mat4(1.0f);
            modelObj = glm::translate(modelObj, glm::vec3(5.18f, -3.0f, -2.5f)); 
            modelObj = glm::scale(modelObj, glm::vec3(0.002f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObj));
            glBindTexture(GL_TEXTURE_2D, lamp.textureID);
            glBindVertexArray(lamp.VAO);
            glDrawArrays(GL_TRIANGLES, 0, lamp.vertexCount);
        }

        float targetAngle = isDoorOpen ? -90.0f : 0.0f;
        currentDoorAngle += (targetAngle - currentDoorAngle) * 5.0f * deltaTime;
        
        float hingeOffsetZ = 0.85f; 

        if (door.vertexCount > 0) {
            glm::mat4 modelObj = glm::mat4(1.0f);
            
            modelObj = glm::translate(modelObj, glm::vec3(-4.95f, -2.0f, hingeOffsetZ)); 
            modelObj = glm::rotate(modelObj, glm::radians(currentDoorAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            modelObj = glm::translate(modelObj, glm::vec3(0.0f, 0.0f, -hingeOffsetZ));
            
            modelObj = glm::rotate(modelObj, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            modelObj = glm::rotate(modelObj, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelObj = glm::scale(modelObj, glm::vec3(0.02f));
            
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObj));
            glBindTexture(GL_TEXTURE_2D, door.textureID);
            glBindVertexArray(door.VAO);
            glDrawArrays(GL_TRIANGLES, 0, door.vertexCount);
        }

        if (fan.vertexCount > 0) {
            glm::mat4 modelObj = glm::mat4(1.0f);
            modelObj = glm::translate(modelObj, glm::vec3(0.0f, 1.55f, -3.6f)); 
            modelObj = glm::rotate(modelObj, (float)glfwGetTime() * -3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            modelObj = glm::rotate(modelObj, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            modelObj = glm::scale(modelObj, glm::vec3(0.012f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObj));
            glBindTexture(GL_TEXTURE_2D, fan.textureID);
            glBindVertexArray(fan.VAO);
            glDrawArrays(GL_TRIANGLES, 0, fan.vertexCount);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}