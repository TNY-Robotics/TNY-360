#pragma once
#include "common/utils.hpp"
#include "ui/Draw.hpp"

namespace UIWidgets
{
    class Animation
    {
    public:
        Animation() = default;
        Animation(uint8_t* buffer, uint8_t frameCount, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool transparent = false, bool loop = true)
            : buffer(buffer), length(frameCount), x(x), y(y), width(width), height(height), transparent(transparent), loop(loop), index(0) {}
        virtual ~Animation() = default;

        void update();
        void render();

        bool isLooping() const;
        bool isTransparent() const;
        bool isFinished() const;

        void setFrameIndex(uint8_t index);
        uint8_t getFrameIndex() const;
        uint8_t getFrameCount() const;

    private:
        uint8_t* buffer = nullptr;
        uint8_t length = 0;
        uint8_t x = 0;
        uint8_t y = 0;
        uint8_t width = 0;
        uint8_t height = 0;
        bool transparent = false;
        bool loop = true;
        uint8_t index = 0;
    };
}