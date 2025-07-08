#ifndef MY_CAMERA_H
#define MY_CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Constraints on pitch and zoom
const float MIN_PITCH = -89.0f;
const float MAX_PITCH = 89.0f;
const float MIN_ZOOM = 1.0f;
const float MAX_ZOOM = 60.0f;

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float CAMERA_SPEED = 2.5f;
const float MOUSE_SENSITIVITY = 0.1f;
const float ZOOM = 50.0f; // FOV

// Wall constraints (changed later during setup)
float WALL_X_MIN = -50.0f;
float WALL_X_MAX = 50.0f;
float WALL_Z_MIN = -50.0f;
float WALL_Z_MAX = 50.0f;

// For collision detection
float FISH_TANK_RAD = 7.5f;

class Camera
{
public:
    // Camera attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Euler Angles
    float yaw;
    float pitch;

    // Camera params
    float movementSpeed;
    float mouseSensitivity;
    float zoom;
    bool fps;
    float fixedYPos;
    bool zoomEnabled;

    // Constructor
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW, float pitch = PITCH,
        bool fps = false, float yFixed = 0.0f,
        bool zoomEnable = true)
        : front(glm::vec3(0.0f, 0.0f, -1.0f))
        , movementSpeed(CAMERA_SPEED)
        , mouseSensitivity(MOUSE_SENSITIVITY)
        , zoom(ZOOM)
    {
        this->position = position;
        this->worldUp = up;
        this->yaw = yaw;
        this->pitch = pitch;
        this->fps = fps;
        this->fixedYPos = yFixed;
        this->zoomEnabled = zoomEnable;
        updateCameraVectors();
    }

    // Set sensitivity
    void setMouseSensitivity(const float newSensitivity)
    {
        mouseSensitivity = newSensitivity;
    }

    // Set camera movement speed
    void setCameraMovementSpeed(const float newSpeed)
    {
        movementSpeed = newSpeed;
    }

    // Set FPS camera
    void setFPSCamera(const bool fps, const float yPos)
    {
        this->fps = fps;
        this->fixedYPos = yPos;
    }

    // Set wall constraints
    void setWallConstrains(const float xMin, const float xMax, const float zMin, const float zMax)
    {
        WALL_X_MIN = xMin;
        WALL_X_MAX = xMax;
        WALL_Z_MIN = zMin;
        WALL_Z_MAX = zMax;
    }

    // Override
    void setWallConstrains(const glm::vec4 constraints)
    {
        // Order: xMin, xMax, zMin, zMax
        WALL_X_MIN = constraints[0];
        WALL_X_MAX = constraints[1];
        WALL_Z_MIN = constraints[2];
        WALL_Z_MAX = constraints[3];
    }

    // Set zoom
    void setZoom(const float zoom)
    {
        this->zoom = zoom;
    }

    // Enable/Disable zoom
    void setZoomEnabled(const bool enable)
    {
        zoomEnabled = enable;
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 getViewMatrix()
    {
        return glm::lookAt(position, position + front, up);
    }

    // Processes input received from keyboard
    void processKeyboardInput(const int direction, float deltaTime)
    {
        float velocity = movementSpeed * deltaTime;
        if (direction == GLFW_KEY_W)
        {
            if (checkPositionConstraints(position + front * velocity))
                position += front * velocity;
        }
        if (direction == GLFW_KEY_A)
        {
            if (checkPositionConstraints(position - right * velocity))
                position -= right * velocity;
        }
        if (direction == GLFW_KEY_S)
        {
            if (checkPositionConstraints(position - front * velocity))
                position -= front * velocity;
        }
        if (direction == GLFW_KEY_D)
        {
            if (checkPositionConstraints(position + right * velocity))
                position += right * velocity;
        }

        // If FPS camera, ignore y-coordinate changes
        if (fps)
            position.y = fixedYPos;
    }

    // Position constraints check - so far just wall constraints
    bool checkPositionConstraints(glm::vec3 newPosition)
    {
        // Only need to check XZ constraints for FPS camera
        if (newPosition.x < WALL_X_MIN ||
            newPosition.x > WALL_X_MAX ||
            newPosition.z < WALL_Z_MIN ||
            newPosition.z > WALL_Z_MAX)
            return false;

        // Check not going into fishtank
        if (std::sqrtf(std::powf(newPosition.x, 2.0f) + std::powf(newPosition.z, 2.0f)) <= (FISH_TANK_RAD + 0.2f))
            return false;

        return true;
    }

    // Processes input received from mouse
    void processMouseMovement(float xOff, float yOff)
    {
        xOff *= mouseSensitivity;
        yOff *= mouseSensitivity;
        yaw += xOff; pitch += yOff;

        // Constrain pitch
        if (pitch > MAX_PITCH)
            pitch = MAX_PITCH;
        if (pitch < MIN_PITCH)
            pitch = MIN_PITCH;

        // Update front, right and up vectors with updated Euler angles
        updateCameraVectors();
    }

    // Processes input received from mouse scroll-wheel
    void processMouseScroll(float yOff)
    {
        // Only if zoom is enabled
        if (zoomEnabled)
        {
            zoom -= yOff;
            // Constrain zoom
            if (zoom < MIN_ZOOM)
                zoom = MIN_ZOOM;
            if (zoom > MAX_ZOOM)
                zoom = MAX_ZOOM;
        }
    }

private:
    // Calculates the front vector from camera's new Euler Angles
    void updateCameraVectors()
    {
        // Front vector
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(newFront);

        // Right and up vectors
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};
#endif // MY_CAMERA_H

