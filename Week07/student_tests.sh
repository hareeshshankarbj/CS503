#!/usr/bin/env bats

@test "Pipeline: ls | grep dshlib.c returns dshlib.c" {
  run ./dsh <<'EOF'
ls | grep dshlib.c
exit
EOF
  # Check that the output includes "dshlib.c"
  echo "$output" | grep -q "dshlib.c"
}

@test "Exit command terminates shell" {
  run ./dsh <<'EOF'
exit
EOF
  # Check exit status and that "exiting..." is printed
  [ "$status" -eq 0 ]
  echo "$output" | grep -q "exiting..."
}

@test "Empty input prints warning" {
  run ./dsh <<'EOF'
  
exit
EOF
  # Check that the output contains the warning about no commands
  echo "$output" | grep -q "warning: no commands provided"
}
