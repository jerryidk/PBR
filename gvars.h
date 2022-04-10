#ifndef GVARS_H
#define GVARS_H
char *AppName = (char*) "PBR";
int SCR_W = 800;
int SCR_H = 600;
float pre_xpos = 0;
float pre_ypos = 0;
Camera camera(cy::Vec3f(0.0f, 0.0f, 3.0f),
              cy::Vec3f(0.0f, 0.0f, 0.0f),
              cy::Vec3f(0.0f, 1.0f, 0.0f));
GLFWwindow *window;

bool onImgui = false;
float metallic = 0.3f;
float roughness = 0.3f;
float ao = 0.05f;
float albedo[3] = {0.5f, 0.0f, 0.0f};
int gammacorrect = 0;

float lightPositions[6] = {
    0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f
}; 
float lightColors[6] = {
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f
}; 


#endif