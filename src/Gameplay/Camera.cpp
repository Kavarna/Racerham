#include "Camera.h"

#include "Utils/Constants.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

Camera::Camera(glm::vec3 position, f32 width, f32 heigh, f32 fieldOfView)
    : mPosition(position), mWidth(width), mHeight(heigh),
      mFieldOfView(fieldOfView)
{
    mForwardDirection = Constants::DEFAULT_FORWARD_DIRECTION;
    mRightDirection = Constants::DEFAULT_RIGHT_DIRECTION;
    mUpDirection = Constants::DEFAULT_UP_DIRECTION;
}

void Camera::CalculateMatrices()
{
    mView =
        glm::lookAtLH(mPosition, mPosition + mForwardDirection, mUpDirection);
    mProjection =
        glm::perspectiveLH(mFieldOfView, mWidth / mHeight, 0.1f, 1000.0f);
    mProjection[1][1] *= -1;
}

void Camera::CalculateVectors()
{
    auto rotateMatrix = glm::eulerAngleXYZ(mPitch, mYaw, mRoll);

    mForwardDirection = Constants::DEFAULT_FORWARD_DIRECTION * rotateMatrix;
    mRightDirection = Constants::DEFAULT_RIGHT_DIRECTION * rotateMatrix;
    mUpDirection = glm::cross(mForwardDirection, mRightDirection);
}
