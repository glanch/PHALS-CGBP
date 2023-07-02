cmake_minimum_required(VERSION 3.0.0)
project(PHALS VERSION 0.0.1)

find_package(SCIP REQUIRED)
include_directories(${SCIP_INCLUDE_DIRS})

include(CTest)
enable_testing()

#add every .cpp - file
add_executable(PHALS
    main.cpp
    compact/CompactModel.cpp
    convexification/Master.cpp
    convexification/Pricer.cpp
    convexification/SubProblem.cpp
    convexification/SubProblemInitialColumns.cpp
    Instance.cpp
)

target_link_libraries(PHALS ${SCIP_LIBRARIES} stdc++fs)

if( TARGET examples )
    add_dependencies( examples dicbap )
endif()

enable_testing()


