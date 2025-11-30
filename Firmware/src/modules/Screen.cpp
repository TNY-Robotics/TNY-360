#include "modules/Screen.hpp"
#include "modules/I2C.hpp"
#include "font8x8_basic.h"
#include "esp_lcd_panel_sh1106.h"
#include <esp_lcd_io_i2c.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <memory.h>
#include <utility>

namespace Screen
{
    Info info;

    constexpr uint16_t SH1106_PAGE_HEIGHT = 8;

    esp_lcd_panel_handle_t panel_handle = NULL;
    
    bool screen_data[SH1106_HEIGHT * SH1106_WIDTH];
    uint8_t buffer_data[SH1106_BUFFER_SIZE];

    /* Convert a screen buffer data to a lcd-format image buffer (e.g. SH1106)
    * @param screen_info: the screen buffer information, see screen_info_t
    * @param buffer_info: the buffer information, see buffer_info_t
    */
    void screen_to_buffer(Info* screen_info, uint8_t* buffer_data)
    {
        uint16_t page_count = screen_info->height / SH1106_PAGE_HEIGHT;
        for (uint16_t i = 0; i < page_count; i++)
        {
            for (uint16_t j = 0; j < screen_info->width; j++)
            {
                uint8_t byte = 0;
                for (uint16_t k = 0; k < SH1106_PAGE_HEIGHT; k++)
                {
                    byte |= (screen_info->data[(i * SH1106_PAGE_HEIGHT + k) * screen_info->width + j] ? 1 : 0) << k;
                }
                buffer_data[i * screen_info->width + j] = byte;
            }
        }
    }

    Error Init() {
        esp_lcd_panel_io_handle_t io_handle = NULL;
        esp_lcd_panel_io_i2c_config_t io_config = ESP_SH1106_DEFAULT_IO_CONFIG;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(I2C::handle_secondary, &io_config, &io_handle));

        esp_lcd_panel_dev_config_t panel_config = {
            .reset_gpio_num = -1,
            .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
            .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
            .bits_per_pixel = SH1106_PIXELS_PER_BYTE / 8,
            .flags = {
                .reset_active_high = false,
            },
            .vendor_config = NULL,
        };
        ESP_ERROR_CHECK(esp_lcd_new_panel_sh1106(io_handle, &panel_config, &panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
        
        info = {
            .data = screen_data,
            .width = SH1106_WIDTH,
            .height = SH1106_HEIGHT,
        };

        // Clear all screen data and upload it
        if (Error err = Clear(); err != Error::Ok) return err;
        if (Error err = Upload(); err != Error::Ok) return err;

        return Error::Ok;
    }

    Error Clear()
    {
        memset(screen_data, 0, SH1106_HEIGHT * SH1106_WIDTH);
        memset(buffer_data, 0, SH1106_BUFFER_SIZE);
        return Error::Ok;
    }

    Error Upload()
    {
        screen_to_buffer(&info, buffer_data);
        esp_err_t err = esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, SH1106_WIDTH, SH1106_HEIGHT, buffer_data);
        if (err != ESP_OK) {
            return Error::Unknown;
        }
        return Error::Ok;
    }
}

namespace Draw
{
    void Pixel(Screen::Info* screen_info, uint16_t x, uint16_t y, Screen::Color c)
    {
        screen_info->data[y * screen_info->width + x] = c;
    }

    void Line(Screen::Info* screen_info, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Screen::Color c)
    {
        // Clipping basique pour éviter de sortir de la RAM (crash assuré avec des pointeurs)
        if (x1 < 0 || x1 >= screen_info->width || y1 < 0 || y1 >= screen_info->height ||
            x2 < 0 || x2 >= screen_info->width || y2 < 0 || y2 >= screen_info->height) 
        {
            // Idéalement, il faut une fonction de clipping (Cohen-Sutherland), 
            // mais pour l'instant on ignore les lignes hors-limites pour protéger la mémoire.
            return; 
        }

        int16_t dx = abs(x2 - x1);
        int16_t dy = abs(y2 - y1);
        
        // Calcul des incréments de pointeur
        // sx: Saut mémoire pour bouger en X (+1 ou -1)
        // sy: Saut mémoire pour bouger en Y (+width ou -width)
        int16_t sx = (x1 < x2) ? 1 : -1;
        int32_t sy = (y1 < y2) ? screen_info->width : -screen_info->width; // int32 pour être sûr
        
        int16_t err = dx - dy;

        // Pointeur direct vers le premier pixel
        // Note : data est traité ici comme uint8_t/bool
        auto* ptr = &screen_info->data[y1 * screen_info->width + x1];

        while (true)
        {
            *ptr = c; // Écriture directe, zéro multiplication !

            if (x1 == x2 && y1 == y2) break;

            int16_t e2 = 2 * err;
            
            // On met à jour le pointeur et les coordonnées
            if (e2 > -dy) { 
                err -= dy; 
                x1 += (x1 < x2 ? 1 : -1); // On garde x1 juste pour le test de fin de boucle
                ptr += sx;                // Déplacement mémoire X
            }
            if (e2 < dx) { 
                err += dx; 
                y1 += (y1 < y2 ? 1 : -1); // On garde y1 juste pour le test de fin de boucle
                ptr += sy;                // Déplacement mémoire Y
            }
        }
    }

    void LineThick(Screen::Info* screen_info, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t w, Screen::Color c)
    {
        // Si épaisseur 1, on utilise la version rapide standard
        if (w <= 1) {
            Line(screen_info, x1, y1, x2, y2, c);
            return;
        }
        
        // Dessine des cercles aux extrémités pour arrondir
        CircleFilled(screen_info, x1, y1, w / 2, c);
        CircleFilled(screen_info, x2, y2, w / 2, c);

        int16_t dx = abs(x2 - x1);
        int16_t dy = abs(y2 - y1);
        
        int16_t sx = (x1 < x2) ? 1 : -1;
        int16_t sy = (y1 < y2) ? 1 : -1;
        
        int16_t err = dx - dy;

        // Calcul du décalage pour centrer l'épaisseur
        // Si thickness = 3, offset = 1 (on dessine à -1, 0, +1 du centre)
        int16_t t_offset = w / 2;

        while (true)
        {
            // --- C'est ici que ça change ---
            // Au lieu de dessiner un pixel, on dessine une "tranche"
            
            if (dx > dy) {
                // Ligne "Horizontale" -> On dessine une tranche Verticale
                // On vérifie les bornes Y pour ne pas dessiner hors écran
                int16_t y_start = y1 - t_offset;
                // Petit clamp manuel (optimisable)
                if (y_start < 0) y_start = 0; 
                if (y_start + w <= screen_info->height) {
                    Vline(screen_info, x1, y_start, w, c);
                }
            } else {
                // Ligne "Verticale" -> On dessine une tranche Horizontale
                int16_t x_start = x1 - t_offset;
                if (x_start < 0) x_start = 0;
                if (x_start + w <= screen_info->width) {
                    Hline(screen_info, x_start, y1, w, c);
                }
            }

            if (x1 == x2 && y1 == y2) break;
            
            int16_t e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x1 += sx; }
            if (e2 < dx) { err += dx; y1 += sy; }
        }
    }

    void Hline(Screen::Info* screen_info, uint16_t x, uint16_t y, uint16_t l, Screen::Color c)
    {
        bool* ptr = &screen_info->data[y * screen_info->width + x];
        memset(ptr, c, l);
    }

    void Vline(Screen::Info* screen_info, uint16_t x, uint16_t y, uint16_t l, Screen::Color c)
    {
        for (uint16_t i = 0; i < l; i++)
        {
            screen_info->data[(y + i) * screen_info->width + x] = c;
        }
    }

    void RectFilled(Screen::Info* screen_info, uint16_t x, uint16_t y, uint16_t w, uint16_t h, Screen::Color c)
    {
        for (uint16_t i = 0; i < h; i++)
        {
            for (uint16_t j = 0; j < w; j++)
            {
                screen_info->data[(y + i) * screen_info->width + (x + j)] = c;
            }
        }
    }

    void RectRounded(Screen::Info* screen_info, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, Screen::Color c)
    {
        // 1. Sécurité : Le rayon ne peut pas être plus grand que la moitié de la largeur ou hauteur
        if (r > w / 2) r = w / 2;
        if (r > h / 2) r = h / 2;

        // 2. DESSIN DU CORPS CENTRAL
        // On dessine un gros rectangle au milieu qui couvre toute la hauteur sauf les arrondis
        // Cela va de y+r à y+h-r.
        // Note : Utilise une fonction FillRect basique ici qui appelle des Hline en boucle
        for (int16_t i = 0; i < h - 2 * r; i++) {
            Hline(screen_info, x, y + r + i, w, c);
        }

        // 3. DESSIN DES CHAPEAUX (Haut et Bas) avec Bresenham
        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x_off = 0;
        int16_t y_off = r;

        while (x_off < y_off)
        {
            if (f >= 0)
            {
                y_off--;
                ddF_y += 2;
                f += ddF_y;
            }
            x_off++;
            ddF_x += 2;
            f += ddF_x;

            // C'est ici l'astuce : on dessine 4 lignes horizontales à chaque étape du calcul
            // Ces lignes relient le coin gauche au coin droit.
            
            // --- Partie A : Les lignes proches du sommet (et du bas absolu) ---
            // Largeur = Largeur totale - 2*rayon + partie interne du cercle (2*x_off)
            // Pour être sûr des maths : Longueur = w - 2*r + 2*x_off
            
            // Ligne du HAUT (vers le sommet)
            Hline(screen_info, x + r - x_off, y + r - y_off, w - 2 * r + 2 * x_off, c);
            // Ligne du BAS (vers le fond)
            Hline(screen_info, x + r - x_off, y + h - r + y_off - 1, w - 2 * r + 2 * x_off, c);

            // --- Partie B : Les lignes proches du corps central ---
            // Ligne du HAUT (juste au dessus du corps central)
            Hline(screen_info, x + r - y_off, y + r - x_off, w - 2 * r + 2 * y_off, c);
            // Ligne du BAS (juste en dessous du corps central)
            Hline(screen_info, x + r - y_off, y + h - r + x_off - 1, w - 2 * r + 2 * y_off, c);
        }
    }

    void CircleFilled(Screen::Info* screen_info, uint16_t x0, uint16_t y0, uint16_t r, Screen::Color c)
    {
        int16_t x = 0;
        int16_t y = r;
        int16_t d = 3 - 2 * r; // Variable de décision (Bresenham)

        while (y >= x)
        {
            // On dessine des lignes horizontales pour remplir le cercle
            // On profite de la symétrie à 4 axes pour tout faire en une boucle
            
            // Ligne centrale et ses voisines (haut/bas) - Partie large du cercle
            // Relie (x0 - y, y0 - x) à (x0 + y, y0 - x)
            Hline(screen_info, x0 - y, y0 - x, 2 * y + 1, c);
            // Relie (x0 - y, y0 + x) à (x0 + y, y0 + x)
            Hline(screen_info, x0 - y, y0 + x, 2 * y + 1, c);

            // Les "chapeaux" (haut et bas extrêmes) - Partie étroite du cercle
            // Relie (x0 - x, y0 - y) à (x0 + x, y0 - y)
            Hline(screen_info, x0 - x, y0 - y, 2 * x + 1, c);
            // Relie (x0 - x, y0 + y) à (x0 + x, y0 + y)
            Hline(screen_info, x0 - x, y0 + y, 2 * x + 1, c);

            // Magie de Bresenham : mise à jour des coordonnées sans float ni racine carrée
            x++;
            if (d > 0)
            {
                y--;
                d = d + 4 * (x - y) + 10;
            }
            else
            {
                d = d + 4 * x + 6;
            }
        }
    }

    void TriangleFilled(Screen::Info* screen_info, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, Screen::Color c)
    {
        // 1. Tri des points par Y croissant (y0 en haut, y2 en bas)
        // Tri à bulle manuel simple
        if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
        if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
        if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }

        // Clipping vertical basique : si le triangle est hors écran, on sort
        if (y2 < 0 || y0 >= screen_info->height) return;

        // 2. Initialisation des pentes (Fixed Point 16.16)
        // On utilise int32_t pour stocker la coordonnée X avec une précision sub-pixel
        // << 16 équivaut à multiplier par 65536
        
        int32_t dx01 = 0, dx02 = 0, dx12 = 0;

        // Pente entre P0 et P1
        if (y1 > y0) dx01 = ((int32_t)(x1 - x0) << 16) / (y1 - y0);
        // Pente entre P0 et P2 (le côté long)
        if (y2 > y0) dx02 = ((int32_t)(x2 - x0) << 16) / (y2 - y0);
        // Pente entre P1 et P2
        if (y2 > y1) dx12 = ((int32_t)(x2 - x1) << 16) / (y2 - y1);

        // 3. Remplissage
        
        // --- Partie Haute (P0 vers P1) ---
        // On a deux curseurs X qui partent de x0
        int32_t xa = (int32_t)x0 << 16;
        int32_t xb = xa;

        // Si y1 == y0, on saute la partie haute (triangle à sommet plat)
        for (int16_t y = y0; y < y1; y++)
        {
            if (y >= 0 && y < screen_info->height) {
                // On ramène les coordonnées fixes vers des entiers normaux (>> 16)
                int16_t x_start = xa >> 16;
                int16_t x_end   = xb >> 16;
                
                // On s'assure que start < end pour Hline
                if (x_start > x_end) std::swap(x_start, x_end);
                
                // Clipping horizontal manuel avant d'appeler Hline
                // (Car ta Hline utilise memset qui peut crash si x < 0)
                if (x_start < 0) x_start = 0;
                if (x_end >= screen_info->width) x_end = screen_info->width - 1;

                if (x_end >= x_start) {
                    Hline(screen_info, x_start, y, x_end - x_start + 1, c);
                }
            }
            // On avance les curseurs selon la pente
            xa += dx02; // Le côté long
            xb += dx01; // Le côté court du haut
        }

        // --- Partie Basse (P1 vers P2) ---
        // Le curseur 'xa' continue sa route sur la longue ligne P0->P2
        // Le curseur 'xb' repart de x1 et suit la ligne P1->P2
        xb = (int32_t)x1 << 16;

        for (int16_t y = y1; y <= y2; y++)
        {
             if (y >= 0 && y < screen_info->height) {
                int16_t x_start = xa >> 16;
                int16_t x_end   = xb >> 16;
                
                if (x_start > x_end) std::swap(x_start, x_end);
                
                if (x_start < 0) x_start = 0;
                if (x_end >= screen_info->width) x_end = screen_info->width - 1;

                if (x_end >= x_start) {
                    Hline(screen_info, x_start, y, x_end - x_start + 1, c);
                }
            }
            xa += dx02; // Continue sur le côté long
            xb += dx12; // Suit le côté court du bas
        }
    }

    void Blit(Screen::Info* screen_info, uint16_t x, uint16_t y, Screen::Info* blit_info)
    {
        for (uint16_t i = 0; i < blit_info->height; i++)
        {
            for (uint16_t j = 0; j < blit_info->width; j++)
            {
                screen_info->data[(y + i) * screen_info->width + (x + j)] = blit_info->data[i * blit_info->width + j];
            }
        }
    }

    void Text(Screen::Info* screen_info, uint16_t x, uint16_t y, const char* text, Screen::Color color)
    {
        uint16_t text_length = strlen(text);
        for (uint16_t i = 0; i < text_length; i++)
        {
            uint8_t c = text[i];
            for (uint16_t j = 0; j < 8; j++)
            {
                for (uint16_t k = 0; k < 8; k++)
                {
                    screen_info->data[(y + j) * screen_info->width + (x + i * 8 + k)] = (font8x8_basic[c][j] & (1 << k)) ? color : !color;
                }
            }
        }
    }

    uint16_t GetTextWidth(const char* text)
    {
        return strlen(text) * 8;
    }
}