
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was kfleeConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

# kflee CMake 包配置
#
# 导出所有 kflee::xxx 目标，下游项目通过 find_package(kflee) 使用。
#
# 使用示例：
#   find_package(kflee REQUIRED)
#   target_link_libraries(my_app PRIVATE kflee::pipeline kflee::tracker)

include(CMakeFindDependencyMacro)

# OpenCV: libprep / libcropper / libdisplay 依赖
if(NOT TARGET ${OpenCV_LIBS})
    find_dependency(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs videoio highgui)
endif()

# ONNX Runtime: libdnn_onnx 依赖
if(NOT TARGET onnxruntime::onnxruntime)
    find_dependency(onnxruntime QUIET)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/kfleeTargets.cmake")
