cmake_minimum_required(VERSION 3.13)
project(kitchensound)

set(CMAKE_CXX_STANDARD 17)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

find_path(LOCAL_INCLUDES NAMES libconfig.h++ HINTS /usr/local/include REQUIRED)
message(STATUS "LOCAL_INCLUDES => ${LOCAL_INCLUDES}")

find_path(SDL2_INCLUDE NAMES SDL.h HINTS /usr/local/include/SDL2 REQUIRED)
message(STATUS "SDL2_INCLUDE => ${SDL2_INCLUDE}")

find_path(CURL_INCLUDE NAMES curl/curl.h HINTS /usr/include/arm-linux-gnueabihf REQUIRED)
message(STATUS "CURL_INCLUDE => ${CURL_INCLUDE}")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frecord-gcc-switches")

add_executable(kitchensound src/main.cpp
        src/renderer.cpp
        src/input.cpp
        src/config.cpp
        src/time_based_standby.cpp
        src/mpd_controller.cpp
        src/state_controller.cpp
        src/bt_controller.cpp
        src/volume.cpp
        src/cache_manager.cpp
        src/render_text.cpp
        src/resource_manager.cpp
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
        src/version.cpp
        src/file_playback.cpp
        src/sdl_util.cpp
        src/gpio_util.cpp
        src/init_system_conns.cpp
        src/os_util.cpp
        src/timer.cpp src/timer_manager.cpp)

target_include_directories(kitchensound PUBLIC include)
target_include_directories(kitchensound PUBLIC ${LOCAL_INCLUDES})
target_include_directories(kitchensound PUBLIC ${SDL2_INCLUDE})
target_include_directories(kitchensound PUBLIC ${CURL_INCLUDE})
target_link_libraries(kitchensound stdc++fs)
target_link_libraries(kitchensound SDL2)
target_link_libraries(kitchensound SDL2_ttf)
target_link_libraries(kitchensound SDL2_image)
target_link_libraries(kitchensound ao)
target_link_libraries(kitchensound mpg123)
target_link_libraries(kitchensound config++)
target_link_libraries(kitchensound mpdclient)
target_link_libraries(kitchensound Threads::Threads)
target_link_libraries(kitchensound sdbus-c++)
target_link_libraries(kitchensound asound)
target_link_libraries(kitchensound curlpp)
target_link_libraries(kitchensound curl)
target_link_libraries(kitchensound gpiod)