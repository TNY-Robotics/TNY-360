#include "ui/widgets/qrcode.hpp"

namespace UIWidgets
{
    QRCode::QRCode(const char* data, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
        : data(data), x(x), y(y), width(width), height(height)
    {
        generate();
    }

    void QRCode::setData(const char* newData)
    {
        data = newData;
        generate();
    }

    void QRCode::generate()
    {
        if (data == nullptr)
        {
            is_valid = false;
            return;
        }

        uint8_t tempBuffer[qrcodegen_BUFFER_LEN_FOR_VERSION(4)];
        
        bool ok = qrcodegen_encodeText(data, tempBuffer, qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 4, qrcodegen_Mask_AUTO, true);
        
        if (ok)
        {
            qr_size = qrcodegen_getSize(qrcode);
            is_valid = true;
        }
        else
        {
            is_valid = false;
        }
    }

    void QRCode::update()
    {
        // Nothing to do here
    }

    void QRCode::render()
    {
        if (!is_valid || width == 0 || height == 0)
        {
            // TODO : DISPLAY ERROR (Generation failed or invalid dimensions)
            return;
        }

        constexpr const int QR_PADDING = 2;
        int scale = 1;

        // --- Calcul de l'échelle adaptative (x2 ou x1) ---
        int size_x2 = (qr_size * 2) + (QR_PADDING * 2);
        int size_x1 = (qr_size * 1) + (QR_PADDING * 2);

        if (size_x2 <= width && size_x2 <= height)
        {
            scale = 2;
        }
        else if (size_x1 <= width && size_x1 <= height)
        {
            scale = 1;
        }
        else
        {
            // TODO : DISPLAY ERROR (too little space to render QR Code)
            return;
        }

        int actual_qr_size = qr_size * scale;
        int corner_x = x + (width - actual_qr_size) / 2;
        int corner_y = y + (height - actual_qr_size) / 2;

        Draw::RectRounded(corner_x - QR_PADDING, corner_y - QR_PADDING, actual_qr_size + QR_PADDING * 2, actual_qr_size + QR_PADDING * 2, 2);

        for (int j = 0; j < qr_size; j++)
        {
            for (int i = 0; i < qr_size; i++)
            {
                if (qrcodegen_getModule(qrcode, i, j)) 
                {
                    Draw::RectFilled(corner_x + (i * scale), corner_y + (j * scale), scale, scale, ScreenDriver::COLOR_BLACK);
                }
            }
        }
    }
}