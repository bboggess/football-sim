cmake_minimum_required(VERSION 3.14)
project(fb VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Armadillo REQUIRED)
find_package(MLPACK REQUIRED)
find_package(OpenMP REQUIRED)

set(SRC_DIR src)
include_directories(${SRC_DIR})

set(LEARN_DIR ${SRC_DIR}/learn)
set(ENGINE_DIR ${SRC_DIR}/engine)

set(MODEL_TRAIN_SRC ${LEARN_DIR}/train.cpp)
set(MODEL_LIB_SRC ${LEARN_DIR}/classify.cpp)
set(TRAINING_SET ${LEARN_DIR}/training_set.csv)
set(ENGINE_SRC ${ENGINE_DIR}/clock.cpp ${ENGINE_DIR}/game.cpp ${ENGINE_DIR}/gamestates.cpp ${ENGINE_DIR}/play.cpp ${ENGINE_DIR}/team.cpp ${ENGINE_DIR}/userteam.cpp ${ENGINE_DIR}/utils.cpp)

set(TRAIN_BIN playcall-train)
set(MODEL_LIB playcall-learn-lib)
set(ENGINE_LIB fb-engine)
set(DRIVER_BIN driver)

set(MLPACK_LIBS mlpack boost_serialization ${ARMADILLO_LIBRARIES} OpenMP::OpenMP_CXX)

add_library(${ENGINE_LIB} STATIC ${ENGINE_SRC})

add_executable(${DRIVER_BIN} ${SRC_DIR}/main.cpp)
target_link_libraries(${DRIVER_BIN} PUBLIC ${ENGINE_LIB} ${MODEL_LIB})

add_library(${MODEL_LIB} STATIC ${MODEL_LIB_SRC})
include_directories(${ARMADILLO_INCLUDE_DIRS})
target_link_libraries(${MODEL_LIB} PUBLIC ${MLPACK_LIBS})

add_executable(${TRAIN_BIN} ${MODEL_TRAIN_SRC})
add_custom_command(
    TARGET ${TRAIN_BIN} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/${TRAINING_SET}
    ${CMAKE_BINARY_DIR}/training_set.csv
)
add_custom_target(train-model
    COMMAND ${TRAIN_BIN} training_set.csv
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "train playcall model"
    SOURCES ${MODEL_TRAIN_SRC}
)
target_link_libraries(${TRAIN_BIN} PUBLIC ${MLPACK_LIBS})

add_dependencies(${DRIVER_BIN} train-model)
