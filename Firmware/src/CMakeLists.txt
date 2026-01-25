# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(SRCS ${app_sources}
                       EMBED_TXTFILES "certs/root_ca.pem")
                    #    REQUIRES esp_littlefs)

# # Build LittleFS image if the command is available
# if(COMMAND littlefs_create_partition_image)
#     littlefs_create_partition_image(storage "../data" FLASH_IN_PROJECT)
# endif()