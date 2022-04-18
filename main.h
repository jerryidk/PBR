
#pragma once

#define GLEW_STATIC
#define CY_GL_DONT_CHECK_CONTEXT
#include "GL/glew.h"
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw.h"
#include "IMGUI/imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"
#include "cyCodeBase/cyTriMesh.h"
#include "cyCodeBase/cyMatrix.h"
#include "cyCodeBase/cyGL.h"
#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"
#include "camera.h"    // camera class

char *AppName = (char*) "PBR";
int SCR_W = 800*2;
int SCR_H = 600*2;
float pre_xpos = 0;
float pre_ypos = 0;
Camera camera(cy::Vec3f(0.0f, 0.0f, 3.0f),
              cy::Vec3f(0.0f, 0.0f, 0.0f),
              cy::Vec3f(0.0f, 1.0f, 0.0f));
cy::Vec3f lightPosition(12.5f, 1.5f, 10.0f); 
cy::Vec3f lightColor(1000.0f, 1000.0f, 1000.0f); 

GLFWwindow *window;

bool onImgui = false;
float metallic = 0.5f;
float roughness = 0.5f;
float ao = 0.5f;
float albedo[3] = {0.5f, 0.0f, 0.0f};
int gammacorrect = 1;
bool convolute = false;
int lightmap = 0;
int enviromentMap = 0;
int irradianceMap = 0;



void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    int esc_state = glfwGetKey(window, GLFW_KEY_ESCAPE);
    if (esc_state == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
{

    int left_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    int right_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    int middle_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);

    if (left_state == GLFW_PRESS && !onImgui)
    {
        if (xpos > pre_xpos)
            camera.RotateHorizontal(1.0f);
        if (xpos < pre_xpos)
            camera.RotateHorizontal(-1.0f);
        if (ypos > pre_ypos)
            camera.RotateVertical(1.0f);
        if (ypos < pre_ypos)
            camera.RotateVertical(-1.0f);
      
        pre_xpos = xpos;
        pre_ypos = ypos; 
    }

    if (right_state == GLFW_PRESS && !onImgui)
    {
        if (xpos > pre_xpos)
            camera.Zoom(3.0f);
        if (xpos < pre_xpos)
            camera.Zoom(-3.0f);
        pre_xpos = xpos;
        pre_ypos = ypos; 
    }

    if (middle_state == GLFW_PRESS && !onImgui)
    {
        if (xpos > pre_xpos)
            lightPosition *= 1.01;
        if (xpos < pre_xpos)
            lightPosition *= 0.99;
        pre_xpos = xpos;
        pre_ypos = ypos; 
    }
}

/**
 * @brief Set the Up G L F W
 *
 */
void SetUpGLFW()
{
    if (!glfwInit())
        std::cout << "glfw error" << std::endl;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(SCR_W, SCR_H, AppName, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        std::cout << "glfw error" << std::endl;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
}

/**
 * @brief Set the Up I M G U I
 *
 */
void SetUpIMGUI()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
}

/**
 * @brief Set the Up G L E W
 *
 */
void SetUpGLEW()
{
    if (glewInit() != GLEW_OK)
        std::cout << "glew error" << std::endl;
}

// return array of floats and buffer_length is the length of the array
float *GetMeshInfo(cy::TriMesh &mesh, bool need_nor, bool need_uv, int &buffersize)
{
    int stride = 3;
    if (need_nor)
        stride += 3;
    if (need_uv)
        stride += 2;

    int nf = mesh.NF();
    float *buffer = (float *)malloc(3 * nf * stride * sizeof(float));
    int k = 0;
    // for each face
    for (int i = 0; i < nf; i++)
    {
        // get v/vn/vt for the faces
        for (int j = 0; j < 3; j++)
        {
            unsigned int p_index = mesh.F(i).v[j];
            buffer[k] = mesh.V(p_index).x;
            buffer[k + 1] = mesh.V(p_index).y;
            buffer[k + 2] = mesh.V(p_index).z;

            if (need_nor)
            {
                unsigned int n_index = mesh.FN(i).v[j];
                buffer[k + 3] = mesh.VN(n_index).x;
                buffer[k + 4] = mesh.VN(n_index).y;
                buffer[k + 5] = mesh.VN(n_index).z;
            }

            if (need_uv)
            {
                unsigned int t_index = mesh.FT(i).v[j];
                buffer[k + 6] = mesh.VT(t_index).x;
                buffer[k + 7] = mesh.VT(t_index).y;
            }

            k += stride;
        }
    }

    buffersize = nf * 3 * stride * sizeof(float);

    return buffer;
}

/**
 * @brief
 *  Create a mesh with a vao and a vbo. This method is based on convention
 *
 *  data is the buffer array
 *  datasize is how big the buffer is in byte
 *  nAttribute tells how many attribute there are
 *  sAttribute tells size of the attribute.
 * @return mesh vao ID
 */
unsigned int CreateVAO(float *data, int datasize, int nAttribute, int *sAttribute)
{

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, datasize, data, GL_STATIC_DRAW);

    int stride = 0;
    for (int i = 0; i < nAttribute; i++)
        stride += sAttribute[i];

    int offset = 0;
    for (int i = 0; i < nAttribute; i++)
    {
        int loc = i;
        int size = sAttribute[loc];
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, size, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void *)(offset * sizeof(float)));
        offset += size;
    }
    return VAO;
}

unsigned int LoadTexture(char *name)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float *data = stbi_loadf(name, &width, &height, &nrComponents, 0);
    unsigned int Texture;
    if (data)
    {
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load HDR image." << std::endl;
    }

    return Texture;
}

unsigned int CreateFramebuffer()
{
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    return fbo;
}

unsigned int CreateRenderbuffer(GLuint type, int w, int h)
{
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, type, w, h);
    return rbo;
}

unsigned int CreateTexture(GLenum type, int w, int h) 
{
  unsigned int texture;
  glGenTextures(1, &texture);

  // pre-allocate enough memory for the LUT texture.
  glBindTexture(type, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, w, h, 0, GL_RG, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return texture;
}

unsigned int CreateEnvMap(int w, int h, bool mipmap)
{
    unsigned int envMap;
    glGenTextures(1, &envMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        // note that we store each face with 16 bit floating point values
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                     w, h, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (mipmap) {
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
      glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    } else
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return envMap;
}
