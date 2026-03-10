#pragma once
#include "common/utils.hpp"

class CameraDriver
{
public:
    CameraDriver();
    ~CameraDriver();

    Error init();
    Error deinit();
};