/**
 * @file camera.h
 * @author Jerry Z 
 * @brief Arcball camera cpp header class
 * @version 0.1
 * @date 2022-03-16
 * 
 * An arcball camera for OpenGL
 * Assuming the world space is in y up, z out, x right coordinate system.
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef CAMERA_H
#define CAMERA_H

#include "cyCodeBase/cyVector.h"
#define _USE_MATH_DEFINES
#include "math.h"

class Camera
{
public:
    cy::Vec3f pos;
    cy::Vec3f lookat;
    cy::Vec3f up;
    
private:
    float yaw;   // Vertical angle in rad
    float pitch; // Horizontal angle in rad
    float sense = 0.005;
    float r;
    cy::Vec3f dir;

public:
    Camera(cy::Vec3f pos, cy::Vec3f lookat, cy::Vec3f up)
    {
        this->pos = pos;
        this->lookat = lookat;
        this->up = up; 
        this->yaw = atan2(pos.y, sqrt(pow(pos.x, 2)+pow(pos.z, 2)));
        this->pitch = atan2(pos.z, pos.x);
        this->dir = (lookat-pos).GetNormalized();
        this->r = (pos-lookat).Length();
    }

    void UpdatePosition(cy::Vec3f &p)
    {
        this->pos = p;
        this->dir = (lookat-pos).GetNormalized();
        this->r = (pos-lookat).Length();
    }

    void RotateHorizontal(float mag)
    {
        mag *= sense;
        pitch += mag;
        cy::Vec3f p(cos(yaw) * cos(pitch) * r, 
                    sin(yaw) * r, 
                    cos(yaw) * sin(pitch) * r);
        UpdatePosition(p);
    }

    void RotateVertical(float mag)
    {
        mag *= sense;
        if (abs(yaw+mag) < M_PI_2)
        {
            yaw += mag; 
            cy::Vec3f p(cos(yaw) * cos(pitch) * r, 
                        sin(yaw) * r, 
                        cos(yaw) * sin(pitch) * r);
            UpdatePosition(p);
        }
    }

    void Zoom(float mag)
    {
        mag *= sense;
        cy::Vec3f p = pos + dir * mag;
        UpdatePosition(p);
    }
};

#endif