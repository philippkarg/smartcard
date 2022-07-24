cmake_minimum_required(VERSION 3.5)

# Debug Postfix
set(CMAKE_DEBUG_POSTFIX d)
option(Debug "Build a debug executable." OFF)
option(Masking "Enable masking of the AES alogrithm on the card." OFF)
option(Shuffling "Enable shuffling of S-Box accesses of the AES alogrithm on the card." OFF)
option(DummyOps "Enable dummy NOPs on the card." OFF)

# Variables regarding the AVR chip
set(MCU   atmega644)
set(F_CPU 4800000UL)
set(BAUD  9600UL)
set(PROG_TYPE avrisp2)
add_definitions(-DF_CPU=${F_CPU})

# program names
set(AVRCPP   avr-g++)
set(AVRC     avr-gcc)
set(AVRSTRIP avr-strip)
set(OBJCOPY  avr-objcopy)
set(OBJDUMP  avr-objdump)
set(AVRSIZE  avr-size)
set(AVRDUDE  avrdude)
set(AVRDUDE_PART "m644")

# Sets the compiler
# Needs to come before the project function
set(CMAKE_SYSTEM_NAME   Generic)
set(CMAKE_CXX_COMPILER  ${AVRCPP})
set(CMAKE_C_COMPILER    ${AVRC})
set(CMAKE_ASM_COMPILER  ${AVRC})

project (atmega644 CXX)

# Important project paths
set(BASE_PATH    "${CMAKE_CURRENT_LIST_DIR}")
set(INC_PATH     "${BASE_PATH}/include" "${BASE_PATH}/include/aes")
set(LIB_DIR_PATH "${BASE_PATH}/lib")

# Sources
set(SRC_FILES   "${CMAKE_CURRENT_LIST_DIR}/src/main.cpp"
                "${CMAKE_CURRENT_LIST_DIR}/src/communication.cpp"
                "${CMAKE_CURRENT_LIST_DIR}/src/aes/aes.cpp"
                "${CMAKE_CURRENT_LIST_DIR}/src/aes/aesMath.cpp"
)

# Some additional defintions
# Adding DEBUG definitions
if(Debug)
    set(CMAKE_BUILD_TYPE "Debug")
    message(STATUS "[INFO]: Currently compiling in debug mode.")
    list(APPEND SRC_FILES "${CMAKE_CURRENT_LIST_DIR}/src/logger.cpp")
    add_compile_definitions("DEBUG")
else()
    set(CMAKE_BUILD_TYPE "Release")
    message(STATUS "[INFO]: Currently compiling in release mode.")
endif()

# Adding MASKING definitions
if(Masking)
    message(STATUS "[INFO]: Masking of the AES algorithm is enabled.")
    add_compile_definitions("MASKING")
    list(APPEND SRC_FILES "${CMAKE_CURRENT_LIST_DIR}/src/aes/masking.cpp")
else()
    message(STATUS "[INFO]: Masking of the AES algorithm is disabled.")
endif()

# Addusing SHUFFLING definitions
if(Shuffling)
    message(STATUS "[INFO]: Shuffling of S-Box accesses of the AES algorithm is enabled.")
    add_compile_definitions("SHUFFLING")
else()
    message(STATUS "[INFO]: Shuffling of S-Box accesses of the AES algorithm is disabled.")
endif()

# Adding DUMMYOPS definitions
if(DummyOps)
    message(STATUS "[INFO]: Dummy NOPs are enabled.")
    add_compile_definitions("DUMMY_OPS")
else()
    message(STATUS "[INFO]: Dummy NOPs are disabled.")
endif()

if(Masking OR Shuffling OR DummyOps)
    list(APPEND SRC_FILES "${CMAKE_CURRENT_LIST_DIR}/src/aes/rng.cpp")
endif()

if(Shuffling OR DummyOps)
    list(APPEND SRC_FILES "${CMAKE_CURRENT_LIST_DIR}/src/aes/hiding.cpp")
endif()


# Files to be compiled
set(LIB_SRC_FILES)
set(LIB_INC_PATH)
file(GLOB LIBRARIES "${LIB_DIR_PATH}/*")
foreach(subdir ${LIBRARIES})
    file(GLOB lib_files "${subdir}/*.cpp"
                        "${subdir}/*.cc"
                        "${subdir}/*.c"
                        "${subdir}/*.cxx"
                        "${subdir}/*.S"
                        "${subdir}/*.s"
                        "${subdir}/*.sx"
                        "${subdir}/*.asm")
    if(IS_DIRECTORY ${subdir})
        list(APPEND LIB_INC_PATH  "${subdir}")
    endif()
    list(APPEND LIB_SRC_FILES "${lib_files}")
endforeach()

# Compiler flags
set(DEBUG   "-O0 -gstabs -g -ggdb")
set(RELEASE "-Os -lm -lprintf_flt")
set(WARN    "-Wall -Wl,--gc-sections -Wl,--relax")
set(TUNING  "-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -ffunction-sections -fdata-sections")
set(MCU     "-mmcu=${MCU}")
set(DEFS    "-DF_CPU=${F_CPU} -DBAUD=${BAUD}")

set(CMAKE_CXX_FLAGS_RELEASE "${MCU} ${WARN} ${DEFS} ${RELEASE} ${TUNING}")
set(CMAKE_CXX_FLAGS_DEBUG "${MCU} ${WARN} ${DEFS} ${DEBUG} ${TUNING}")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Doxygen
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    add_subdirectory(docs)
else()
    message(STATUS "[INFO] Doxygen not found. Not building documentation.")
endif()

# Executable
add_executable(${PROJECT_NAME} ${SRC_FILES} ${LIB_SRC_FILES} )
target_include_directories(${PROJECT_NAME} PRIVATE ${INC_PATH} ${LIB_INC_PATH})
set_target_properties(${PROJECT_NAME}   PROPERTIES  OUTPUT_NAME "${PROJECT_NAME}.elf")

# Compiling targets
add_custom_target(strip ALL     ${AVRSTRIP} "${PROJECT_NAME}.elf" DEPENDS ${PROJECT_NAME})
add_custom_target(hex   ALL     ${OBJCOPY} -R .eeprom -O ihex "${PROJECT_NAME}.elf" "${PROJECT_NAME}.hex" DEPENDS strip)
add_custom_target(flash         ${AVRDUDE} -p ${AVRDUDE_PART} -P usb -c ${PROG_TYPE} -v -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.hex;${PROJECT_NAME}.eeprom;${PROJECT_NAME}.lst")
