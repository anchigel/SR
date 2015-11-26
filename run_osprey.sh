#!/bin/bash
#
# Author: Mark Gottscho
# Email: mgottscho@ucla.edu
#
# Tests SnakeRouter with an Osprey program.
#
# Example usage:
#
# ./run_osprey.sh AND2_X1 min minimum

CELL=$1
DESIGNRULES=$2
DESIGNRULES_FULL=$3

./Osprey $CELL $CELL\_routed_$DESIGNRULES\rule testcases/$CELL.netlist testcases/$DESIGNRULES_FULL.designrules

