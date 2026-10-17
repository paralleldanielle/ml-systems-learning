# Shared settings for exercise targets (see docs/engineering-practices.md, CPP-ERR-3).

# Warnings are errors for exercise code (not for third-party code such as GoogleTest).
function(learn_set_warnings target)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
endfunction()

# Optional clang-tidy on exercise code (enabled by the `tidy` preset).
function(learn_set_tidy target)
    if(LEARN_CLANG_TIDY)
        find_program(LEARN_CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
        set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${LEARN_CLANG_TIDY_EXE}")
    endif()
endfunction()

# Directory holding the code an exercise is built from: the student's work, or the teacher reference.
function(learn_code_root out_var exercise_dir)
    if(LEARN_USE_REFERENCE)
        set(${out_var} "${exercise_dir}/.teacher/reference" PARENT_SCOPE)
    else()
        set(${out_var} "${exercise_dir}" PARENT_SCOPE)
    endif()
endfunction()
