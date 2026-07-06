#include "ui/menus/Update.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "Robot.hpp"

MenuUpdate::MenuUpdate(Menu* parent)
    : Menu("Update", parent, Icons::UpdateMenu)
{
}

bool MenuUpdate::onBack()
{
    return false;
}

bool MenuUpdate::onSelect()
{
    UpdateManager& updt = Robot::GetInstance().getNetworkManager().getUpdateManager();
    if (updt.getStatus() == UpdateManager::Status::Done && updt.isUpdateAvailable())
    {
        // install update
        updt.startUpdate();
        return true;
    }

    return false;
}

bool MenuUpdate::onNext()
{
    return false;
}

bool MenuUpdate::onPrev()
{
    return false;
}


void MenuUpdate::onShow()
{
    UpdateManager& updt = Robot::GetInstance().getNetworkManager().getUpdateManager();
    updt.checkForUpdate();
}

void MenuUpdate::onHide()
{ 
}

void MenuUpdate::onRender()
{
    renderHeader();

    UpdateManager& updt = Robot::GetInstance().getNetworkManager().getUpdateManager();
    switch (updt.getStatus())
    {
        case UpdateManager::Status::Done : {
            uint16_t height = 8;
            if (updt.isUpdateAvailable())
            {
                const char* text = "New update!";
                const char* text_ver = updt.getLatestVersion().c_str();
                uint16_t text_width = Draw::GetTextWidth(text);
                uint16_t ver_width = Draw::GetTextWidth(text_ver);
                Draw::RectRounded(
                    ScreenDriver::info.width / 2 - ver_width / 2 - 4, HEADER_HEIGHT + 4 + 12,
                    ver_width + 8, height + 4, 2
                );
                Draw::Text(
                    ScreenDriver::info.width / 2 - ver_width / 2, HEADER_HEIGHT + 4 + 14,
                    text_ver, ScreenDriver::COLOR_BLACK
                );
                Draw::Text(ScreenDriver::info.width / 2 - text_width / 2, HEADER_HEIGHT + 4, text);

                const char* inst_txt = "install";
                uint16_t inst_txt_width = Draw::GetTextWidth(inst_txt);
                Draw::Text(ScreenDriver::info.width - 12 - inst_txt_width, ScreenDriver::info.height - height - 1, inst_txt);
                Draw::Blit(ScreenDriver::info.width - 8 - 1, ScreenDriver::info.height - 8 - 1, 8, 8, (uint8_t*)Icons::ChevronRight);
            }
            else
            {
                const char* text = "No update :(";
                uint16_t text_width = Draw::GetTextWidth(text);
                Draw::Text(ScreenDriver::info.width / 2 - text_width / 2, ScreenDriver::info.height * 0.5f - height / 2, text);
            }
            break;
        }
        case UpdateManager::Status::FetchingUpdate : {
            const char* text = "Fetching";
            uint16_t width = Draw::GetTextWidth(text);
            uint16_t height = 8;
            Draw::RectRounded(
                ScreenDriver::info.width / 2 - width / 2 - 4, ScreenDriver::info.height / 2 - height / 2 - 4,
                width + 8, height + 8, 4
            );
            Draw::Text(
                ScreenDriver::info.width / 2 - width / 2, ScreenDriver::info.height / 2 - height / 2,
                text, ScreenDriver::COLOR_BLACK
            );
            break;
        }
        case UpdateManager::Status::DownloadingFirmware : {
            {
                const char* text = "Downloading";
                uint16_t width = Draw::GetTextWidth(text);
                Draw::Text(ScreenDriver::info.width / 2 - width / 2, HEADER_HEIGHT + 4, text);
            }
            {
                const char* text = "Firmware";
                uint16_t width = Draw::GetTextWidth(text);
                Draw::Text(ScreenDriver::info.width / 2 - width / 2, HEADER_HEIGHT + 16, text);
            }
            Draw::RectRounded(8, HEADER_HEIGHT + 30, ScreenDriver::info.width - 16, 16, 4);
            Draw::RectRounded(9, HEADER_HEIGHT + 31, ScreenDriver::info.width - 18, 14, 3, ScreenDriver::COLOR_BLACK);
            Draw::RectRounded(9, HEADER_HEIGHT + 31, (ScreenDriver::info.width - 18) * updt.getProgress(), 14, 3);
            break;
        }
        case UpdateManager::Status::UpdatingFirmware : {
            {
                const char* text = "Updating";
                uint16_t width = Draw::GetTextWidth(text);
                Draw::Text(ScreenDriver::info.width / 2 - width / 2, HEADER_HEIGHT + 4, text);
            }
            {
                const char* text = "Firmware";
                uint16_t width = Draw::GetTextWidth(text);
                Draw::Text(ScreenDriver::info.width / 2 - width / 2, HEADER_HEIGHT + 16, text);
            }
            Draw::RectRounded(8, HEADER_HEIGHT + 30, ScreenDriver::info.width - 16, 16, 4);
            Draw::RectRounded(9, HEADER_HEIGHT + 31, ScreenDriver::info.width - 18, 14, 3, ScreenDriver::COLOR_BLACK);
            Draw::RectRounded(9, HEADER_HEIGHT + 31, (ScreenDriver::info.width - 18) * updt.getProgress(), 14, 3);
            break;
        }
        case UpdateManager::Status::DownloadingFilesystem : {
            {
                const char* text = "Downloading";
                uint16_t width = Draw::GetTextWidth(text);
                Draw::Text(ScreenDriver::info.width / 2 - width / 2, HEADER_HEIGHT + 4, text);
            }
            {
                const char* text = "Filesystem";
                uint16_t width = Draw::GetTextWidth(text);
                Draw::Text(ScreenDriver::info.width / 2 - width / 2, HEADER_HEIGHT + 16, text);
            }
            Draw::RectRounded(8, HEADER_HEIGHT + 30, ScreenDriver::info.width - 16, 16, 4);
            Draw::RectRounded(9, HEADER_HEIGHT + 31, ScreenDriver::info.width - 18, 14, 3, ScreenDriver::COLOR_BLACK);
            Draw::RectRounded(9, HEADER_HEIGHT + 31, (ScreenDriver::info.width - 18) * updt.getProgress(), 14, 3);
            break;
        }
        case UpdateManager::Status::UpdatingFilesystem : {
            {
                const char* text = "Updating";
                uint16_t width = Draw::GetTextWidth(text);
                Draw::Text(ScreenDriver::info.width / 2 - width / 2, HEADER_HEIGHT + 4, text);
            }
            {
                const char* text = "Filesystem";
                uint16_t width = Draw::GetTextWidth(text);
                Draw::Text(ScreenDriver::info.width / 2 - width / 2, HEADER_HEIGHT + 16, text);
            }
            Draw::RectRounded(8, HEADER_HEIGHT + 30, ScreenDriver::info.width - 16, 16, 4);
            Draw::RectRounded(9, HEADER_HEIGHT + 31, ScreenDriver::info.width - 18, 14, 3, ScreenDriver::COLOR_BLACK);
            Draw::RectRounded(9, HEADER_HEIGHT + 31, (ScreenDriver::info.width - 18) * updt.getProgress(), 14, 3);
            break;
        }
        case UpdateManager::Status::Rebooting : {
            uint16_t height = 8;
            const char* text = "Rebooting ...";
            uint16_t width = Draw::GetTextWidth(text);
            Draw::Text(ScreenDriver::info.width / 2 - width / 2, ScreenDriver::info.height / 2 - height / 2, text);
            break;
        }
        case UpdateManager::Status::ErrorUnreachable : {
            const char* text = "No internet";
            uint16_t width = Draw::GetTextWidth(text);
            uint16_t height = 8;
            Draw::Text(ScreenDriver::info.width / 2 - width / 2, ScreenDriver::info.height / 2 - height / 2 - 6, text);
            const char* text2 = "connection";
            uint16_t width2 = Draw::GetTextWidth(text2);
            uint16_t height2 = 8;
            Draw::Text(ScreenDriver::info.width / 2 - width2 / 2, ScreenDriver::info.height / 2 - height2 / 2 + 6, text2);
            break;
        }
        case UpdateManager::Status::ErrorInvalidJson :
        case UpdateManager::Status::ErrorEmptyResponse : {
            const char* text = "Server error";
            uint16_t width = Draw::GetTextWidth(text);
            uint16_t height = 8;
            Draw::Text(ScreenDriver::info.width / 2 - width / 2, ScreenDriver::info.height / 2 - height / 2, text);
            break;
        }
        default: {
            const char* text = "Error :(";
            uint16_t width = Draw::GetTextWidth(text);
            uint16_t height = 8;
            Draw::Text(ScreenDriver::info.width / 2 - width / 2, ScreenDriver::info.height / 2 - height / 2, text);
            break;
        }
    }
}

void MenuUpdate::onUpdate()
{
    triggerRender();
}
