INCLUDE_DIRECTORIES(${CAMSENSE_SOURCE_DIR}/dep/freeglut/include
                    ${CAMSENSE_SOURCE_DIR}/dep/lidar/include
                    ${CAMSENSE_SOURCE_DIR}/src/base
                    )

LINK_DIRECTORIES(${CAMSENSE_SOURCE_DIR}/dep/freeglut/lib
                 ${CAMSENSE_SOURCE_DIR}/dep/lidar/lib
                 ${CAMSENSE_SOURCE_DIR}/src/base
                 )

ADD_SUBDIRECTORY(base)

ADD_EXECUTABLE (sdk_demo sdk_demo.cpp)
TARGET_LINK_LIBRARIES (sdk_demo lidar)

ADD_EXECUTABLE(rangeView rangeView.cpp)
TARGET_LINK_LIBRARIES(rangeView freeglut lidar)