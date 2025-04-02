# Copyright 2017-2025 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Common routines for testing multi-target features.

load_lib gdbserver-support.exp

standard_testfile multi-target.c

# Keep a list of (inferior ID, spawn ID).
set server_spawn_ids [list]

proc connect_target_extended_remote {binfile num} {
    set res [gdbserver_start "--multi" ""]
    global server_spawn_ids server_spawn_id
    lappend server_spawn_ids $num $server_spawn_id
    set gdbserver_gdbport [lindex $res 1]
    return [gdb_target_cmd "extended-remote" $gdbserver_gdbport]
}

# Add and start inferior number NUM.  Returns true on success, false
# otherwise.
proc add_inferior {num target binfile {gcorefile ""}} {
    # Start another inferior.
    gdb_test "add-inferior -no-connection" "Added inferior $num" \
	"add empty inferior $num"
    gdb_test "inferior $num" "Switching to inferior $num.*" \
	"switch to inferior $num"
    gdb_test "file ${binfile}" ".*" "load file in inferior $num"
    gdb_test_no_output "set remote exec-file ${binfile}" \
	"set remote-exec file in inferior $num"

    if {$target == "core"} {
	gdb_test "core $gcorefile" "Core was generated by.*" \
	    "core [file tail $gcorefile], inf $num"
	return 1
    }

    if {$target == "extended-remote"} {
	if {[connect_target_extended_remote $binfile $num]} {
	    return 0
	}
    }
    if ![runto "all_started"] then {
	return 0
    }
    delete_breakpoints

    return 1
}

proc prepare_core {} {
    global gcorefile gcore_created
    global binfile

    clean_restart ${binfile}

    if ![runto all_started] then {
	return -1
    }

    global testfile
    set gcorefile [standard_output_file $testfile.gcore]
    set gcore_created [gdb_gcore_cmd $gcorefile "save a core file"]
}

proc next_live_inferior {inf} {
    incr inf
    if {$inf == 3} {
	# 3 is a core.
	return 4
    }
    if {$inf > 5} {
	# 6 is a core.
	return 1
    }

    return $inf
}

# Clean up the server_spawn_ids.
proc cleanup_gdbservers { } {
    global server_spawn_id
    global server_spawn_ids
    foreach { inferior_id spawn_id } $server_spawn_ids {
	set server_spawn_id $spawn_id
	gdb_test "inferior $inferior_id"
	gdbserver_exit 0
    }
    set server_spawn_ids [list]
}

# Return true on success, false otherwise.

proc setup {non-stop {multi_process ""}} {
    global gcorefile gcore_created
    global binfile

    cleanup_gdbservers

    save_vars { ::GDBFLAGS } {
	# Make GDB read files from the local file system, not through the
	# remote targets, to speed things up.
	set ::GDBFLAGS "${::GDBFLAGS} -ex \"set sysroot\""
	clean_restart ${binfile}
    }

    # multi-target depends on target running in non-stop mode.  Force
    # it on for remote targets, until this is the default.
    gdb_test_no_output "maint set target-non-stop on"

    gdb_test_no_output "set non-stop ${non-stop}"

    if {${multi_process} ne ""} then {
	gdb_test \
	    "set remote multiprocess-feature-packet $multi_process" \
	    "Support for the 'multiprocess-feature' packet on future remote targets is set to \"${multi_process}\"."
    }

    if ![runto all_started] then {
	return 0
    }

    delete_breakpoints

    # inferior 1 -> native
    # inferior 2 -> extended-remote
    # inferior 3 -> core
    # inferior 4 -> native
    # inferior 5 -> extended-remote
    # inferior 6 -> core
    if {![add_inferior 2 "extended-remote" $binfile]} {
	return 0
    }
    if {![add_inferior 3 "core" $binfile $gcorefile]} {
	return 0
    }
    if {![add_inferior 4 "native" $binfile]} {
	return 0
    }
    if {![add_inferior 5 "extended-remote" $binfile]} {
	return 0
    }
    if {![add_inferior 6 "core" $binfile $gcorefile]} {
	return 0
    }

    # For debugging.
    gdb_test "info threads" ".*"

    # Make "continue" resume all inferiors.
    if {${non-stop} == "off"} {
	gdb_test_no_output "set schedule-multiple on"
    }

    return 1
}

proc multi_target_prepare {} {
    global binfile srcfile

    if { ![allow_gdbserver_tests] } {
	return 0
    }

    # The plain remote target can't do multiple inferiors.
    if {[target_info gdb_protocol] != ""} {
	return 0
    }

    if { [prepare_for_testing "failed to prepare" ${binfile} "${srcfile}" \
	      {debug pthreads}] } {
	return 0
    }

    # Make a core file with two threads upfront.  Several tests load
    # the same core file.
    prepare_core

    return 1
}

proc multi_target_cleanup {} {
    cleanup_gdbservers
}
