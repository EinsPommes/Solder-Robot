cmake_minimum_required(VERSION 3.16)
project(SolderRobot VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt-Komponenten
find_package(Qt6 COMPONENTS 
    Core
    Gui
    Widgets
    SerialPort
    Network
    WebEngineWidgets
    REQUIRED
)

# OpenCV
find_package(OpenCV REQUIRED)

# Zusätzliche Bibliotheken
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(OpenSSL REQUIRED)

set(SOURCES
    src/main.cpp
    src/gui.cpp
    src/motion_controller.cpp
    src/sensor_manager.cpp
    src/temperature_control.cpp
    src/program_manager.cpp
    src/vision_system.cpp
    src/quality_control.cpp
    src/maintenance_system.cpp
    src/network_manager.cpp
    src/energy_manager.cpp
    src/security_system.cpp
    src/data_logger.cpp
)

set(HEADERS
    include/gui.h
    include/motion_controller.h
    include/sensor_manager.h
    include/temperature_control.h
    include/program_manager.h
    include/vision_system.h
    include/quality_control.h
    include/maintenance_system.h
    include/network_manager.h
    include/energy_manager.h
    include/security_system.h
    include/data_logger.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::Network
    Qt6::WebEngineWidgets
    ${OpenCV_LIBS}
    Boost::system
    Boost::filesystem
    OpenSSL::SSL
    OpenSSL::Crypto
)
