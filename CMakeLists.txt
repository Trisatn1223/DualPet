CU_SET_PATH("CMAKE_MOD_ANIMAL_COMPANION_DIR" "${CMAKE_CURRENT_LIST_DIR}")

AC_ADD_SCRIPT("${CMAKE_MOD_ANIMAL_COMPANION_DIR}/src/animal_companion.cpp")

message(STATUS "Animal Companion module loaded")
