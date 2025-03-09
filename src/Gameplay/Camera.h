#pragma once

#include "Check.h"
#include "Jnrlib.h"
#include <glm/glm.hpp>

class Camera
{
public:
    Camera() = default;
    Camera(glm::vec3 position, f32 width, f32 heigh, f32 fieldOfView);

public:
    void Update()
    {
        CalculateMatrices();
    }
    void MoveForward(float amount)
    {
        mPosition = mPosition + mForwardDirection * amount;
        DSHOWINFO("Camera position: ", mPosition.x, ", ", mPosition.y, ", ",
                  mPosition.z);
    }

    void MoveBackward(float amount)
    {
        MoveForward(-amount);
    }

    void StrafeRight(float amount)
    {
        mPosition = mPosition + mRightDirection * amount;
    }

    void StrafeLeft(float amount)
    {
        StrafeRight(-amount);
    }

    void Roll(float amount)
    {
        mRoll += amount;
        CalculateVectors();
    }

    void Yaw(float amount)
    {
        mYaw -= amount;
        CalculateVectors();
    }

    void Pitch(float amount)
    {
        mPitch -= amount;
        CalculateVectors();
    }

    glm::mat4x4 &GetView()
    {
        return mView;
    }
    glm::mat4x4 &GetProjection()
    {
        return mProjection;
    }

private:
    void CalculateMatrices();
    void CalculateVectors();

private:
    glm::vec3 mPosition;
    f32 mWidth;
    f32 mHeight;
    f32 mFieldOfView;

    f32 mPitch = 0.0f;
    f32 mYaw = 0.0f;
    f32 mRoll = 0.0f;

    glm::vec3 mForwardDirection;
    glm::vec3 mRightDirection;
    glm::vec3 mUpDirection;

    glm::mat4x4 mView;
    glm::mat4x4 mProjection;
};
