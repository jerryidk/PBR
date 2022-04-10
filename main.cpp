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

int main(void)
{
    SetUpGLFW();
    SetUpIMGUI();
    SetUpGLEW();

    glEnable(GL_DEPTH_TEST);
    cy::GLSLProgram shader;
    cy::TriMesh teapot;
    shader.BuildFiles("./shaders/teapot.vs",
                      "./shaders/teapot.fs");
    teapot.LoadFromFileObj("./resources/teapot.obj");
    int datasize = 0;
    float *datateapot = GetMeshInfo(teapot, true, false, datasize);
    int sizes[2] = {3, 3};
    int VAO = CreateVAO(datateapot, datasize, 2, sizes);

    cy::Matrix4f M, V, P, S, R;

    P.SetPerspective(45.0f * M_PI / 180.0, (float)SCR_W / (float)SCR_H, 0.1f, 100.0f);
    S.SetScale(0.05f, 0.05f, 0.05f);
    R.SetRotationX(-60.0f * M_PI / 180.0);
    M = S * R;

    ImGuiIO &io = ImGui::GetIO();
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
        ImGui::ColorEdit3("albedo", albedo);
        ImGui::End();
        ImGui::Render();

        //----------
        // Opengl Render
        //----------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Bind();
        glBindVertexArray(VAO);
        V.SetView(camera.pos, camera.lookat, camera.up);
        shader["mvp"] = P * V * M;
        shader["model"] = M;
        shader["metallic"] = metallic;
        shader["roughness"] = roughness;
        shader["ao"] = ao;
        shader["camPos"] = camera.pos;
        shader.SetUniform("gammacorrect", gammacorrect);
        shader.SetUniform3("albedo", 1, albedo);
        shader.SetUniform3("lightPositions", 2, lightPositions);
        shader.SetUniform3("lightColors", 2, lightColors);
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
