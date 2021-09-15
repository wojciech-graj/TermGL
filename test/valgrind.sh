#!/bin/bash
cd "$(dirname "$0")"

valgrind --log-file="termgl.txt" --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./test_

## for SERIOUS debugging!
#--vgdb-error=0
