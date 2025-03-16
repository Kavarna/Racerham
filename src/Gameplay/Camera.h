#pragma once

#include "Check.h"
#include "Jnrlib.h"
#include "Utils/Constants.h"
#include <glm/glm.hpp>

class Camera
{
public:
    Camera() = default;
    Camera(glm::vec3 position, f32 width, f32 heigh, f32 fieldOfView);

public:
    bool Update()
    {
        return CalculateMatrices();
    }
    void MoveForward(float amount)
    {
        mPosition = mPosition + mForwardDirection * amount;
        MarkDirty();
    }

    void MoveBackward(float amount)
    {
        MoveForward(-amount);
    }

    void StrafeRight(float amount)
    {
        mPosition = mPosition + mRightDirection * amount;
        MarkDirty();
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

    glm::mat4x4 const &GetView() const
    {
        return mView;
    }
    glm::mat4x4 const &GetProjection() const
    {
        return mProjection;
    }

private:
    bool CalculateMatrices();
    void CalculateVectors();

    void MarkDirty()
    {

        mDirtyFrames = Constants::MAX_IN_FLIGHT_FRAMES;
    }

private:
    u32 mDirtyFrames;
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
