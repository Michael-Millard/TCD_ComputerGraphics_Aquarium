#ifndef MY_POSES_H
#define MY_POSES_H

#include <glad/glad.h>
#include <random>

float generateRandomNumInRange(float low, float high)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(low, high);

    return dis(gen);
}

class ModelPose
{
public:
    GLfloat transX;
    GLfloat transY;
    GLfloat transZ;
    GLfloat rotX;
    GLfloat rotY;
    GLfloat rotZ;

    // Constructor with just translations
    ModelPose(float tX, float tY, float tZ)
    {
        transX = tX;
        transY = tY;
        transZ = tZ;

        rotX = 0.0f;
        rotY = generateRandomNumInRange(0.0f, 180.0f);
        rotZ = 0.0f;
    }

    // Constructor with range vals for translations
    ModelPose(float tXLow, float tXHigh,
        float tYLow, float tYHigh,
        float tZLow, float tZHigh)
    {
        transX = generateRandomNumInRange(tXLow, tXHigh);
        transY = generateRandomNumInRange(tYLow, tYHigh);
        transZ = generateRandomNumInRange(tZLow, tZHigh);

        rotX = 0.0f;
        rotY = generateRandomNumInRange(0, 180.0f);
        rotZ = 0.0f;
    }
};

#endif // MY_POSES_H
