# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

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
CMAKE_SOURCE_DIR = /home/pi/apriltags

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pi/apriltags/pod-build

# Utility rule file for tmp.

# Include the progress variables for this target.
include CMakeFiles/tmp.dir/progress.make

tmp: CMakeFiles/tmp.dir/build.make

.PHONY : tmp

# Rule to build all files generated by this target.
CMakeFiles/tmp.dir/build: tmp

.PHONY : CMakeFiles/tmp.dir/build

CMakeFiles/tmp.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/tmp.dir/cmake_clean.cmake
.PHONY : CMakeFiles/tmp.dir/clean

CMakeFiles/tmp.dir/depend:
	cd /home/pi/apriltags/pod-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pi/apriltags /home/pi/apriltags /home/pi/apriltags/pod-build /home/pi/apriltags/pod-build /home/pi/apriltags/pod-build/CMakeFiles/tmp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/tmp.dir/depend

