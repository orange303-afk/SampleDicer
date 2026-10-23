cmake_minimum_required(VERSION 3.22)
project(SampleDicer VERSION 1.6.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
  add_subdirectory(JUCE)
else()
  message(FATAL_ERROR "JUCE not found. Clone it into ./JUCE")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSVC)
  set(vst3_auto_manifest FALSE)
else()
  set(vst3_auto_manifest TRUE)
endif()

juce_add_plugin(SampleDicer COMPANY_NAME "Ilya Orange" BUNDLE_ID com.sampledicer.plugin
  PLUGIN_MANUFACTURER_CODE SmDc PLUGIN_CODE Sdcr FORMATS AU VST3 Standalone
  PRODUCT_NAME "Sample Dicer" IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE COPY_PLUGIN_AFTER_BUILD FALSE
  VST3_AUTO_MANIFEST ${vst3_auto_manifest}
  ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Assets/SampleDicerIcon.png")
include(FetchContent)
find_package(SQLite3 QUIET)
if(NOT SQLite3_FOUND)
    message(STATUS "SQLite3 not found on system; fetching amalgamation...")
    FetchContent_Declare(
        sqlite3_amalgamation
        URL https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip
        URL_HASH SHA256=77823cb110929c2bcb0f5d48e4833b5c59a8a6e40cdea3936b99e199dbbe5784
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(sqlite3_amalgamation)

    add_library(sqlite3_amalgamation_target STATIC
        "${sqlite3_amalgamation_SOURCE_DIR}/sqlite3.c")
    target_include_directories(sqlite3_amalgamation_target PUBLIC
        "${sqlite3_amalgamation_SOURCE_DIR}")
    add_library(SQLite3::SQLite3 ALIAS sqlite3_amalgamation_target)
endif()

juce_generate_juce_header(SampleDicer)
target_sources(SampleDicer PRIVATE
  Source/PluginProcessor.cpp Source/PluginProcessor.h
  Source/PluginEditor.cpp Source/PluginEditor.h
  Source/Database/SampleDatabase.cpp Source/Database/SampleDatabase.h
  Source/Database/SampleScanner.cpp Source/Database/SampleScanner.h
  Source/UI/SampleBrowserModalComponent.cpp Source/UI/SampleBrowserModalComponent.h)
target_compile_definitions(SampleDicer PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(SampleDicer PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_gui_extra SQLite3::SQLite3 PUBLIC
  juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

if(APPLE)
  foreach(bundle_target SampleDicer_AU SampleDicer_VST3 SampleDicer_Standalone)
    add_custom_command(TARGET ${bundle_target} PRE_LINK
      COMMAND /bin/sh -c "/usr/bin/xattr -dr com.apple.FinderInfo \"$<TARGET_BUNDLE_DIR:${bundle_target}>\" 2>/dev/null || true"
      COMMAND /bin/sh -c "/usr/bin/xattr -cr \"$<TARGET_BUNDLE_DIR:${bundle_target}>\" 2>/dev/null || true"
      VERBATIM)
    add_custom_command(TARGET ${bundle_target} POST_BUILD
      COMMAND /bin/sh -c "/usr/bin/xattr -dr com.apple.FinderInfo \"$<TARGET_BUNDLE_DIR:${bundle_target}>\" 2>/dev/null || true"
      COMMAND /bin/sh -c "/usr/bin/xattr -cr \"$<TARGET_BUNDLE_DIR:${bundle_target}>\" 2>/dev/null || true"
      COMMAND /usr/bin/codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:${bundle_target}>"
      VERBATIM)
  endforeach()

  set(vst3_install_dir "/Library/Audio/Plug-Ins/VST3")
  set(stale_user_vst3 "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/Sample Dicer.vst3")
  add_custom_command(TARGET SampleDicer_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${stale_user_vst3}"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${vst3_install_dir}/Sample Dicer.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_BUNDLE_DIR:SampleDicer_VST3>"
            "${vst3_install_dir}/Sample Dicer.vst3"
    COMMAND /usr/bin/xattr -dr com.apple.quarantine "${vst3_install_dir}/Sample Dicer.vst3"
    COMMAND /usr/bin/xattr -cr "${vst3_install_dir}/Sample Dicer.vst3"
    COMMENT "Installing VST3 to ${vst3_install_dir}"
    VERBATIM)
elseif(WIN32 AND MSVC AND NOT DEFINED ENV{CI})
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(vst3_install_dir "$ENV{CommonProgramW6432}/VST3")
  else()
    set(vst3_install_dir "$ENV{CommonProgramFiles(x86)}/VST3")
  endif()
  add_custom_command(TARGET SampleDicer_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${vst3_install_dir}"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${vst3_install_dir}/Sample Dicer.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_BUNDLE_DIR:SampleDicer_VST3>"
            "${vst3_install_dir}/Sample Dicer.vst3"
    COMMENT "Installing VST3 to ${vst3_install_dir}"
    VERBATIM)
endif()
