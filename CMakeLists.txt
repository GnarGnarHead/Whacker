cmake_minimum_required(VERSION 3.20)

project(whacker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

option(WHACKER_BUILD_APP "Build the SDL2 app executable." ON)
option(WHACKER_BUILD_TESTS "Build smoke tests." ON)
option(WHACKER_STRICT_WARNINGS "Enable stricter warning flags for local audits." OFF)
option(WHACKER_STORY_STRICT_CONTENT "Enable strict story authoring integrity checks (for CI/release gates)." OFF)
option(WHACKER_R36_GLES2_TEXTURES "Use the GLES2 triangle backend for RGBA texture rendering." OFF)
set(WHACKER_INPUT_PROFILE "desktop" CACHE STRING "Default SDL app input profile: desktop or handheld.")
set_property(CACHE WHACKER_INPUT_PROFILE PROPERTY STRINGS desktop handheld)
set(WHACKER_HOST_STORY_PACK_COMPILER "" CACHE FILEPATH "Path to a native story_pack_compiler executable for cross builds.")

if(NOT WHACKER_INPUT_PROFILE MATCHES "^(desktop|handheld)$")
    message(FATAL_ERROR "WHACKER_INPUT_PROFILE must be 'desktop' or 'handheld'.")
endif()

# Enforce clean-slate AI ownership: no legacy ai/ stack is allowed.
file(GLOB WHACKER_LEGACY_AI_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/ai/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ai/*.hpp"
)
if(WHACKER_LEGACY_AI_FILES)
    message(FATAL_ERROR "Legacy AI stack files detected under ai/ or include/ai/. This branch keeps AI implementation out of that path.")
endif()

function(whacker_apply_warnings target_name)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
        if(WHACKER_STRICT_WARNINGS)
            target_compile_options(
                ${target_name}
                PRIVATE
                    -Wshadow
                    -Wconversion
                    -Wsign-conversion
                    -Wdouble-promotion
            )
        endif()
    endif()
endfunction()

function(whacker_configure_smoke_test target_name)
    whacker_apply_warnings(${target_name})
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target_name} PRIVATE -UNDEBUG)
    endif()
    target_compile_definitions(${target_name} PRIVATE WHACKER_TEST_ASSERTIONS_ACTIVE=1)
endfunction()

function(whacker_add_story_pack_compiler_test test_name)
    add_test(
        NAME ${test_name}
        COMMAND
            ${WHACKER_STORY_PACK_COMPILER_COMMAND}
            --validate-only
            ${ARGN}
    )
endfunction()

set(WHACKER_STORY_WEEK01_TEXT_PACK "${CMAKE_CURRENT_SOURCE_DIR}/story/content/season1/week01.toml")
set(WHACKER_STORY_SEASON1_GRAPH_PACK "${CMAKE_CURRENT_SOURCE_DIR}/story/content/season1/season1_graph.toml")
set(WHACKER_UI_MENU_STICKER_PACK "${CMAKE_CURRENT_SOURCE_DIR}/story/content/ui/menu_stickers.toml")

if(CMAKE_CROSSCOMPILING AND NOT WHACKER_HOST_STORY_PACK_COMPILER)
    message(FATAL_ERROR "Cross builds must set WHACKER_HOST_STORY_PACK_COMPILER to a native story_pack_compiler executable.")
endif()

set(WHACKER_STORY_PACK_COMPILER_COMMAND "")
set(WHACKER_STORY_PACK_COMPILER_DEPENDS "")
if(WHACKER_HOST_STORY_PACK_COMPILER)
    get_filename_component(
        WHACKER_HOST_STORY_PACK_COMPILER_ABS
        "${WHACKER_HOST_STORY_PACK_COMPILER}"
        ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    if(NOT EXISTS "${WHACKER_HOST_STORY_PACK_COMPILER_ABS}")
        message(FATAL_ERROR "WHACKER_HOST_STORY_PACK_COMPILER does not exist: ${WHACKER_HOST_STORY_PACK_COMPILER_ABS}")
    endif()
    set(WHACKER_STORY_PACK_COMPILER_COMMAND "${WHACKER_HOST_STORY_PACK_COMPILER_ABS}")
    set(WHACKER_STORY_PACK_COMPILER_DEPENDS "${WHACKER_HOST_STORY_PACK_COMPILER_ABS}")
    message(STATUS "Using host story pack compiler: ${WHACKER_HOST_STORY_PACK_COMPILER_ABS}")
else()
    add_executable(story_pack_compiler
        tools/story_pack_compiler.cpp
    )
    target_include_directories(
        story_pack_compiler
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/app
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_compile_features(story_pack_compiler PRIVATE cxx_std_20)
    whacker_apply_warnings(story_pack_compiler)
    set(WHACKER_STORY_PACK_COMPILER_COMMAND "$<TARGET_FILE:story_pack_compiler>")
    set(WHACKER_STORY_PACK_COMPILER_DEPENDS story_pack_compiler)
endif()

set(WHACKER_STORY_PACK_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/story")
set(WHACKER_STORY_PACK_GEN_CPP "${WHACKER_STORY_PACK_GEN_DIR}/story_pack_gen.cpp")
add_custom_command(
    OUTPUT
        ${WHACKER_STORY_PACK_GEN_CPP}
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${WHACKER_STORY_PACK_GEN_DIR}
    COMMAND
        ${WHACKER_STORY_PACK_COMPILER_COMMAND}
            --input ${WHACKER_STORY_WEEK01_TEXT_PACK}
            --input ${WHACKER_STORY_SEASON1_GRAPH_PACK}
            --input ${WHACKER_UI_MENU_STICKER_PACK}
            --out-cpp ${WHACKER_STORY_PACK_GEN_CPP}
    DEPENDS
        ${WHACKER_STORY_PACK_COMPILER_DEPENDS}
        ${WHACKER_STORY_WEEK01_TEXT_PACK}
        ${WHACKER_STORY_SEASON1_GRAPH_PACK}
        ${WHACKER_UI_MENU_STICKER_PACK}
    COMMENT
        "Compiling story content packs"
    VERBATIM
)

add_library(story_pack_gen STATIC
    ${WHACKER_STORY_PACK_GEN_CPP}
)
target_include_directories(
    story_pack_gen
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(story_pack_gen PRIVATE cxx_std_20)
whacker_apply_warnings(story_pack_gen)

add_library(whacker_core
    sim/config_io.cpp
    sim/collision.cpp
    sim/physics.cpp
    sim/spin.cpp
    progression/skills.cpp
    progression/style.cpp
    progression/tags.cpp
    progression/reputation.cpp
    input/gamepad.cpp
    input/keyboard.cpp
    input/mouse.cpp
    input/serial.cpp
)

target_include_directories(whacker_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(whacker_core PUBLIC cxx_std_20)
whacker_apply_warnings(whacker_core)

set(WHACKER_AI_PLANNER_SOURCES
    app/ai_core.cpp
    app/ai_frame.cpp
    app/ai_seed.cpp
    app/ai_profile.cpp
    app/ai_predict.cpp
    app/ai_reachability.cpp
    app/ai_candidates.cpp
    app/ai_score.cpp
    app/ai_execute.cpp
    app/ai_decision_templates.cpp
)

set(WHACKER_AI_PLAY_CONTROL_RUNTIME_SOURCES
    ${WHACKER_AI_PLANNER_SOURCES}
    app/ai_plan_state_bridge.cpp
    app/play_control_human.cpp
    app/play_control.cpp
    app/paddle_tuning.cpp
)

if(WHACKER_BUILD_APP)
    add_executable(whacker
        app/action_input.cpp
        app/app_runtime.cpp
        app/audio_engine.cpp
        app/ai_core.cpp
        app/ai_frame.cpp
        app/ai_seed.cpp
        app/ai_profile.cpp
        app/ai_predict.cpp
        app/ai_reachability.cpp
        app/ai_candidates.cpp
        app/ai_score.cpp
        app/ai_execute.cpp
        app/ai_decision_templates.cpp
        app/ai_plan_state_bridge.cpp
        app/game_render.cpp
        app/input_binding_codec.cpp
        app/input_profiles.cpp
        app/main.cpp
        app/main_menu_actions.cpp
        app/match_end_flow.cpp
        app/match_exit_policy.cpp
        app/match_flow.cpp
        app/menu_overlay.cpp
        app/menu_settings.cpp
        app/menu_sticker_render.cpp
        app/navigation.cpp
        app/options_menu_actions.cpp
        app/overlay_layout_math.cpp
        app/paddle_tuning.cpp
        app/paddle_tuning_actions.cpp
        app/paddle_tuning_overlay.cpp
        app/pause_menu_actions.cpp
        app/player_control_presets.cpp
        app/play_control.cpp
        app/play_control_human.cpp
        app/platform_sdl.cpp
        app/pixel_font.cpp
        app/png_rgba_loader.cpp
        app/quick_menu_actions.cpp
        app/quick_menu_render.cpp
        app/rgba_texture.cpp
        app/runtime_asset_path.cpp
        app/runtime_story_scene.cpp
        app/runtime_transitions.cpp
        app/runtime_visual_transition.cpp
        app/runtime_visual_transition_render.cpp
        app/sdl_options_binding_access.cpp
        app/sdl_options_capture.cpp
        app/sdl_options_controller.cpp
        app/sdl_options_value_labels.cpp
        app/sdl_input.cpp
        app/sdl_runtime_audio.cpp
        app/sdl_runtime_labels.cpp
        app/sdl_runtime_render.cpp
        app/sdl_runtime_state.cpp
        app/sdl_runtime_transitions.cpp
        app/sdl_runtime_update.cpp
        app/story_chat_layout.cpp
        app/story_chat_portrait_overlay.cpp
        app/story_classification.cpp
        app/story_continue_resume.cpp
        app/story_hub_controller.cpp
        app/story_intro.cpp
        app/story_intro_overlay.cpp
        app/story_intro_text_layout.cpp
        app/story_match.cpp
        app/story_menu_actions.cpp
        app/story_menu_controller.cpp
        app/story_onboarding_sync.cpp
        app/story_overlays.cpp
        app/story_panel_layout.cpp
        app/story_play_session.cpp
        app/story_portrait_render.cpp
        app/story_portraits.cpp
        app/story_runtime.cpp
        app/story_runtime_invariants.cpp
        app/story_save.cpp
        app/story_script_catalog.cpp
        app/story_scene.cpp
        app/story_scene_overlay.cpp
        app/story_scene_text_layout.cpp
        app/story_text.cpp
        app/story_text_feedback.cpp
        app/story_text_week.cpp
        app/story_text_week_01.cpp
        app/text_utils.cpp
        app/text_wrap.cpp
        app/ui_text.cpp
    )
    target_link_libraries(whacker PRIVATE whacker_core)
    target_link_libraries(whacker PRIVATE story_pack_gen)
    target_compile_definitions(whacker PRIVATE WHACKER_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    whacker_apply_warnings(whacker)

    set(WHACKER_SDL2_TARGET "")
    find_package(SDL2 QUIET)
    if(TARGET SDL2::SDL2)
        set(WHACKER_SDL2_TARGET SDL2::SDL2)
    elseif(TARGET SDL2::SDL2-static)
        set(WHACKER_SDL2_TARGET SDL2::SDL2-static)
    else()
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(SDL2_PC QUIET sdl2)
            if(SDL2_PC_FOUND)
                target_include_directories(whacker PRIVATE ${SDL2_PC_INCLUDE_DIRS})
                target_link_libraries(whacker PRIVATE ${SDL2_PC_LINK_LIBRARIES})
                target_compile_options(whacker PRIVATE ${SDL2_PC_CFLAGS_OTHER})
            endif()
        endif()
    endif()

    if(WHACKER_SDL2_TARGET)
        target_link_libraries(whacker PRIVATE ${WHACKER_SDL2_TARGET})
    elseif(NOT SDL2_PC_FOUND)
        message(FATAL_ERROR "SDL2 is required when WHACKER_BUILD_APP=ON. Install SDL2 or configure with -DWHACKER_BUILD_APP=OFF for a headless/test-only build.")
    endif()

    target_compile_definitions(
        whacker
        PRIVATE
            WHACKER_PLATFORM_SDL2=1
            WHACKER_HAS_SDL2_AUDIO=1
    )
    if(WHACKER_INPUT_PROFILE STREQUAL "handheld")
        target_compile_definitions(whacker PRIVATE WHACKER_INPUT_PROFILE_HANDHELD=1)
    endif()
    if(WHACKER_R36_GLES2_TEXTURES)
        target_compile_definitions(whacker PRIVATE WHACKER_R36_GLES2_TEXTURES=1)
        target_link_libraries(whacker PRIVATE ${CMAKE_DL_LIBS})
    endif()

    find_package(OpenGL QUIET)
    if(TARGET OpenGL::GL)
        target_link_libraries(whacker PRIVATE OpenGL::GL)
    else()
        message(WARNING "OpenGL library not found. Rendering symbols may be unavailable once the SDL2 render path is enabled.")
    endif()

    find_package(PNG QUIET)
    if(TARGET PNG::PNG)
        target_link_libraries(whacker PRIVATE PNG::PNG)
        target_compile_definitions(whacker PRIVATE WHACKER_HAS_PNG=1)
    else()
        message(WARNING "libpng not found. Story portraits will render deterministic built-in placeholders once the SDL2 render path is enabled.")
    endif()

    add_executable(play_control_duel
        tools/play_control_duel.cpp
        ${WHACKER_AI_PLAY_CONTROL_RUNTIME_SOURCES}
        app/match_flow.cpp
    )
    target_link_libraries(play_control_duel PRIVATE whacker_core)
    target_include_directories(play_control_duel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
    target_compile_definitions(
        play_control_duel
        PRIVATE
            WHACKER_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
    whacker_apply_warnings(play_control_duel)
endif()

if(WHACKER_BUILD_TESTS)
    include(CTest)

    if(BUILD_TESTING)
        add_executable(assert_config_smoke
            tests/assert_config_smoke.cpp
        )
        target_link_libraries(assert_config_smoke PRIVATE whacker_core)
        target_include_directories(assert_config_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(assert_config_smoke)
        add_test(NAME assert_config_smoke COMMAND assert_config_smoke)

        add_executable(story_flow_smoke
            tests/story_flow_smoke.cpp
            app/story_onboarding_flow.cpp
            app/story_classification.cpp
            app/story_runtime_invariants.cpp
            app/story_onboarding_sync.cpp
            app/story_script_catalog.cpp
            app/story_text.cpp
            app/story_text_week.cpp
            app/story_text_week_01.cpp
        )
        target_link_libraries(story_flow_smoke PRIVATE whacker_core story_pack_gen)
        target_include_directories(story_flow_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_flow_smoke)
        add_test(NAME story_flow_smoke COMMAND story_flow_smoke)

        whacker_add_story_pack_compiler_test(
            story_pack_compiler_smoke
            --input ${WHACKER_STORY_WEEK01_TEXT_PACK}
            --input ${WHACKER_STORY_SEASON1_GRAPH_PACK}
            --input ${WHACKER_UI_MENU_STICKER_PACK}
        )

        whacker_add_story_pack_compiler_test(
            story_pack_compiler_invalid_missing_keys_smoke
            --input ${CMAKE_CURRENT_SOURCE_DIR}/tests/story_packs/invalid_week01_missing_keys.toml
            --input ${WHACKER_STORY_SEASON1_GRAPH_PACK}
            --input ${WHACKER_UI_MENU_STICKER_PACK}
        )
        set_tests_properties(story_pack_compiler_invalid_missing_keys_smoke PROPERTIES WILL_FAIL TRUE)

        whacker_add_story_pack_compiler_test(
            story_pack_compiler_invalid_dangling_graph_edge_smoke
            --input ${WHACKER_STORY_WEEK01_TEXT_PACK}
            --input ${CMAKE_CURRENT_SOURCE_DIR}/tests/story_packs/invalid_season1_graph_dangling_next_node.toml
            --input ${WHACKER_UI_MENU_STICKER_PACK}
        )
        set_tests_properties(story_pack_compiler_invalid_dangling_graph_edge_smoke PROPERTIES WILL_FAIL TRUE)

        whacker_add_story_pack_compiler_test(
            story_pack_compiler_invalid_menu_sticker_anchor_smoke
            --input ${WHACKER_STORY_WEEK01_TEXT_PACK}
            --input ${WHACKER_STORY_SEASON1_GRAPH_PACK}
            --input ${CMAKE_CURRENT_SOURCE_DIR}/tests/story_packs/invalid_menu_stickers_bad_anchor.toml
        )
        set_tests_properties(story_pack_compiler_invalid_menu_sticker_anchor_smoke PROPERTIES WILL_FAIL TRUE)

        whacker_add_story_pack_compiler_test(
            story_pack_compiler_invalid_menu_sticker_inside_missing_xy_smoke
            --input ${WHACKER_STORY_WEEK01_TEXT_PACK}
            --input ${WHACKER_STORY_SEASON1_GRAPH_PACK}
            --input ${CMAKE_CURRENT_SOURCE_DIR}/tests/story_packs/invalid_menu_stickers_inside_missing_xy.toml
        )
        set_tests_properties(story_pack_compiler_invalid_menu_sticker_inside_missing_xy_smoke PROPERTIES WILL_FAIL TRUE)

        whacker_add_story_pack_compiler_test(
            story_pack_compiler_invalid_menu_sticker_height_missing_height_smoke
            --input ${WHACKER_STORY_WEEK01_TEXT_PACK}
            --input ${WHACKER_STORY_SEASON1_GRAPH_PACK}
            --input ${CMAKE_CURRENT_SOURCE_DIR}/tests/story_packs/invalid_menu_stickers_height_missing_height.toml
        )
        set_tests_properties(story_pack_compiler_invalid_menu_sticker_height_missing_height_smoke PROPERTIES WILL_FAIL TRUE)

        whacker_add_story_pack_compiler_test(
            story_pack_compiler_invalid_menu_sticker_bad_norm_smoke
            --input ${WHACKER_STORY_WEEK01_TEXT_PACK}
            --input ${WHACKER_STORY_SEASON1_GRAPH_PACK}
            --input ${CMAKE_CURRENT_SOURCE_DIR}/tests/story_packs/invalid_menu_stickers_bad_norm.toml
        )
        set_tests_properties(story_pack_compiler_invalid_menu_sticker_bad_norm_smoke PROPERTIES WILL_FAIL TRUE)

        whacker_add_story_pack_compiler_test(
            story_pack_compiler_invalid_menu_sticker_bad_height_px_smoke
            --input ${WHACKER_STORY_WEEK01_TEXT_PACK}
            --input ${WHACKER_STORY_SEASON1_GRAPH_PACK}
            --input ${CMAKE_CURRENT_SOURCE_DIR}/tests/story_packs/invalid_menu_stickers_bad_height_px.toml
        )
        set_tests_properties(story_pack_compiler_invalid_menu_sticker_bad_height_px_smoke PROPERTIES WILL_FAIL TRUE)

        add_executable(story_continue_resume_smoke
            tests/story_continue_resume_smoke.cpp
            app/story_continue_resume.cpp
            app/story_runtime_invariants.cpp
            app/story_onboarding_sync.cpp
            app/story_script_catalog.cpp
            app/story_text.cpp
            app/story_text_week.cpp
            app/story_text_week_01.cpp
        )
        target_link_libraries(story_continue_resume_smoke PRIVATE whacker_core story_pack_gen)
        target_include_directories(story_continue_resume_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_continue_resume_smoke)
        add_test(NAME story_continue_resume_smoke COMMAND story_continue_resume_smoke)

        add_executable(story_scene_smoke
            tests/story_scene_smoke.cpp
            app/story_script_catalog.cpp
            app/story_scene.cpp
            app/story_text.cpp
            app/story_text_feedback.cpp
            app/story_text_week.cpp
            app/story_text_week_01.cpp
        )
        target_link_libraries(story_scene_smoke PRIVATE whacker_core)
        target_link_libraries(story_scene_smoke PRIVATE story_pack_gen)
        target_include_directories(story_scene_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_scene_smoke)
        add_test(NAME story_scene_smoke COMMAND story_scene_smoke)

        add_executable(story_panel_layout_smoke
            tests/story_panel_layout_smoke.cpp
            app/story_panel_layout.cpp
        )
        target_link_libraries(story_panel_layout_smoke PRIVATE whacker_core)
        target_include_directories(story_panel_layout_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_panel_layout_smoke)
        add_test(NAME story_panel_layout_smoke COMMAND story_panel_layout_smoke)

        add_executable(story_scene_text_layout_smoke
            tests/story_scene_text_layout_smoke.cpp
            app/story_scene_text_layout.cpp
            app/overlay_layout_math.cpp
            app/story_chat_layout.cpp
            app/text_wrap.cpp
        )
        target_link_libraries(story_scene_text_layout_smoke PRIVATE whacker_core)
        target_include_directories(story_scene_text_layout_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_scene_text_layout_smoke)
        add_test(NAME story_scene_text_layout_smoke COMMAND story_scene_text_layout_smoke)

        add_executable(story_intro_text_layout_smoke
            tests/story_intro_text_layout_smoke.cpp
            app/story_intro.cpp
            app/story_intro_text_layout.cpp
            app/story_chat_layout.cpp
            app/text_wrap.cpp
            app/story_text.cpp
            app/story_text_feedback.cpp
            app/story_text_week.cpp
            app/story_text_week_01.cpp
        )
        target_link_libraries(story_intro_text_layout_smoke PRIVATE whacker_core)
        target_link_libraries(story_intro_text_layout_smoke PRIVATE story_pack_gen)
        target_include_directories(story_intro_text_layout_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_intro_text_layout_smoke)
        add_test(NAME story_intro_text_layout_smoke COMMAND story_intro_text_layout_smoke)

        add_executable(overlay_layout_math_smoke
            tests/overlay_layout_math_smoke.cpp
            app/overlay_layout_math.cpp
        )
        target_link_libraries(overlay_layout_math_smoke PRIVATE whacker_core)
        target_include_directories(overlay_layout_math_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(overlay_layout_math_smoke)
        add_test(NAME overlay_layout_math_smoke COMMAND overlay_layout_math_smoke)

        add_executable(text_wrap_variant_smoke
            tests/text_wrap_variant_smoke.cpp
            app/text_wrap.cpp
            app/overlay_layout_math.cpp
        )
        target_link_libraries(text_wrap_variant_smoke PRIVATE whacker_core)
        target_include_directories(text_wrap_variant_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(text_wrap_variant_smoke)
        add_test(NAME text_wrap_variant_smoke COMMAND text_wrap_variant_smoke)

        add_executable(story_chat_layout_smoke
            tests/story_chat_layout_smoke.cpp
            app/story_panel_layout.cpp
            app/overlay_layout_math.cpp
            app/story_chat_layout.cpp
        )
        target_link_libraries(story_chat_layout_smoke PRIVATE whacker_core)
        target_include_directories(story_chat_layout_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_chat_layout_smoke)
        add_test(NAME story_chat_layout_smoke COMMAND story_chat_layout_smoke)

        add_executable(story_portraits_smoke
            tests/story_portraits_smoke.cpp
            app/story_portraits.cpp
        )
        target_link_libraries(story_portraits_smoke PRIVATE whacker_core)
        target_include_directories(story_portraits_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_portraits_smoke)
        add_test(NAME story_portraits_smoke COMMAND story_portraits_smoke)

        find_program(WHACKER_JQ_EXE jq)
        if(WHACKER_JQ_EXE)
            add_test(
                NAME portrait_profile_contract_smoke
                COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/story/characters/art/normalize_portraits.sh --validate-only
            )
        endif()

        add_executable(story_rivals_smoke
            tests/story_rivals_smoke.cpp
            app/story_script_catalog.cpp
            app/story_text.cpp
            app/story_text_feedback.cpp
            app/story_text_week.cpp
            app/story_text_week_01.cpp
        )
        target_link_libraries(story_rivals_smoke PRIVATE whacker_core story_pack_gen)
        target_include_directories(story_rivals_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_rivals_smoke)
        add_test(NAME story_rivals_smoke COMMAND story_rivals_smoke)

        if(WHACKER_STORY_STRICT_CONTENT)
            add_executable(story_graph_authoring_guard_smoke
                tests/story_graph_authoring_guard_smoke.cpp
                app/story_script_catalog.cpp
                app/story_text.cpp
                app/story_text_feedback.cpp
                app/story_text_week.cpp
                app/story_text_week_01.cpp
            )
            target_link_libraries(story_graph_authoring_guard_smoke PRIVATE whacker_core story_pack_gen)
            target_include_directories(story_graph_authoring_guard_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
            whacker_configure_smoke_test(story_graph_authoring_guard_smoke)
            add_test(NAME story_graph_authoring_guard_smoke COMMAND story_graph_authoring_guard_smoke)
        endif()

        add_executable(story_text_week_smoke
            tests/story_text_week_smoke.cpp
            app/story_text.cpp
            app/story_text_feedback.cpp
            app/story_text_week.cpp
            app/story_text_week_01.cpp
        )
        target_link_libraries(story_text_week_smoke PRIVATE whacker_core)
        target_link_libraries(story_text_week_smoke PRIVATE story_pack_gen)
        target_include_directories(story_text_week_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_text_week_smoke)
        add_test(NAME story_text_week_smoke COMMAND story_text_week_smoke)

        add_executable(story_match_policy_smoke
            tests/story_match_policy_smoke.cpp
            app/story_script_catalog.cpp
            app/story_text.cpp
            app/story_text_feedback.cpp
            app/story_text_week.cpp
            app/story_text_week_01.cpp
        )
        target_link_libraries(story_match_policy_smoke PRIVATE whacker_core)
        target_link_libraries(story_match_policy_smoke PRIVATE story_pack_gen)
        target_include_directories(story_match_policy_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_match_policy_smoke)
        add_test(NAME story_match_policy_smoke COMMAND story_match_policy_smoke)

        add_executable(story_progression_policy_smoke
            tests/story_progression_policy_smoke.cpp
            app/match_flow.cpp
            app/runtime_visual_transition.cpp
            app/story_scene.cpp
            app/story_classification.cpp
            app/story_intro.cpp
            app/story_match.cpp
            app/story_onboarding_sync.cpp
            app/story_play_session.cpp
            app/story_runtime.cpp
            app/story_runtime_invariants.cpp
            app/story_script_catalog.cpp
            app/story_text.cpp
            app/story_text_feedback.cpp
            app/story_text_week.cpp
            app/story_text_week_01.cpp
        )
        target_link_libraries(story_progression_policy_smoke PRIVATE whacker_core)
        target_link_libraries(story_progression_policy_smoke PRIVATE story_pack_gen)
        target_include_directories(story_progression_policy_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_progression_policy_smoke)
        add_test(NAME story_progression_policy_smoke COMMAND story_progression_policy_smoke)

        add_executable(story_save_smoke
            tests/story_save_smoke.cpp
            app/story_save.cpp
            app/story_script_catalog.cpp
            app/story_text.cpp
            app/story_text_feedback.cpp
            app/story_text_week.cpp
            app/story_text_week_01.cpp
            app/text_utils.cpp
        )
        target_link_libraries(story_save_smoke PRIVATE whacker_core story_pack_gen)
        target_include_directories(story_save_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_save_smoke)
        add_test(NAME story_save_smoke COMMAND story_save_smoke)

        add_executable(paddle_tuning_smoke
            tests/paddle_tuning_smoke.cpp
            app/paddle_tuning.cpp
        )
        target_link_libraries(paddle_tuning_smoke PRIVATE whacker_core)
        target_include_directories(paddle_tuning_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(paddle_tuning_smoke)
        add_test(NAME paddle_tuning_smoke COMMAND paddle_tuning_smoke)

        add_executable(skills_growth_smoke
            tests/skills_growth_smoke.cpp
        )
        target_link_libraries(skills_growth_smoke PRIVATE whacker_core)
        target_include_directories(skills_growth_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(skills_growth_smoke)
        add_test(NAME skills_growth_smoke COMMAND skills_growth_smoke)

        add_executable(story_xp_pacing_smoke
            tests/story_xp_pacing_smoke.cpp
        )
        target_link_libraries(story_xp_pacing_smoke PRIVATE whacker_core)
        target_include_directories(story_xp_pacing_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_xp_pacing_smoke)
        add_test(NAME story_xp_pacing_smoke COMMAND story_xp_pacing_smoke)

        add_executable(story_integration_smoke
            tests/story_integration_smoke.cpp
            app/action_input.cpp
            app/main_menu_actions.cpp
            app/match_end_flow.cpp
            app/match_flow.cpp
            app/runtime_story_scene.cpp
            app/runtime_visual_transition.cpp
            app/story_classification.cpp
            app/story_continue_resume.cpp
            app/story_intro.cpp
            app/story_intro_text_layout.cpp
            app/story_chat_layout.cpp
            app/story_match.cpp
            app/story_menu_actions.cpp
            app/story_menu_controller.cpp
            app/story_hub_controller.cpp
            app/paddle_tuning.cpp
            app/paddle_tuning_actions.cpp
            app/story_play_session.cpp
            app/story_onboarding_flow.cpp
            app/story_onboarding_sync.cpp
            app/story_runtime_invariants.cpp
            app/story_runtime.cpp
            app/story_script_catalog.cpp
            app/story_scene.cpp
            app/text_wrap.cpp
            app/story_text.cpp
            app/story_text_feedback.cpp
            app/story_text_week.cpp
            app/story_text_week_01.cpp
            app/text_utils.cpp
        )
        target_link_libraries(story_integration_smoke PRIVATE whacker_core)
        target_link_libraries(story_integration_smoke PRIVATE story_pack_gen)
        target_include_directories(story_integration_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_integration_smoke)
        add_test(NAME story_integration_smoke COMMAND story_integration_smoke)

        add_executable(match_flow_smoke
            tests/match_flow_smoke.cpp
            app/match_flow.cpp
            app/match_exit_policy.cpp
            app/story_script_catalog.cpp
            app/story_text.cpp
            app/story_text_feedback.cpp
            app/story_text_week.cpp
            app/story_text_week_01.cpp
        )
        target_link_libraries(match_flow_smoke PRIVATE whacker_core story_pack_gen)
        target_include_directories(match_flow_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(match_flow_smoke)
        add_test(NAME match_flow_smoke COMMAND match_flow_smoke)

        add_executable(match_end_flow_smoke
            tests/match_end_flow_smoke.cpp
            app/match_end_flow.cpp
            app/match_flow.cpp
            app/runtime_visual_transition.cpp
            app/story_runtime_invariants.cpp
            app/story_scene.cpp
            app/story_onboarding_sync.cpp
            app/story_script_catalog.cpp
            app/story_text.cpp
            app/story_text_week.cpp
            app/story_text_week_01.cpp
        )
        target_link_libraries(match_end_flow_smoke PRIVATE whacker_core)
        target_link_libraries(match_end_flow_smoke PRIVATE story_pack_gen)
        target_include_directories(match_end_flow_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(match_end_flow_smoke)
        add_test(NAME match_end_flow_smoke COMMAND match_end_flow_smoke)

        add_executable(runtime_transitions_smoke
            tests/runtime_transitions_smoke.cpp
            app/runtime_transitions.cpp
            app/runtime_visual_transition.cpp
            app/match_exit_policy.cpp
            app/match_flow.cpp
            app/story_runtime_invariants.cpp
            app/story_script_catalog.cpp
            app/story_text.cpp
            app/story_text_feedback.cpp
            app/story_text_week.cpp
            app/story_text_week_01.cpp
            app/text_utils.cpp
        )
        target_link_libraries(runtime_transitions_smoke PRIVATE whacker_core)
        target_link_libraries(runtime_transitions_smoke PRIVATE story_pack_gen)
        target_include_directories(runtime_transitions_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(runtime_transitions_smoke)
        add_test(NAME runtime_transitions_smoke COMMAND runtime_transitions_smoke)

        add_executable(runtime_visual_transition_smoke
            tests/runtime_visual_transition_smoke.cpp
            app/runtime_visual_transition.cpp
        )
        target_link_libraries(runtime_visual_transition_smoke PRIVATE whacker_core)
        target_include_directories(runtime_visual_transition_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(runtime_visual_transition_smoke)
        add_test(NAME runtime_visual_transition_smoke COMMAND runtime_visual_transition_smoke)

        add_executable(runtime_loop_policy_smoke
            tests/runtime_loop_policy_smoke.cpp
            app/runtime_loop_policy.cpp
        )
        target_link_libraries(runtime_loop_policy_smoke PRIVATE whacker_core)
        target_include_directories(runtime_loop_policy_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(runtime_loop_policy_smoke)
        add_test(NAME runtime_loop_policy_smoke COMMAND runtime_loop_policy_smoke)

        add_executable(action_input_smoke
            tests/action_input_smoke.cpp
            app/action_input.cpp
            app/main_menu_actions.cpp
            app/options_menu_actions.cpp
            app/pause_menu_actions.cpp
            app/quick_menu_actions.cpp
            app/story_menu_actions.cpp
        )
        target_link_libraries(action_input_smoke PRIVATE whacker_core)
        target_include_directories(action_input_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(action_input_smoke)
        add_test(NAME action_input_smoke COMMAND action_input_smoke)

        add_executable(control_plan_smoke
            tests/control_plan_smoke.cpp
        )
        target_link_libraries(control_plan_smoke PRIVATE whacker_core)
        target_include_directories(control_plan_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(control_plan_smoke)
        add_test(NAME control_plan_smoke COMMAND control_plan_smoke)

        add_executable(render_context_smoke
            tests/render_context_smoke.cpp
        )
        target_link_libraries(render_context_smoke PRIVATE whacker_core)
        target_include_directories(render_context_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(render_context_smoke)
        add_test(NAME render_context_smoke COMMAND render_context_smoke)

        add_executable(navigation_smoke
            tests/navigation_smoke.cpp
            app/navigation.cpp
        )
        target_link_libraries(navigation_smoke PRIVATE whacker_core)
        target_include_directories(navigation_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(navigation_smoke)
        add_test(NAME navigation_smoke COMMAND navigation_smoke)

        add_executable(rgba_texture_smoke
            tests/rgba_texture_smoke.cpp
        )
        target_link_libraries(rgba_texture_smoke PRIVATE whacker_core)
        target_include_directories(rgba_texture_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(rgba_texture_smoke)
        add_test(NAME rgba_texture_smoke COMMAND rgba_texture_smoke)

        add_executable(runtime_asset_path_smoke
            tests/runtime_asset_path_smoke.cpp
            app/runtime_asset_path.cpp
        )
        target_link_libraries(runtime_asset_path_smoke PRIVATE whacker_core)
        target_include_directories(runtime_asset_path_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        target_compile_definitions(runtime_asset_path_smoke PRIVATE WHACKER_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
        whacker_configure_smoke_test(runtime_asset_path_smoke)
        add_test(NAME runtime_asset_path_smoke COMMAND runtime_asset_path_smoke)

        add_executable(input_binding_codec_smoke
            tests/input_binding_codec_smoke.cpp
            app/action_input.cpp
            app/input_binding_codec.cpp
        )
        target_include_directories(input_binding_codec_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(input_binding_codec_smoke)
        add_test(NAME input_binding_codec_smoke COMMAND input_binding_codec_smoke)

        add_executable(sdl_options_binding_access_smoke
            tests/sdl_options_binding_access_smoke.cpp
            app/action_input.cpp
            app/sdl_options_binding_access.cpp
        )
        target_include_directories(sdl_options_binding_access_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(sdl_options_binding_access_smoke)
        add_test(NAME sdl_options_binding_access_smoke COMMAND sdl_options_binding_access_smoke)

        add_executable(input_profiles_smoke
            tests/input_profiles_smoke.cpp
            app/action_input.cpp
            app/input_profiles.cpp
        )
        target_include_directories(input_profiles_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(input_profiles_smoke)
        add_test(NAME input_profiles_smoke COMMAND input_profiles_smoke)

        add_executable(player_control_presets_smoke
            tests/player_control_presets_smoke.cpp
            app/action_input.cpp
            app/player_control_presets.cpp
        )
        target_include_directories(player_control_presets_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(player_control_presets_smoke)
        add_test(NAME player_control_presets_smoke COMMAND player_control_presets_smoke)

        add_executable(story_runtime_invariants_smoke
            tests/story_runtime_invariants_smoke.cpp
            app/story_runtime_invariants.cpp
        )
        target_link_libraries(story_runtime_invariants_smoke PRIVATE whacker_core)
        target_include_directories(story_runtime_invariants_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(story_runtime_invariants_smoke)
        add_test(NAME story_runtime_invariants_smoke COMMAND story_runtime_invariants_smoke)

        add_executable(play_control_human_smoke
            tests/play_control_human_smoke.cpp
            app/play_control_human.cpp
        )
        target_link_libraries(play_control_human_smoke PRIVATE whacker_core)
        target_include_directories(play_control_human_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(play_control_human_smoke)
        add_test(NAME play_control_human_smoke COMMAND play_control_human_smoke)

        add_executable(play_control_ai_stub_smoke
            tests/play_control_ai_stub_smoke.cpp
            ${WHACKER_AI_PLAY_CONTROL_RUNTIME_SOURCES}
        )
        target_link_libraries(play_control_ai_stub_smoke PRIVATE whacker_core)
        target_include_directories(play_control_ai_stub_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(play_control_ai_stub_smoke)
        add_test(NAME play_control_ai_stub_smoke COMMAND play_control_ai_stub_smoke)

        add_executable(ai_core_smoke
            tests/ai_core_smoke.cpp
            ${WHACKER_AI_PLANNER_SOURCES}
        )
        target_link_libraries(ai_core_smoke PRIVATE whacker_core)
        target_include_directories(ai_core_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(ai_core_smoke)
        add_test(NAME ai_core_smoke COMMAND ai_core_smoke)

        add_executable(ai_candidate_id_smoke
            tests/ai_candidate_id_smoke.cpp
            ${WHACKER_AI_PLANNER_SOURCES}
        )
        target_link_libraries(ai_candidate_id_smoke PRIVATE whacker_core)
        target_include_directories(ai_candidate_id_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(ai_candidate_id_smoke)
        add_test(NAME ai_candidate_id_smoke COMMAND ai_candidate_id_smoke)

        add_executable(ai_plan_state_bridge_smoke
            tests/ai_plan_state_bridge_smoke.cpp
            app/ai_plan_state_bridge.cpp
        )
        target_link_libraries(ai_plan_state_bridge_smoke PRIVATE whacker_core)
        target_include_directories(ai_plan_state_bridge_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(ai_plan_state_bridge_smoke)
        add_test(NAME ai_plan_state_bridge_smoke COMMAND ai_plan_state_bridge_smoke)

        add_executable(ai_style_fidelity_smoke
            tests/ai_style_fidelity_smoke.cpp
            ${WHACKER_AI_PLANNER_SOURCES}
        )
        target_link_libraries(ai_style_fidelity_smoke PRIVATE whacker_core)
        target_include_directories(ai_style_fidelity_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(ai_style_fidelity_smoke)
        add_test(NAME ai_style_fidelity_smoke COMMAND ai_style_fidelity_smoke)

        add_executable(ai_realized_spin_smoke
            tests/ai_realized_spin_smoke.cpp
            ${WHACKER_AI_PLAY_CONTROL_RUNTIME_SOURCES}
        )
        target_link_libraries(ai_realized_spin_smoke PRIVATE whacker_core)
        target_include_directories(ai_realized_spin_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(ai_realized_spin_smoke)
        add_test(NAME ai_realized_spin_smoke COMMAND ai_realized_spin_smoke)

        add_executable(ai_competence_ladder_smoke
            tests/ai_competence_ladder_smoke.cpp
            ${WHACKER_AI_PLAY_CONTROL_RUNTIME_SOURCES}
        )
        target_link_libraries(ai_competence_ladder_smoke PRIVATE whacker_core)
        target_include_directories(ai_competence_ladder_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(ai_competence_ladder_smoke)
        add_test(NAME ai_competence_ladder_smoke COMMAND ai_competence_ladder_smoke)

        add_executable(ai_determinism_trace_smoke
            tests/ai_determinism_trace_smoke.cpp
            ${WHACKER_AI_PLAY_CONTROL_RUNTIME_SOURCES}
        )
        target_link_libraries(ai_determinism_trace_smoke PRIVATE whacker_core)
        target_include_directories(ai_determinism_trace_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(ai_determinism_trace_smoke)
        add_test(NAME ai_determinism_trace_smoke COMMAND ai_determinism_trace_smoke)

        add_executable(ai_side_parity_smoke
            tests/ai_side_parity_smoke.cpp
            ${WHACKER_AI_PLANNER_SOURCES}
        )
        target_link_libraries(ai_side_parity_smoke PRIVATE whacker_core)
        target_include_directories(ai_side_parity_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(ai_side_parity_smoke)
        add_test(NAME ai_side_parity_smoke COMMAND ai_side_parity_smoke)

        add_executable(ai_canonical_symmetry_smoke
            tests/ai_canonical_symmetry_smoke.cpp
            ${WHACKER_AI_PLANNER_SOURCES}
        )
        target_link_libraries(ai_canonical_symmetry_smoke PRIVATE whacker_core)
        target_include_directories(ai_canonical_symmetry_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
        whacker_configure_smoke_test(ai_canonical_symmetry_smoke)
        add_test(NAME ai_canonical_symmetry_smoke COMMAND ai_canonical_symmetry_smoke)

    endif()
endif()
