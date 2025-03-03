#!/usr/bin/env bats

@test "Local: ls runs without errors" {
  run ./dsh <<'EOF'
ls
exit
EOF
  # Check that the shell exits with 0 status.
  [ "$status" -eq 0 ]
}

@test "Local: pipeline 'ls | grep dshlib.c' returns dshlib.c" {
  run ./dsh <<'EOF'
ls | grep dshlib.c
exit
EOF
  # Check that output includes "dshlib.c"
  echo "$output" | grep -q "dshlib.c"
  [ "$status" -eq 0 ]
}

@test "Local: exit command terminates shell" {
  run ./dsh <<'EOF'
exit
EOF
  # Verify that the exit message is printed (adjust expected text if needed)
  echo "$output" | grep -q "exiting..."
  [ "$status" -eq 0 ]
}

@test "Local: empty input prints warning" {
  run ./dsh <<'EOF'
  
exit
EOF
  # Check for the expected warning message
  echo "$output" | grep -q "warning: no commands provided"
  [ "$status" -eq 0 ]
}
