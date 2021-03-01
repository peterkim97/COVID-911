cmake_minimum_required(VERSION 3.0.2)
project(motor_final)
find_package(catkin REQUIRED COMPONENTS std_msgs roscpp)
catkin_package( 
  INCLUDE_DIRS include
  LIBRARIES motor_final
  CATKIN_DEPENDS std_msgs roscpp )
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME}
  src/main.cpp
  src/motor_driver.cpp
  /usr/include/i2c/smbus.h
  /usr/include/aarch64-linux-gnu/sys/socket.h
  /usr/include/netinet/in.h
  /usr/include/arpa/inet.h
)
add_executable(motor_distance src/main.cpp src/motor_driver.cpp)
add_dependencies(motor_distance ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS}) 
target_link_libraries(motor_distance 
 /usr/lib/aarch64-linux-gnu/libi2c.so
${catkin_LIBRARIES})

