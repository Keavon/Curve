# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/gyatso/Documents/cpe476/CPE-476-Project-Curve

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/gyatso/Documents/cpe476/CPE-476-Project-Curve/build

# Utility rule file for releasable.

# Include the progress variables for this target.
include yaml-cpp/CMakeFiles/releasable.dir/progress.make

yaml-cpp/CMakeFiles/releasable:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/gyatso/Documents/cpe476/CPE-476-Project-Curve/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Adjusting settings for release compilation"
	cd /home/gyatso/Documents/cpe476/CPE-476-Project-Curve/build/yaml-cpp && $(MAKE) clean
	cd /home/gyatso/Documents/cpe476/CPE-476-Project-Curve/build/yaml-cpp && /usr/bin/cmake -DCMAKE_BUILD_TYPE=Release /home/gyatso/Documents/cpe476/CPE-476-Project-Curve

releasable: yaml-cpp/CMakeFiles/releasable
releasable: yaml-cpp/CMakeFiles/releasable.dir/build.make

.PHONY : releasable

# Rule to build all files generated by this target.
yaml-cpp/CMakeFiles/releasable.dir/build: releasable

.PHONY : yaml-cpp/CMakeFiles/releasable.dir/build

yaml-cpp/CMakeFiles/releasable.dir/clean:
	cd /home/gyatso/Documents/cpe476/CPE-476-Project-Curve/build/yaml-cpp && $(CMAKE_COMMAND) -P CMakeFiles/releasable.dir/cmake_clean.cmake
.PHONY : yaml-cpp/CMakeFiles/releasable.dir/clean

yaml-cpp/CMakeFiles/releasable.dir/depend:
	cd /home/gyatso/Documents/cpe476/CPE-476-Project-Curve/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gyatso/Documents/cpe476/CPE-476-Project-Curve /home/gyatso/Documents/cpe476/CPE-476-Project-Curve/yaml-cpp /home/gyatso/Documents/cpe476/CPE-476-Project-Curve/build /home/gyatso/Documents/cpe476/CPE-476-Project-Curve/build/yaml-cpp /home/gyatso/Documents/cpe476/CPE-476-Project-Curve/build/yaml-cpp/CMakeFiles/releasable.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : yaml-cpp/CMakeFiles/releasable.dir/depend

