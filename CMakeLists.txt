cmake_minimum_required(VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake_modules/")
include(cotire)
set(DEPS_DIR ${DEPS_DIR} "${CMAKE_SOURCE_DIR}/ref")
project("eu07")

set(CMAKE_CXX_STANDARD 14)
include_directories("." "Console" "McZapkie")
file(GLOB HEADERS "*.h" "Console/*.h" "McZapkie/*.h")

if (APPLE)
	set (CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000")
endif()

option(USE_IMGUI_GL3 "Use OpenGL3+ imgui implementation" OFF)
option(WITH_UART "Compile with libserialport" ON)

set(SOURCES
"Texture.cpp"
"Timer.cpp"
"Track.cpp"
"Traction.cpp"
"TractionPower.cpp"
"Train.cpp"
"TrkFoll.cpp"
"AirCoupler.cpp"
"AnimModel.cpp"
"Button.cpp"
"Camera.cpp"
"Driver.cpp"
"dumb3d.cpp"
"DynObj.cpp"
"EU07.cpp"
"Event.cpp"
"EvLaunch.cpp"
"Float3d.cpp"
"Gauge.cpp"
"Globals.cpp"
"Logs.cpp"
"McZapkie/friction.cpp"
"McZapkie/hamulce.cpp"
"McZapkie/Mover.cpp"
"McZapkie/Oerlikon_ESt.cpp"
"MdlMngr.cpp"
"MemCell.cpp"
"Model3d.cpp"
"mtable.cpp"
"parser.cpp"
"renderer.cpp"
"PyInt.cpp"
"ResourceManager.cpp"
"sn_utils.cpp"
"Segment.cpp"
"sky.cpp"
"sun.cpp"
"stars.cpp"
"lightarray.cpp"
"skydome.cpp"
"sound.cpp"
"Spring.cpp"
"shader.cpp"
"frustum.cpp"
"uilayer.cpp"
"openglmatrixstack.cpp"
"moon.cpp"
"command.cpp"
"keyboardinput.cpp"
"gamepadinput.cpp"
"openglgeometrybank.cpp"
"drivermouseinput.cpp"
"translation.cpp"
"material.cpp"
"lua.cpp"
"stdafx.cpp"
"messaging.cpp"
"scene.cpp"
"scenenode.cpp"
"simulation.cpp"
"vertex.cpp"
"audio.cpp"
"audiorenderer.cpp"
"motiontelemetry.cpp"
"utilities.cpp"
"uitranscripts.cpp"
"station.cpp"
"application.cpp"
"simulationtime.cpp"
"sceneeditor.cpp"
"screenshot.cpp"
"driverkeyboardinput.cpp"
"drivermode.cpp"
"driveruilayer.cpp"
"driveruipanels.cpp"
"editorkeyboardinput.cpp"
"editormode.cpp"
"editormouseinput.cpp"
"editoruilayer.cpp"
"editoruipanels.cpp"
"scenarioloadermode.cpp"
"scenenodegroups.cpp"
"simulationenvironment.cpp"
"simulationstateserializer.cpp"
"precipitation.cpp"
"openglcolor.cpp"
"dictionary.cpp"

"imgui/imgui.cpp"
"imgui/imgui_demo.cpp"
"imgui/imgui_draw.cpp"
"imgui/imgui_widgets.cpp"
"imgui/imgui_impl_glfw.cpp"
)

if (USE_IMGUI_GL3)
	set(SOURCES ${SOURCES} "imgui/imgui_impl_opengl3.cpp")
else()
	add_definitions(-DEU07_USEIMGUIIMPLOPENGL2)
	set(SOURCES ${SOURCES} "imgui/imgui_impl_opengl2.cpp")
endif()

if (WITH_UART)
	add_definitions(-DWITH_UART)
	set(SOURCES ${SOURCES} "uart.cpp")
endif()

set (PREFIX "")

if (WIN32)
	add_definitions(-DHAVE_ROUND) # to make pymath to not redefine round
	add_definitions(-DGLFW_DLL)

	set(SOURCES ${SOURCES} "windows.cpp" "Console.cpp" "Console/LPT.cpp" "Console/PoKeys55.cpp")
	set(GLEW_INCLUDE_DIR ${GLEW_INCLUDE_DIR} "${DEPS_DIR}/glew/include/")
	set(GLFW3_INCLUDE_DIR ${GLFW3_INCLUDE_DIR} "${DEPS_DIR}/glfw/include/")
	set(PNG_INCLUDE_DIRS ${PNG_INCLUDE_DIRS} "${DEPS_DIR}/libpng/include/")
	set(ZLIB_INCLUDE_DIR ${ZLIB_INCLUDE_DIR} "${DEPS_DIR}/zlib/")
	set(GLM_INCLUDE_DIR ${GLM_INCLUDE_DIR} "${DEPS_DIR}/glm/")
	set(OPENAL_INCLUDE_DIR ${OPENAL_INCLUDE_DIR} "${DEPS_DIR}/openal/include")
	set(LIBSNDFILE_INCLUDE_DIR ${LIBSNDFILE_INCLUDE_DIR} "${DEPS_DIR}/libsndfile/include")
	set(LUAJIT_INCLUDE_DIR ${LUAJIT_INCLUDE_DIR} "${DEPS_DIR}/luajit/include")
	set(PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIR} "${DEPS_DIR}/python/include")
	set(libserialport_INCLUDE_DIR ${LIBSERIALPORT_INCLUDE_DIR} "${DEPS_DIR}/libserialport/include")
	
	if(${CMAKE_EXE_LINKER_FLAGS} STREQUAL "/machine:x64")
		set (ARCH "x64")
	else()
		set (ARCH "x86")
	endif()

	set (PREFIX "-${ARCH}")

	set(GLEW_LIBRARY ${GLEW_LIBRARY} "${DEPS_DIR}/glew/lib/${ARCH}/glew32.lib")
	set(GLFW3_LIBRARIES ${GLFW3_LIBRARIES} "${DEPS_DIR}/glfw/lib/${ARCH}/glfw3dll.lib")
	set(PNG_LIBRARIES ${PNG_LIBRARIES} "${DEPS_DIR}/libpng/lib/${ARCH}/libpng16.lib")
	set(OPENAL_LIBRARY ${OPENAL_LIBRARY} "${DEPS_DIR}/openal/lib/${ARCH}/OpenAL32.lib")
	set(LIBSNDFILE_LIBRARY ${LIBSNDFILE_LIBRARY} "${DEPS_DIR}/libsndfile/lib/${ARCH}/libsndfile-1.lib")
	set(LUAJIT_LIBRARIES ${LUAJIT_LIBRARIES} "${DEPS_DIR}/luajit/lib/${ARCH}/lua51.lib")
	set(PYTHON_LIBRARY ${PYTHON_LIBRARY} "${DEPS_DIR}/python/lib/${ARCH}/python27.lib")
	set(libserialport_LIBRARY ${LIBSERIALPORT_LIBRARY} "${DEPS_DIR}/libserialport/lib/${ARCH}/libserialport-0.lib")
endif()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(SOURCES ${SOURCES} "eu07.rc")
	set(SOURCES ${SOURCES} "eu07.ico")
endif()

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	# /wd4996: disable "deprecation" warnings
	# /wd4244: disable warnings for conversion with possible loss of data
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4996 /wd4244")
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/LARGEADDRESSAWARE")
endif()

string(TIMESTAMP data_exe %y%m%d)

set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdb"
	OUTPUT_NAME "${PROJECT_NAME}${PREFIX}_${data_exe}"
	DEBUG_POSTFIX "_d"
)

if (POLICY CMP0072)
       cmake_policy(SET CMP0072 NEW)
endif()

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

find_package(GLEW REQUIRED)
target_link_libraries(${PROJECT_NAME} GLEW::GLEW)

find_package(GLFW3 REQUIRED)
include_directories(${GLFW3_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${GLFW3_LIBRARIES})

find_package(PythonLibs 2 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARIES})

if (NOT WIN32)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(PNG libpng16 REQUIRED)
endif()
include_directories(${PNG_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARIES})

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

find_package(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${OPENAL_LIBRARY})

find_package(libsndfile REQUIRED)
include_directories(${LIBSNDFILE_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LIBSNDFILE_LIBRARY})

find_package(LuaJIT REQUIRED)
include_directories(${LUAJIT_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LUAJIT_LIBRARIES})

if (WITH_UART)
	find_package(libserialport REQUIRED)
	include_directories(${libserialport_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${libserialport_LIBRARY})
endif()

if (WIN32)
	target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties(${PROJECT_NAME} PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
cotire(${PROJECT_NAME})
