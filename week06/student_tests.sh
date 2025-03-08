#!/usr/bin/env bats

# File: student_tests.sh
#
# Additional unit tests for the custom shell (dsh).

@test "Confirm 'ls' executes without error" {
    run ./dsh <<EOF
ls
EOF

    # Validate that the shell continues normally (status 0)
    [ "$status" -eq 0 ]
}

@test "'exit' ends the shell session" {
    run ./dsh <<EOF
exit
EOF

    # Exit code should be zero, and the shell loop should print its termination message
    [ "$status" -eq 0 ]
    [[ "$output" == *"cmd loop returned 0"* ]]
}

@test "Change directory to a valid path" {
    tmpdir=$(mktemp -d)
    run ./dsh <<EOF
cd $tmpdir
pwd
exit
EOF
    # Remove temporary directory
    rmdir "$tmpdir"

    [ "$status" -eq 0 ]
    [[ "$output" == *"$tmpdir"* ]]
}

@test "Show error for missing command" {
    run ./dsh <<EOF
this_command_does_not_exist
EOF

    # The shell stays alive (status 0), but we expect an error message
    [ "$status" -eq 0 ]
    [[ "$output" == *"Command not found"* ]]
}

@test "Empty or whitespace-only command produces warning" {
    run ./dsh <<EOF
    
    # Only spaces or newline
EOF

    [ "$status" -eq 0 ]
    [[ "$output" == *"warning: no commands"* ]]
}
