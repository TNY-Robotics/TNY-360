#include "ui/widgets/animation.hpp"

namespace UIWidgets
{
    void Animation::update()
    {
        if (loop)
        {
            index = (index + 1) % length;
        }
        else
        {
            if (index < length - 1)
            {
                index++;
            }
        }
    }

    void Animation::render()
    {
        if (buffer == nullptr || length == 0)
        {
            return;
        }

        uint8_t* frameData = buffer + (index * (width * height) / 8); // 1 bit per pixel, so divide by 8
        Draw::Blit(x, y, width, height, frameData, ScreenDriver::COLOR_WHITE, transparent);
    }

    bool Animation::isLooping() const
    {
        return loop;
    }

    bool Animation::isTransparent() const
    {
        return transparent;
    }

    bool Animation::isFinished() const
    {
        return !loop && index >= length - 1;
    }

    void Animation::setFrameIndex(uint8_t newIndex)
    {
        if (newIndex < length)
        {
            index = newIndex;
        }
    }

    uint8_t Animation::getFrameIndex() const
    {
        return index;
    }

    uint8_t Animation::getFrameCount() const
    {
        return length;
    }
}