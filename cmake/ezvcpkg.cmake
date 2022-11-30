set(EZVCPKG_VERSION 0.1)

macro(EZVCPKG_CALCULATE_PATHS)
  if(NOT EZVCPKG_BASEDIR)
    if(DEFINED ENV{EZVCPKG_BASEDIR})
      set(EZVCPKG_BASEDIR $ENV{EZVCPKG_BASEDIR})
    else()
      if(APPLE)
        # OSX wipes binaries from the temp directory over time, leaving the
        # remaining files This causes the vcpkg folder to end up in an unusable
        # state, so we default to the home directory instead
        set(EZVCPKG_BASEDIR "$ENV{HOME}/.ezvcpkg")
      else()
        # Initially defaulted to $ENV{TEMP} but the github build hosts don't
        # populate this
        set(EZVCPKG_BASEDIR "$ENV{HOME}/.ezvcpkg")
      endif()
      # We want people to specify a base directory, either through the calling
      # EZVCPKG_FETCH function or through an environment variable.
      message(
        STATUS
          "EZVCPKG_BASEDIR envrionment variable not found and basedir not set, using default ${EZVCPKG_BASEDIR}"
      )
    endif()
  endif()
  file(TO_CMAKE_PATH "${EZVCPKG_BASEDIR}/${EZVCPKG_COMMIT}" EZVCPKG_DIR)
  file(TO_CMAKE_PATH "${EZVCPKG_BASEDIR}/${EZVCPKG_COMMIT}.lock" EZVCPKG_LOCK)

  if(EZVCPKG_USE_HOST_VCPKG)
    find_program(EZVCPKG_HOST_VCPKG vcpkg)
    if(EZVCPKG_HOST_VCPKG)
      message(STATUS "EZVCPKG using host vcpkg binary ${EZVCPKG_HOST_VCPKG}")
    endif()
  endif()

  if(WIN32)
    file(TO_CMAKE_PATH "${EZVCPKG_DIR}/vcpkg.exe" EZVCPKG_EXE)
    file(TO_CMAKE_PATH "${EZVCPKG_DIR}/bootstrap-vcpkg.bat" EZVCPKG_BOOTSTRAP)
  else()
    file(TO_CMAKE_PATH "${EZVCPKG_DIR}/vcpkg" EZVCPKG_EXE)
    file(TO_CMAKE_PATH "${EZVCPKG_DIR}/bootstrap-vcpkg.sh" EZVCPKG_BOOTSTRAP)
  endif()

  # The tag file exists purely to be a touch target every time the ezvcpkg macro
  # is called making it easy to find out of date ezvcpkg folder.
  file(TO_CMAKE_PATH "${EZVCPKG_DIR}/.tag" EZVCPKG_TAG)
  file(TO_CMAKE_PATH "${EZVCPKG_DIR}/README.md" EZVCPKG_README)

  # The whole host-triplet / triplet setup is to support cross compiling,
  # specifically for things like android.  The idea is that some things you
  # might need from vcpkg to act as tools to execute on the host system, like
  # glslang and spirv-cross, while other things you need as binaries compatible
  # with the target system.  However, this isn't fully fleshed out, so don't try
  # to use it yet
  if(WIN32)
    set(EZVCPKG_HOST_TRIPLET "x64-windows")
  elseif(APPLE)
    set(EZVCPKG_HOST_TRIPLET "x64-osx")
  else()
    set(EZVCPKG_HOST_TRIPLET "x64-linux")
  endif()

  if(ANDROID)
    set(EZVCPKG_TRIPLET "arm64-android")
  else()
    set(EZVCPKG_TRIPLET ${EZVCPKG_HOST_TRIPLET})
  endif()

  file(TO_CMAKE_PATH ${EZVCPKG_DIR}/installed/${EZVCPKG_TRIPLET}
       EZVCPKG_INSTALLED_DIR)
  file(TO_CMAKE_PATH ${EZVCPKG_DIR}/scripts/buildsystems/vcpkg.cmake
       EZVCPKG_CMAKE_TOOLCHAIN)
endmacro()

macro(EZVCPKG_BOOTSTRAP)
  if(NOT EXISTS ${EZVCPKG_README})
    message(STATUS "EZVCPKG Bootstrapping")
    find_package(Git)
    if(NOT Git_FOUND)
      message(FATAL_ERROR "EZVCPKG Git not found, can't bootstrap vcpkg")
    endif()
    message(STATUS "EZVCPKG Cloning repository")
    execute_process(COMMAND ${GIT_EXECUTABLE} "clone" ${EZVCPKG_URL}
                            ${EZVCPKG_DIR} OUTPUT_QUIET)

    # FIXME put this into a separate check verifying the commit ID of the
    # current directory If the previous run had the clone work, but the checkout
    # fail, the readme will be present and the user will have the default
    # checkout, rather than their requested commit
    message(STATUS "EZVCPKG Checking out commit ${EZVCPKG_COMMIT}")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} "-c" "advice.detachedHead=false" "checkout"
              ${EZVCPKG_COMMIT}
      WORKING_DIRECTORY ${EZVCPKG_DIR}
      OUTPUT_QUIET)
  endif()

  if(EZVCPKG_HOST_VCPKG)
    set(EZVCPKG_EXE ${EZVCPKG_HOST_VCPKG})
  elseif(NOT EXISTS ${EZVCPKG_EXE})
    message(STATUS "EZVCPKG Bootstrapping vcpkg binary")
    execute_process(
      COMMAND ${EZVCPKG_BOOTSTRAP}
      WORKING_DIRECTORY ${EZVCPKG_DIR}
      OUTPUT_QUIET)
  endif()

  if(NOT EXISTS ${EZVCPKG_EXE})
    message(FATAL_ERROR "EZVCPKG vcpkg binary not failed")
  endif()
endmacro()

macro(EZVCPKG_BUILD)
  if(EZVCPKG_SERIALIZE)
    foreach(_PACKAGE ${EZVCPKG_PACKAGES})
      message(STATUS "EZVCPKG Building/Verifying package ${_PACKAGE}")
      execute_process(
        COMMAND ${EZVCPKG_EXE} --vcpkg-root ${EZVCPKG_DIR} install --triplet
                ${EZVCPKG_TRIPLET} ${_PACKAGE}
        WORKING_DIRECTORY ${EZVCPKG_DIR}
        RESULT_VARIABLE EZVCPKG_RESULT
        ERROR_VARIABLE EZVCPKG_ERROR
        OUTPUT_VARIABLE EZVCPKG_OUTPUT)
    endforeach()
  else()
    message(STATUS "EZVCPKG Building/Verifying packages ${EZVCPKG_PACKAGES}")
    execute_process(
      COMMAND ${EZVCPKG_EXE} --vcpkg-root ${EZVCPKG_DIR} install --triplet
              ${EZVCPKG_TRIPLET} ${EZVCPKG_PACKAGES}
      WORKING_DIRECTORY ${EZVCPKG_DIR}
      RESULT_VARIABLE EZVCPKG_RESULT
      ERROR_VARIABLE EZVCPKG_ERROR
      OUTPUT_VARIABLE EZVCPKG_OUTPUT)
  endif()
  if(${EZVCPKG_RESULT})
    message(
      FATAL_ERROR
        "EZVCPKG vcpkg execute returned failure ${EZVCPKG_ERROR} ${EZVCPKG_OUTPUT}"
    )
  endif()

  file(TO_CMAKE_PATH ${EZVCPKG_DIR}/buildtrees EZVCPKG_BUILDTREES)
  if((EXISTS ${EZVCPKG_BUILDTREES}) AND (NOT EZVCPKG_SAVE_BUILD))
    message("wiping build trees")
    file(REMOVE_RECURSE "${EZVCPKG_DIR}/buildtrees")
  endif()
endmacro()

macro(EZVCPKG_CLEAN)
  message(
    STATUS "EZVCPKG Cleaning unused ezvcpkg dirs after ${EZVCPKG_CLEAN} days")
  message(WARNING "Cleaning not implemented")
endmacro()

macro(EZVCPKG_CHECK_ARGS)
  # Default to a recent vcpkg commit
  if(NOT EZVCPKG_COMMIT)
    set(EZVCPKG_COMMIT "af2287382b1991dbdcb7e5112d236f3323b9dd7a")
  endif()

  if(EZVCPKG_REPO AND EZVCPKG_URL)
    message(
      FATAL_ERROR
        "Set either a Git repository URL or a REPO org/name combination, but not both"
    )
  endif()

  # Default to the microsoft root vcpkg repository
  if((NOT EZVCPKG_URL) AND (NOT EZVCPKG_REPO))
    set(EZVCPKG_REPO "microsoft/vcpkg")
  endif()

  # Default to github
  if(NOT EZVCPKG_URL)
    set(EZVCPKG_URL "https://github.com/${EZVCPKG_REPO}.git")
  endif()

  if(NOT EZVCPKG_PACKAGES)
    message(FATAL_ERROR "No vcpkg packages specified")
  endif()
endmacro()

function(EZVCPKG_FETCH_IMPL)
  if(NOT EZVCPKG_ANNOUNCED)
    set(EZVCPKG_ANNOUNCED
        1
        PARENT_SCOPE)
    message(
      STATUS
        "EZVCPKG v${EZVCPKG_VERSION} starting up\n\tWebsite: https://github.com/jherico/ezvcpkg"
    )
  endif()

  set(options UPDATE_TOOLCHAIN SERIALIZE USE_HOST_VCPKG SAVE_BUILD)
  set(oneValueArgs COMMIT URL REPO BASEDIR OUTPUT CLEAN)
  set(multiValueArgs PACKAGES)
  cmake_parse_arguments(EZVCPKG "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # Validate arguments
  ezvcpkg_check_args()

  # Figure out the paths for everything
  ezvcpkg_calculate_paths()

  message(
    STATUS
      "EZVCPKG initializing\n\tcommit:     ${EZVCPKG_COMMIT}\n\trepository: ${EZVCPKG_URL}\n\tlocal dir:  ${EZVCPKG_DIR}"
  )

  # We can't assume build systems will be well behaved if we attempt two
  # different concurrent builds of the same target, so we need to have some
  # locking here to ensure that for a given vcpkg location, only one instance of
  # vcpkg runs at a given time.

  # This is critical because, for instance, when Android Studio does a native
  # code build it will concurrently execute cmake for debug and release versions
  # of the build, in different directories, and therefore concurrently try to
  # bootstrap and run vcpkg, which will almost certainly corrupt your build
  # directories
  file(LOCK ${EZVCPKG_LOCK})

  ezvcpkg_bootstrap()

  # Touch the tag file so that this directory is marked as recent, and therefore
  # not cleanable
  file(TOUCH ${EZVCPKG_TAG})

  # While still holding the global lock, clean up old build directories
  if(EZVCPKG_CLEAN)
    ezvcpkg_clean()
  endif()

  # Build packages
  ezvcpkg_build()

  file(LOCK ${EZVCPKG_LOCK} RELEASE)
  file(REMOVE ${EZVCPKG_LOCK})

  set(EZVCPKG_DIR
      "${EZVCPKG_INSTALLED_DIR}"
      PARENT_SCOPE)
  if(EZVCPKG_UPDATE_TOOLCHAIN)
    set(CMAKE_TOOLCHAIN_FILE
        ${EZVCPKG_CMAKE_TOOLCHAIN}
        PARENT_SCOPE)
  endif()
  message(STATUS "EZVCPKG done")
endfunction()

macro(EZVCPKG_FETCH)
  ezvcpkg_fetch_impl(${ARGN})
  if(EZVCPKG_QUIT)
    unset(EZVCPKG_QUIT CACHE)
    message(STATUS "EZVCPKG quitting")
    return()
  endif()
endmacro()
