#!/usr/bin/env bash
# Basic integration test for ccsh

echo "Testing ccsh..."

./ccsh <<'EOF'
echo hello > test.txt
cat < test.txt
ls *.txt
alias ll="ls"
ll
sleep 1 &
jobs
help
exit
EOF

echo "Test complete."
