
#ifndef CALLBACK_H
#define CALLBACK_H

#include "GLFW/glfw3.h"
#include <iostream>
#include "camera.h"
#include "gvars.h"     // Global variables

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
        
}

#endif