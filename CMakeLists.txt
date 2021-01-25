cmake_minimum_required(VERSION 3.13)
project(kitchensound)

set(CMAKE_CXX_STANDARD 17)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)

find_package(SDL2 REQUIRED)

find_package(sdbus-c++ REQUIRED)

include(FindPkgConfig)
pkg_check_modules(ALSA REQUIRED alsa)

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
        src/pages/menu_selection_page.cpp src/pages/options_page.cpp src/pages/base_page.cpp src/pages/volume_page.cpp src/pages/loading_page.cpp src/pages/inactive_page.cpp src/pages/page_loader.cpp)

target_include_directories(kitchensound PUBLIC include)
target_include_directories(kitchensound PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(kitchensound PUBLIC ${ALSA_INCLUDE_DIRS})
target_link_libraries(kitchensound stdc++fs)
target_link_libraries(kitchensound ${SDL2_LIBRARIES})
target_link_libraries(kitchensound SDL2_ttf)
target_link_libraries(kitchensound SDL2_image)
target_link_libraries(kitchensound config++)
target_link_libraries(kitchensound mpdclient)
target_link_libraries(kitchensound Threads::Threads)
target_link_libraries(kitchensound SDBusCpp::sdbus-c++)
target_link_libraries(kitchensound ${ALSA_LIBRARIES})
target_link_libraries(kitchensound curlpp)
target_link_libraries(kitchensound curl)
target_link_libraries(kitchensound spdlog::spdlog)
target_link_libraries(kitchensound wiringPi)