# -----------------------------------------------------------------------------
# Automatic Localization JSON Scanning & Code Generation
# -----------------------------------------------------------------------------
file(GLOB LOCALIZATION_JSON_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/texts/*.json")

set(TEXT_RESOURCE_RC_CONTENT "")
set(TEXT_RESOURCE_ASM_CONTENT "")
set(TEXT_CPP_EXTERNS "")
set(TEXT_CPP_REGISTRATION "")

set(RES_ID 200)

foreach(JSON_FILE ${LOCALIZATION_JSON_FILES})
    get_filename_component(LANG_NAME "${JSON_FILE}" NAME_WE)
    math(EXPR RES_ID "${RES_ID} + 1")

    file(TO_CMAKE_PATH "${JSON_FILE}" JSON_FILE_NORM)

    string(APPEND TEXT_RESOURCE_RC_CONTENT "#define IDR_TEXT_${LANG_NAME} ${RES_ID}\n")
    string(APPEND TEXT_RESOURCE_RC_CONTENT "IDR_TEXT_${LANG_NAME} RCDATA \"${JSON_FILE_NORM}\"\n\n")
    
    string(APPEND TEXT_RESOURCE_ASM_CONTENT ".globl _binary_${LANG_NAME}_json_start\n.globl _binary_${LANG_NAME}_json_end\n.p2align 4\n_binary_${LANG_NAME}_json_start:\n    .incbin \"${JSON_FILE_NORM}\"\n_binary_${LANG_NAME}_json_end:\n\n")

    if(WIN32)
        string(APPEND TEXT_CPP_REGISTRATION "    {\n        const unsigned char* data = nullptr;\n        size_t size = 0;\n        if (GetWin32Resource(${RES_ID}, data, size)) {\n            outFiles.push_back({\"${LANG_NAME}\", data, size});\n        }\n    }\n")
    else()
        string(APPEND TEXT_CPP_EXTERNS "extern const unsigned char _binary_${LANG_NAME}_json_start[];\nextern const unsigned char _binary_${LANG_NAME}_json_end[];\n")
        string(APPEND TEXT_CPP_REGISTRATION "    outFiles.push_back({\"${LANG_NAME}\", _binary_${LANG_NAME}_json_start, static_cast<size_t>(_binary_${LANG_NAME}_json_end - _binary_${LANG_NAME}_json_start)});\n")
    endif()
endforeach()

set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GEN_DIR}")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/embedded_texts_impl.inc.in"
    "${GEN_DIR}/embedded_texts_impl.inc"
    @ONLY
)

if(WIN32)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/embedded_resources.rc.in"
        "${GEN_DIR}/embedded_resources.rc"
        @ONLY
    )
    set(EMBEDDED_RESOURCE_FILES "${GEN_DIR}/embedded_resources.rc")
else()
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/embedded_resources_unix.S.in"
        "${GEN_DIR}/embedded_resources_unix.S"
        @ONLY
    )
    set(EMBEDDED_RESOURCE_FILES "${GEN_DIR}/embedded_resources_unix.S")
endif()

set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/embedded_resources.cpp"
    ${EMBEDDED_RESOURCE_FILES}
    PROPERTIES OBJECT_DEPENDS "${LOCALIZATION_JSON_FILES}"
)
