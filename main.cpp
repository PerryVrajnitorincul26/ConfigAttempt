// // Created by perry on 11/18/22.
//
// Lab6 - Texturi.cpp : Defines the entry point for the console application.
//

#include "Camera.h"
#include "tinyobjTest.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


GLuint VAO, VBO, EBO;
unsigned int VertexShaderId, FragmentShaderId, ProgramId;
GLuint ProjMatrixLocation, ViewMatrixLocation, WorldMatrixLocation;
unsigned int texture1Location, texture2Location;

unsigned int mixValueLocation;
GLfloat mixValue = 1.0f;

Camera *pCamera = nullptr;

// Shader-ul de varfuri / Vertex shader (este privit ca un sir de caractere)
const GLchar *VertexShader =
        {
                "#version 330\n"\
   "layout (location = 0) in vec3 aPos;\n"\
   "layout (location = 1) in vec4 aColor;\n"\
   "layout (location = 2) in vec2 aTexCoord;\n"\
   "out vec4 ourColor;\n"\
   "out vec2 TexCoord;\n"\
   "uniform mat4 ProjMatrix;\n"\
   "uniform mat4 ViewMatrix;\n"\
   "uniform mat4 WorldMatrix;\n"\
   "void main()\n"\
   "{\n"\
   "gl_Position = ProjMatrix * ViewMatrix * WorldMatrix * vec4(aPos, 1.0);\n"\
   "ourColor = aColor;\n"\
   "TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"\
   "}\n"
        };
// Shader-ul de fragment / Fragment shader (este privit ca un sir de caractere)
const GLchar *FragmentShader =
        {
                "#version 330\n"\
   "out vec4 FragColor;\n"\
   "in vec4 ourColor;\n"\
   "in vec2 TexCoord;\n"\
   "uniform float mixValue;\n"\
   "uniform sampler2D texture1;\n"\
   "uniform sampler2D texture2;\n"\
   "void main()\n"\
   "{\n"\
   "  FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mixValue);\n"\
   "}\n"
        };


void CreateVBO() {

    MyMesh a = GetMesh();
    GLfloat my_color[72];
    for (int i = 0; i < 72; i+=3) {
        my_color[i]=1.0f;
        my_color[i+1]=1.0f;
        my_color[i+2]=1.0f;
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, a.vertices.size(), a.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, a.v_indices.size(), a.v_indices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void DestroyVBO() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void CreateShaders() {
    VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShaderId, 1, &VertexShader, NULL);
    glCompileShader(VertexShaderId);

    FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShaderId, 1, &FragmentShader, NULL);
    glCompileShader(FragmentShaderId);

    ProgramId = glCreateProgram();
    glAttachShader(ProgramId, VertexShaderId);
    glAttachShader(ProgramId, FragmentShaderId);
    glLinkProgram(ProgramId);

    GLint Success = 0;
    GLchar ErrorLog[1024] = {0};

    glGetProgramiv(ProgramId, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(ProgramId, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glValidateProgram(ProgramId);
    glGetProgramiv(ProgramId, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ProgramId, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ProgramId);

    ProjMatrixLocation = glGetUniformLocation(ProgramId, "ProjMatrix");
    ViewMatrixLocation = glGetUniformLocation(ProgramId, "ViewMatrix");
    WorldMatrixLocation = glGetUniformLocation(ProgramId, "WorldMatrix");
    mixValueLocation = glGetUniformLocation(ProgramId, "mixValue");

    glUniform1i(glGetUniformLocation(ProgramId, "texture1"), 0);
    glUniform1i(glGetUniformLocation(ProgramId, "texture2"), 1);
}

void DestroyShaders() {
    glUseProgram(0);

    glDetachShader(ProgramId, VertexShaderId);
    glDetachShader(ProgramId, FragmentShaderId);

    glDeleteShader(FragmentShaderId);
    glDeleteShader(VertexShaderId);

    glDeleteProgram(ProgramId);
}

void CreateTextures(const std::string &strExePath) {
    // load and create a texture
    // -------------------------
    // texture 1
    // ---------
    glGenTextures(1, &texture1Location);
    glBindTexture(GL_TEXTURE_2D, texture1Location);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load("Stones.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    // texture 2
    // ---------
    glGenTextures(1, &texture2Location);
    glBindTexture(GL_TEXTURE_2D, texture2Location);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    std::cout << strExePath;
    data = stbi_load("Bricks.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}

void Initialize(const std::string &strExePath) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // culoarea de fond a ecranului
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);

    //glFrontFace(GL_CCW);
    //glCullFace(GL_BACK);

    CreateVBO();
    CreateShaders();
    CreateTextures(strExePath);

    // Create camera
    pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.5, 0.5, 10));
}

void RenderCube() {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    int indexArraySize;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &indexArraySize);
    glDrawElements(GL_TRIANGLES, indexArraySize / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
}

void RenderFunction() {
    glm::vec3 cubePositions[] = {
            glm::vec3(0.0f, 0.0f, 0.0f),
    //        glm::vec3(-5.0f, 5.0f, 5.0f),
    //        glm::vec3(-5.0f, -5.0f, 5.0f),
    //        glm::vec3(5.0f, -5.0f, 5.0f),
    //        glm::vec3(5.0f, 5.0f, 5.0f),
    //        glm::vec3(-5.0f, 5.0f, -5.0f),
    //        glm::vec3(-5.0f, -5.0f, -5.0f),
    //        glm::vec3(5.0f, -5.0f, -5.0f),
    //        glm::vec3(5.0f, 5.0f, -5.0f),
    };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(ProgramId);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1Location);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2Location);

    glm::mat4 projection = pCamera->GetProjectionMatrix();
    glUniformMatrix4fv(ProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));

    //glm::mat4 view = pCamera->GetViewMatrix();
    //glUniformMatrix4fv(ViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 view;
    float radius = 10.0f;
    float camX = sin(glfwGetTime()) * radius;
    float camZ = cos(glfwGetTime()) * radius;
    view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(ViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));

    glBindVertexArray(VAO);

    //glUniform1f(mixValueLocation,mixValue);
    glUniform1f(glGetUniformLocation(ProgramId, "mixValue"), mixValue);
    for (unsigned int i = 0; i < sizeof(cubePositions) / sizeof(cubePositions[0]); i++) {
        // calculate the model matrix for each object and pass it to shader before drawing
        glm::mat4 worldTransf = glm::translate(glm::mat4(1.0), cubePositions[i]);
        glUniformMatrix4fv(WorldMatrixLocation, 1, GL_FALSE, glm::value_ptr(worldTransf));

        RenderCube();
    }
}

void Cleanup() {
    DestroyShaders();
    DestroyVBO();

    delete pCamera;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// timing
double deltaTime = 0.0f;    // time between current frame and last frame
double lastFrame = 0.0f;

int main(int argc, char **argv) {
    std::string strFullExeFileName = argv[0];
    std::string strExePath;
    const size_t last_slash_idx = strFullExeFileName.rfind('\\');
    if (std::string::npos != last_slash_idx) {
        strExePath = strFullExeFileName.substr(0, last_slash_idx);
    }

    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lab 6", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);


    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewInit();

    Initialize(strExePath);

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // render
        RenderFunction();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Cleanup();

    // glfw: terminate, clearing all previously allocated GLFW resources
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pCamera->ProcessKeyboard(FORWARD, (float) deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pCamera->ProcessKeyboard(BACKWARD, (float) deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        pCamera->ProcessKeyboard(LEFT, (float) deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        pCamera->ProcessKeyboard(RIGHT, (float) deltaTime);
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        pCamera->ProcessKeyboard(UP, (float) deltaTime);
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        pCamera->ProcessKeyboard(DOWN, (float) deltaTime);

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        pCamera->Reset(width, height);

    }
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        if (mixValue <= 1.0f) {
            mixValue += 0.01f;
            std::cout << "increaased" << std::endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        if (mixValue >= 0.0f) {
            mixValue -= 0.01f;
            std::cout << "decreased" << std::endl;
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    pCamera->Reshape(width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    pCamera->MouseControl((float) xpos, (float) ypos);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yOffset) {
    pCamera->ProcessMouseScroll((float) yOffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
}