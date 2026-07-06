#pragma once
#include "common/utils.hpp"
#include "ui/Draw.hpp"
#include "qrcodegen.hpp"

namespace UIWidgets
{
    class QRCode
    {
    public:
        QRCode() = default;
        QRCode(const char* data, uint8_t x, uint8_t y, uint8_t width, uint8_t height);
        virtual ~QRCode() = default;

        void update();
        void render();
        void setData(const char* newData);

    private:
        void generate();

        const char* data = nullptr;
        uint8_t x = 0;
        uint8_t y = 0;
        uint8_t width = 0;
        uint8_t height = 0;

        uint8_t qrcode[qrcodegen_BUFFER_LEN_FOR_VERSION(4)] = {0};
        int qr_size = 0;
        bool is_valid = false;
    };
}