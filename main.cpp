#define GLEW_STATIC
#define CY_GL_DONT_CHECK_CONTEXT
#include "GLEW/glew.h"
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw.h"
#include "IMGUI/imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"
#include "cyCodeBase/cyTriMesh.h"
#include "cyCodeBase/cyMatrix.h"
#include "cyCodeBase/cyGL.h"
#include "camera.h"    // camera class
#include "gvars.h"     // Global variables
#include "callbacks.h" // Call back functions
#include "common.h"    // Common gl operations including, setting things up, load mesh, creating VAOs

void ConvertMap(cy::GLSLProgram &shader, GLenum fromtype, unsigned int from, unsigned int to,
                GLuint type, int w, int h, int VAO, int nv);

int main(void)
{
    //----------
    // Set up
    //----------
    SetUpGLFW();
    SetUpIMGUI();
    SetUpGLEW();

    glEnable(GL_DEPTH_TEST);

    //----------
    // Load mesh
    //----------

    cy::TriMesh teapot, cube;

    teapot.LoadFromFileObj("./resources/teapot.obj");
    cube.LoadFromFileObj("./resources/cube.obj");

    int datasize = 0;
    float *datateapot = GetMeshInfo(teapot, true, false, datasize);
    int sizes[2] = {3, 3};
    int VAO = CreateVAO(datateapot, datasize, 2, sizes);

    datasize = 0;
    float *datacube = GetMeshInfo(cube, false, false, datasize);
    int sizescube[1] = {3};

    int VAOCube = CreateVAO(datacube, datasize, 1, sizescube);

    //----------
    // Load shaders
    //----------
    cy::GLSLProgram teapotShader, toCubemapShader, envShader, convolutionShader;
    teapotShader.BuildFiles("./shaders/teapot.vs",
                            "./shaders/teapot.fs");
    toCubemapShader.BuildFiles("./shaders/cube.vs",
                             "./shaders/cube.fs");
    envShader.BuildFiles("./shaders/cube.vs",
                         "./shaders/env.fs");
    convolutionShader.BuildFiles("./shaders/cube.vs",
                                 "./shaders/convo.fs");

    //----------
    // Load texture
    //----------
    int hdrMap = LoadTexture("./resources/scene.hdr");

    //-----------
    // Create render buffer and frame buffer to convert hdr image to a env map
    //-----------
    int w = 512;
    int h = 512;
    int envMap = CreateEnvMap(w, h);
    ConvertMap(toCubemapShader, GL_TEXTURE_2D, hdrMap, envMap, GL_DEPTH_COMPONENT24, w, h, VAOCube, cube.NF() * 3);

    //-----------
    // Convert given irradianceMap to a cubemap
    //-----------
    int diffuseMap = LoadTexture("./resources/diffusemap.hdr");
    int irradianceMapGiven = CreateEnvMap(w, h);
    ConvertMap(toCubemapShader, GL_TEXTURE_2D, diffuseMap, irradianceMapGiven, GL_DEPTH_COMPONENT24, w, h, VAOCube, cube.NF() * 3);

    //-----------
    // Create render buffer and frame buffer to convert env cube map to diffuse irradiance lighting map
    //-----------
    w = 32;
    h = 32;
    int irradianceMapMy = CreateEnvMap(w, h);
    ConvertMap(convolutionShader, GL_TEXTURE_CUBE_MAP, envMap, irradianceMapMy, GL_DEPTH_COMPONENT24, w, h, VAOCube, cube.NF() * 3);

    //----------
    // Matrix transformation set up
    //----------
    cy::Matrix4f M, M2, V, P, S, R, T, S2;

    // Teapot
    P.SetPerspective(45.0f * M_PI / 180.0, (float)SCR_W / (float)SCR_H, 0.1f, 100.0f);
    S.SetScale(0.05f, 0.05f, 0.05f);
    R.SetRotationX(-90.0f * M_PI / 180.0);
    M = S * R;

    // For convert cube to center
    S2.SetScale(2.0f, 2.0f, 2.0f);
    T.SetTranslation(cy::Vec3f(-0.5f, -0.5f, -0.5f));
    M2 = S2 * T;

    cy::Matrix4f M3, S3;
    S3.SetScale(10.0f, 10.0f, 10.0f);
    M3 = S3 * T;

    ImGuiIO &io = ImGui::GetIO();

    glViewport(0, 0, SCR_W, SCR_H);
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {

        glfwPollEvents();

        //----------
        // IMGUI
        //----------

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        onImgui = io.WantCaptureMouse;
        ImGui::Begin("Window");
        ImGui::SliderFloat("metallic", &metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("roughness", &roughness, 0.0f, 1.0f);
        ImGui::SliderFloat("ao", &ao, 0.0f, 1.0f);
        ImGui::Checkbox("gamma correction", (bool *)&gammacorrect);
        ImGui::Checkbox("convolute?", &convolute);
        ImGui::Checkbox("show diffuse light map", &lightmap);
        ImGui::ColorEdit3("albedo", albedo);
        ImGui::End();
        ImGui::Render();

        if(convolute)
            irradianceMap = irradianceMapMy;
        else
            irradianceMap = irradianceMapGiven;

        if(lightmap)
            enviromentMap = irradianceMap;
        else
            enviromentMap = envMap; 

        //----------
        // Opengl Render
        //----------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //------
        // Enviroment
        //-----
        glDepthMask(GL_FALSE);
        envShader.Bind();
        glBindVertexArray(VAOCube);
        envShader["mvp"] = P * cy::Matrix4(V.GetSubMatrix3()) * M3;
        envShader["model"] = M3;
        envShader["envMap"] = 0;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, enviromentMap);
        envShader.SetUniform("gammacorrect", gammacorrect);
        glDrawArrays(GL_TRIANGLES, 0, cube.NF() * 3);
        glDepthMask(GL_TRUE);

        // ------
        // Teapot
        // ------
        teapotShader.Bind();
        glBindVertexArray(VAO);
        V.SetView(camera.pos, camera.lookat, camera.up);
        teapotShader["mvp"] = P * V * M;
        teapotShader["model"] = M;
        teapotShader["metallic"] = metallic;
        teapotShader["roughness"] = roughness;
        teapotShader["ao"] = ao;
        teapotShader["camPos"] = camera.pos;
        teapotShader.SetUniform("gammacorrect", gammacorrect);
        teapotShader.SetUniform3("albedo", 1, albedo);
        teapotShader.SetUniform3("lightPositions", 2, lightPositions);
        teapotShader.SetUniform3("lightColors", 2, lightColors);
        teapotShader["irradianceMap"] = 0;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glDrawArrays(GL_TRIANGLES, 0, teapot.NF() * 3);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    free(datateapot);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void ConvertMap(cy::GLSLProgram &shader, GLenum fromtype, unsigned int from, unsigned int to,
                GLuint type, int w, int h, int VAO, int nv)
{

    int fbo = CreateFramebuffer();
    int rbo = CreateRenderbuffer(type, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    cy::Matrix4f P, v1, v2, v3, v4, v5, v6, M, S, T;
    cy::Vec3f origin(0.0f);
    P.SetPerspective(90.0f * M_PI / 180.0, 1.0f, 0.1f, 100.0f);
    v1.SetView(origin, cy::Vec3f(1.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, -1.0f, 0.0f));
    v2.SetView(origin, cy::Vec3f(-1.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, -1.0f, 0.0f));
    v3.SetView(origin, cy::Vec3f(0.0f, 1.0f, 0.0f), cy::Vec3f(0.0f, 0.0f, 1.0f));
    v4.SetView(origin, cy::Vec3f(0.0f, -1.0f, 0.0f), cy::Vec3f(0.0f, 0.0f, -1.0f));
    v5.SetView(origin, cy::Vec3f(0.0f, 0.0f, 1.0f), cy::Vec3f(0.0f, -1.0f, 0.0f));
    v6.SetView(origin, cy::Vec3f(0.0f, 0.0f, -1.0f), cy::Vec3f(0.0f, -1.0f, 0.0f));
    cy::Matrix4f Vs[] = {
        v1, v2, v3, v4, v5, v6};

    S.SetScale(2.0f, 2.0f, 2.0f);
    T.SetTranslation(cy::Vec3f(-0.5f, -0.5f, -0.5f));
    M = S * T;

    shader.Bind();
    shader["model"] = M;
    shader["textureMap"] = 0;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(fromtype, from);

    glViewport(0, 0, w, h);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, to, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader["mvp"] = P * Vs[i] * M;
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, nv);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}