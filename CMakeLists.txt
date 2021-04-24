cmake_minimum_required(VERSION 3.13)
project(kitchensound)

set(CMAKE_CXX_STANDARD 17)

find_path(LOCAL_INCLUDES NAMES libconfig.h++ HINTS /usr/local/include REQUIRED)
message(STATUS "LOCAL_INCLUDES => ${LOCAL_INCLUDES}")

find_path(SYSTEM_INCLUDE NAMES curl/curl.h HINTS /usr/include/arm-linux-gnueabihf REQUIRED)
message(STATUS "SYSTEM_INCLUDE => ${SYSTEM_INCLUDE}")

find_path(FLATBUFFERS_INCLUDE NAMES flatbuffers/flatbuffers.h HINTS /usr/include REQUIRED)
message(STATUS "FLATBUFFERS_INCLUDE => ${FLATBUFFERS_INCLUDE}")

find_path(SYSTEM_INCLUDE2 NAMES spdlog/spdlog.h HINTS /usr/include REQUIRED)
message(STATUS "SYSTEM_INCLUDE2 => ${SYSTEM_INCLUDE2}")

add_executable(kitchensound src/main.cpp
        src/version.cpp
        src/fd_registry.cpp
        src/renderer.cpp
        src/render_text.cpp
        src/input.cpp
        src/config.cpp
        src/timer.cpp
        src/cache_manager.cpp
        src/resource_manager.cpp
        src/state_controller.cpp
        src/application_backbone.cpp
        src/time_based_standby.cpp
        src/volume.cpp
        src/mpd_controller.cpp
        src/bt_controller.cpp
        src/snapcast_controller.cpp
        src/file_playback.cpp
        src/sdl_util.cpp
        src/gpio_util.cpp
        src/os_util.cpp
        src/init_system_conns.cpp
        src/pages/bt_playing_page.cpp
        src/pages/station_playing_page.cpp
        src/pages/station_selection_page.cpp
        src/pages/selection_page.cpp
        src/pages/menu_selection_page.cpp
        src/pages/options_page.cpp
        src/pages/base_page.cpp
        src/pages/volume_page.cpp
        src/pages/loading_page.cpp
        src/pages/inactive_page.cpp
        src/pages/page_loader.cpp
        src/pages/playing_page.cpp
        src/pages/snapcast_playing_page.cpp
        src/analytics_logger.cpp src/network_manager.cpp src/song_faver.cpp src/pages/pages.cpp src/song.cpp)

target_include_directories(kitchensound PUBLIC include)
target_include_directories(kitchensound PUBLIC ${LOCAL_INCLUDES})
target_include_directories(kitchensound PUBLIC ${SYSTEM_INCLUDE})
target_include_directories(kitchensound PUBLIC ${SYSTEM_INCLUDE2})
target_link_directories(kitchensound PUBLIC /usr/local/lib)
target_link_libraries(kitchensound stdc++fs)
target_link_libraries(kitchensound SDL2)
target_link_libraries(kitchensound SDL2_ttf)
target_link_libraries(kitchensound SDL2_image)
target_link_libraries(kitchensound ao)
target_link_libraries(kitchensound mpg123)
target_link_libraries(kitchensound config++)
target_link_libraries(kitchensound mpdclient)
target_link_libraries(kitchensound systemd)
target_link_libraries(kitchensound asound)
target_link_libraries(kitchensound curl)
target_link_libraries(kitchensound gpiod)
target_link_libraries(kitchensound flatbuffers)
