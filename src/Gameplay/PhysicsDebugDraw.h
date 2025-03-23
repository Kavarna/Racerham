#pragma once

#include "Check.h"
#include "LinearMath/btIDebugDraw.h"
#include "Renderer/BatchRenderer.h"
#include "Utils/Vertex.h"

class PhysicsDebugDraw : public btIDebugDraw
{
public:
    PhysicsDebugDraw() = default;
    PhysicsDebugDraw(BatchRenderer *batchRenderer)
        : mBatchRenderer(batchRenderer)
    {
    }

    virtual void drawLine(const btVector3 &from, const btVector3 &to,
                          const btVector3 &color) override
    {
        drawLine(from, to, color, color);
    }

    virtual void drawLine(const btVector3 &from, const btVector3 &to,
                          const btVector3 &fromColor,
                          const btVector3 &toColor) override
    {
        VertexPositionColor v1(from.x(), from.y(), from.z(), fromColor.x(),
                               fromColor.y(), fromColor.z(), 1.0f);
        VertexPositionColor v2(to.x(), to.y(), to.z(), toColor.x(), toColor.y(),
                               toColor.z(), 1.0f);
        mBatchRenderer->AddVertex(v1);
        mBatchRenderer->AddVertex(v2);
    }

    virtual void drawContactPoint(const btVector3 &PointOnB,
                                  const btVector3 &normalOnB, btScalar distance,
                                  int lifeTime, const btVector3 &color) override
    {
    }

    virtual void reportErrorWarning(const char *warningString) override
    {
        SHOWWARNING("Physics warning: ", warningString);
    }

    virtual void draw3dText(const btVector3 &location,
                            const char *textString) override
    {
    }

    virtual void setDebugMode(int debugMode)
    {
    }

    virtual int getDebugMode() const override
    {
        return 0;
        return (1 << 16) - 1;
    }

    virtual void clearLines() override
    {
        mBatchRenderer->Clear();
    }

public:
    BatchRenderer *mBatchRenderer;
};
