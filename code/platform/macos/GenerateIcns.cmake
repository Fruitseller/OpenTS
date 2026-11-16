# Builds the bundle icon from the Windows icon, which carries a single 256 pixel image.
# The argument names are the ones code/CMakeLists.txt passes to this script.

if(NOT EXISTS "${OPENTS_ICON_SOURCE}")
    message(FATAL_ERROR "The application icon ${OPENTS_ICON_SOURCE} does not exist.")
endif()

file(REMOVE_RECURSE "${OPENTS_ICON_WORK_DIR}")
file(MAKE_DIRECTORY "${OPENTS_ICON_WORK_DIR}/OpenTS.iconset")

function(opents_iconset_image size name)
    execute_process(
        COMMAND "${SIPS_EXECUTABLE}" --resampleHeightWidth "${size}" "${size}"
            -s format png "${OPENTS_ICON_SOURCE}"
            --out "${OPENTS_ICON_WORK_DIR}/OpenTS.iconset/${name}"
        RESULT_VARIABLE result
        OUTPUT_QUIET
        ERROR_VARIABLE error
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "sips could not make the ${size} pixel icon image: ${error}")
    endif()
endfunction()

opents_iconset_image(16 icon_16x16.png)
opents_iconset_image(32 icon_16x16@2x.png)
opents_iconset_image(32 icon_32x32.png)
opents_iconset_image(64 icon_32x32@2x.png)
opents_iconset_image(128 icon_128x128.png)
opents_iconset_image(256 icon_128x128@2x.png)
opents_iconset_image(256 icon_256x256.png)
opents_iconset_image(512 icon_256x256@2x.png)
opents_iconset_image(512 icon_512x512.png)
opents_iconset_image(1024 icon_512x512@2x.png)

get_filename_component(output_directory "${OPENTS_ICON_OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${output_directory}")

execute_process(
    COMMAND "${ICONUTIL_EXECUTABLE}" -c icns
        "${OPENTS_ICON_WORK_DIR}/OpenTS.iconset"
        -o "${OPENTS_ICON_OUTPUT}"
    RESULT_VARIABLE result
    OUTPUT_QUIET
    ERROR_VARIABLE error
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "iconutil could not make ${OPENTS_ICON_OUTPUT}: ${error}")
endif()
