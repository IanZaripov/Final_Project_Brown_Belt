cmake_minimum_required(VERSION 3.15)
project(TransportRouter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Исходные файлы
set(SOURCES
    main.cpp
    StopsBase.cpp
    Bus.cpp
    Transport.cpp
    json.cpp
)

# Заголовочные файлы
set(HEADERS
    StopsBase.h
    Bus.h
    Transport.h
    json.h
    graph.h
    router.h
)

# Создаем исполняемый файл
add_executable(transport_router ${SOURCES} ${HEADERS})

# Настройки компилятора
if(MSVC)
    target_compile_options(transport_router PRIVATE /W4)
else()
    target_compile_options(transport_router PRIVATE -Wall -Wextra)
endif()