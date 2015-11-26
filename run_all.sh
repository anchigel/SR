#!/bin/bash
#
# This script automates the complete router flow.
#
# Authors: Mark Gottscho and Yasmine Badr
# Email: mgottscho@ucla.edu, ybadr@ucla.edu
# Copyright (C) 2013 Mark Gottscho and Yasmine Badr

# Set up paths
LOGS=logs
OA_DATABASE=DesignLib
DRC_TEMPLATES=templates/drc
DRC=drc
LVS_TEMPLATES=templates/lvs
LVS=lvs
OUTPUT=output
DRC_OUTPUT=$OUTPUT/drc
#DIRECTION_OUTPUT=$DRC_OUTPUT/direction_check
LVS_OUTPUT=$OUTPUT/lvs
TESTCASES=testcases

# Clean directories
echo "Cleaning directories..."
./clean.sh

# Setup fresh directories
echo "Initializing directories..."
mkdir $LOGS
mkdir $OA_DATABASE
mkdir $DRC
mkdir $LVS
mkdir $OUTPUT
mkdir $DRC_OUTPUT
mkdir $LVS_OUTPUT
#mkdir $DIRECTION_OUTPUT

# Import test cases
echo "Importing test cases to OA..."
./import_testcase_gds_to_oa.sh $TESTCASES > $LOGS/import.log

# Run all min rule cases
echo "Running minimum design rule test cases..."
echo "...AND2_X1..."
time ./SnakeRouter AND2_X1 AND2_X1_routed_minrule testcases/AND2_X1.netlist testcases/minimum.designrules > $LOGS/AND2_X1_routed_minrule.log
echo "...AND2_X4..."
time ./SnakeRouter AND2_X4 AND2_X4_routed_minrule testcases/AND2_X4.netlist testcases/minimum.designrules > $LOGS/AND2_X4_routed_minrule.log
echo "...DFF_X2..."
time ./SnakeRouter DFF_X2 DFF_X2_routed_minrule testcases/DFF_X2.netlist testcases/minimum.designrules > $LOGS/DFF_X2_routed_minrule.log
echo "...INV_X1..."
time ./SnakeRouter INV_X1 INV_X1_routed_minrule testcases/INV_X1.netlist testcases/minimum.designrules > $LOGS/INV_X1_routed_minrule.log
echo "...INV_X32..."
time ./SnakeRouter INV_X32 INV_X32_routed_minrule testcases/INV_X32.netlist testcases/minimum.designrules > $LOGS/INV_X32_routed_minrule.log
echo "...LOGIC0_X1..."
time ./SnakeRouter LOGIC0_X1 LOGIC0_X1_routed_minrule testcases/LOGIC0_X1.netlist testcases/minimum.designrules > $LOGS/LOGIC0_X1_routed_minrule.log
echo "...OAI21_X1..."
time ./SnakeRouter OAI21_X1 OAI21_X1_routed_minrule testcases/OAI21_X1.netlist testcases/minimum.designrules > $LOGS/OAI21_X1_routed_minrule.log
#### IGNORE ####
#echo "...HIDDEN1..."
#time ./SnakeRouter HIDDEN1 HIDDEN1_routed_minrule testcases/HIDDEN1.netlist testcases/minimum.designrules > $LOGS/HIDDEN1_routed_minrule.log
#echo "...HIDDEN2..."
#time ./SnakeRouter HIDDEN2 HIDDEN2_routed_minrule testcases/HIDDEN2.netlist testcases/minimum.designrules > $LOGS/HIDDEN2_routed_minrule.log
#echo "...HIDDEN3..."
#time ./SnakeRouter HIDDEN3 HIDDEN3_routed_minrule testcases/HIDDEN3.netlist testcases/minimum.designrules > $LOGS/HIDDEN3_routed_minrule.log
################

# Run all inter rule cases
echo "Running inter design rule test cases..."
echo "...AND2_X1..."
time ./SnakeRouter AND2_X1 AND2_X1_routed_interrule testcases/AND2_X1.netlist testcases/inter.designrules > $LOGS/AND2_X1_routed_interrule.log
echo "...AND2_X4..."
time ./SnakeRouter AND2_X4 AND2_X4_routed_interrule testcases/AND2_X4.netlist testcases/inter.designrules > $LOGS/AND2_X4_routed_interrule.log
echo "...DFF_X2..."
time ./SnakeRouter DFF_X2 DFF_X2_routed_interrule testcases/DFF_X2.netlist testcases/inter.designrules > $LOGS/DFF_X2_routed_interrule.log
echo "...INV_X1..."
time ./SnakeRouter INV_X1 INV_X1_routed_interrule testcases/INV_X1.netlist testcases/inter.designrules > $LOGS/INV_X1_routed_interrule.log
echo "...INV_X32..."
time ./SnakeRouter INV_X32 INV_X32_routed_interrule testcases/INV_X32.netlist testcases/inter.designrules > $LOGS/INV_X32_routed_interrule.log
echo "...LOGIC0_X1..."
time ./SnakeRouter LOGIC0_X1 LOGIC0_X1_routed_interrule testcases/LOGIC0_X1.netlist testcases/inter.designrules > $LOGS/LOGIC0_X1_routed_interrule.log
echo "...OAI21_X1..."
time ./SnakeRouter OAI21_X1 OAI21_X1_routed_interrule testcases/OAI21_X1.netlist testcases/inter.designrules > $LOGS/OAI21_X1_routed_interrule.log
#### IGNORE ####
#echo "...HIDDEN1..."
#time ./SnakeRouter HIDDEN1 HIDDEN1_routed_interrule testcases/HIDDEN1.netlist testcases/inter.designrules > $LOGS/HIDDEN1_routed_interrule.log
#echo "...HIDDEN2..."
#time ./SnakeRouter HIDDEN2 HIDDEN2_routed_interrule testcases/HIDDEN2.netlist testcases/inter.designrules > $LOGS/HIDDEN2_routed_interrule.log
#echo "...HIDDEN3..."
#time ./SnakeRouter HIDDEN3 HIDDEN3_routed_interrule testcases/HIDDEN3.netlist testcases/inter.designrules > $LOGS/HIDDEN3_routed_interrule.log
################

# Run all rand rule cases
echo "Running rand design rule test cases..."
echo "...AND2_X1..."
time ./SnakeRouter AND2_X1 AND2_X1_routed_randrule testcases/AND2_X1.netlist testcases/random.designrules > $LOGS/AND2_X1_routed_randrule.log
echo "...AND2_X4..."
time ./SnakeRouter AND2_X4 AND2_X4_routed_randrule testcases/AND2_X4.netlist testcases/random.designrules > $LOGS/AND2_X4_routed_randrule.log
echo "...DFF_X2..."
time ./SnakeRouter DFF_X2 DFF_X2_routed_randrule testcases/DFF_X2.netlist testcases/random.designrules > $LOGS/DFF_X2_routed_randrule.log
echo "...INV_X1..."
time ./SnakeRouter INV_X1 INV_X1_routed_randrule testcases/INV_X1.netlist testcases/random.designrules > $LOGS/INV_X1_routed_randrule.log
echo "...INV_X32..."
time ./SnakeRouter INV_X32 INV_X32_routed_randrule testcases/INV_X32.netlist testcases/random.designrules > $LOGS/INV_X32_routed_randrule.log
echo "...LOGIC0_X1..."
time ./SnakeRouter LOGIC0_X1 LOGIC0_X1_routed_randrule testcases/LOGIC0_X1.netlist testcases/random.designrules > $LOGS/LOGIC0_X1_routed_randrule.log
echo "...OAI21_X1..."
time ./SnakeRouter OAI21_X1 OAI21_X1_routed_randrule testcases/OAI21_X1.netlist testcases/random.designrules > $LOGS/OAI21_X1_routed_randrule.log
#### IGNORE ####
#echo "...HIDDEN1..."
#time ./SnakeRouter HIDDEN1 HIDDEN1_routed_randrule testcases/HIDDEN1.netlist testcases/random.designrules > $LOGS/HIDDEN1_routed_randrule.log
#echo "...HIDDEN2..."
#time ./SnakeRouter HIDDEN2 HIDDEN2_routed_randrule testcases/HIDDEN2.netlist testcases/random.designrules > $LOGS/HIDDEN2_routed_randrule.log
#echo "...HIDDEN3..."
#time ./SnakeRouter HIDDEN3 HIDDEN3_routed_randrule testcases/HIDDEN3.netlist testcases/random.designrules > $LOGS/HIDDEN3_routed_randrule.log
################


# Run all max rule cases
echo "Running max design rule test cases..."
echo "...AND2_X1..."
time ./SnakeRouter AND2_X1 AND2_X1_routed_maxrule testcases/AND2_X1.netlist testcases/maximum.designrules > $LOGS/AND2_X1_routed_maxrule.log
echo "...AND2_X4..."
time ./SnakeRouter AND2_X4 AND2_X4_routed_maxrule testcases/AND2_X4.netlist testcases/maximum.designrules > $LOGS/AND2_X4_routed_maxrule.log
echo "...DFF_X2..."
time ./SnakeRouter DFF_X2 DFF_X2_routed_maxrule testcases/DFF_X2.netlist testcases/maximum.designrules > $LOGS/DFF_X2_routed_maxrule.log
echo "...INV_X1..."
time ./SnakeRouter INV_X1 INV_X1_routed_maxrule testcases/INV_X1.netlist testcases/maximum.designrules > $LOGS/INV_X1_routed_maxrule.log
echo "...INV_X32..."
time ./SnakeRouter INV_X32 INV_X32_routed_maxrule testcases/INV_X32.netlist testcases/maximum.designrules > $LOGS/INV_X32_routed_maxrule.log
echo "...LOGIC0_X1..."
time ./SnakeRouter LOGIC0_X1 LOGIC0_X1_routed_maxrule testcases/LOGIC0_X1.netlist testcases/maximum.designrules > $LOGS/LOGIC0_X1_routed_maxrule.log
echo "...OAI21_X1..."
time ./SnakeRouter OAI21_X1 OAI21_X1_routed_maxrule testcases/OAI21_X1.netlist testcases/maximum.designrules > $LOGS/OAI21_X1_routed_maxrule.log
#### IGNORE ####
#echo "...HIDDEN1..."
#time ./SnakeRouter HIDDEN1 HIDDEN1_routed_maxrule testcases/HIDDEN1.netlist testcases/maximum.designrules > $LOGS/HIDDEN1_routed_maxrule.log
#echo "...HIDDEN2..."
#time ./SnakeRouter HIDDEN2 HIDDEN2_routed_maxrule testcases/HIDDEN2.netlist testcases/maximum.designrules > $LOGS/HIDDEN2_routed_maxrule.log
#echo "...HIDDEN3..."
#time ./SnakeRouter HIDDEN3 HIDDEN3_routed_maxrule testcases/HIDDEN3.netlist testcases/maximum.designrules > $LOGS/HIDDEN3_routed_maxrule.log
################

echo "Done routing! Exporting routed results from OA to gds..."
./export_results_to_gds.sh $OA_DATABASE > $LOGS/export.log

# LVS checks
echo "Done exporting! Doing LVS checks..."
cp $LVS_TEMPLATES/* $LVS/
cd $LVS
./run_lvs.sh > ../logs/lvs.log
cd ..

# DRC checks
echo "Done with LVS check! Doing DRC checks..."
cp $DRC_TEMPLATES/* $DRC/
cd $DRC
./run_drc.sh > ../logs/drc.log
cd ..

# Extract wirelength and via metrics
echo "Extracting wirelength and via metrics with Osprey..."
./run_osprey.sh AND2_X1 min minimum > logs/osprey.log 2>/dev/null
./run_osprey.sh AND2_X1 inter inter >> logs/osprey.log 2>/dev/null
./run_osprey.sh AND2_X1 rand random >> logs/osprey.log 2>/dev/null
./run_osprey.sh AND2_X1 max maximum >> logs/osprey.log 2>/dev/null

./run_osprey.sh AND2_X4 min minimum >> logs/osprey.log 2>/dev/null
./run_osprey.sh AND2_X4 inter inter >> logs/osprey.log 2>/dev/null
./run_osprey.sh AND2_X4 rand random >> logs/osprey.log 2>/dev/null
./run_osprey.sh AND2_X4 max maximum >> logs/osprey.log 2>/dev/null

./run_osprey.sh DFF_X2 min minimum >> logs/osprey.log 2>/dev/null
./run_osprey.sh DFF_X2 inter inter >> logs/osprey.log 2>/dev/null
./run_osprey.sh DFF_X2 rand random >> logs/osprey.log 2>/dev/null
./run_osprey.sh DFF_X2 max maximum >> logs/osprey.log 2>/dev/null

./run_osprey.sh INV_X1 min minimum >> logs/osprey.log 2>/dev/null
./run_osprey.sh INV_X1 inter inter >> logs/osprey.log 2>/dev/null
./run_osprey.sh INV_X1 rand random >> logs/osprey.log 2>/dev/null
./run_osprey.sh INV_X1 max maximum >> logs/osprey.log 2>/dev/null

./run_osprey.sh INV_X32 min minimum >> logs/osprey.log 2>/dev/null
./run_osprey.sh INV_X32 inter inter >> logs/osprey.log 2>/dev/null
./run_osprey.sh INV_X32 rand random >> logs/osprey.log 2>/dev/null
./run_osprey.sh INV_X32 max maximum >> logs/osprey.log 2>/dev/null

./run_osprey.sh LOGIC0_X1 min minimum >> logs/osprey.log 2>/dev/null
./run_osprey.sh LOGIC0_X1 inter inter >> logs/osprey.log 2>/dev/null
./run_osprey.sh LOGIC0_X1 rand random >> logs/osprey.log 2>/dev/null
./run_osprey.sh LOGIC0_X1 max maximum >> logs/osprey.log 2>/dev/null

./run_osprey.sh OAI21_X1 min minimum >> logs/osprey.log 2>/dev/null
./run_osprey.sh OAI21_X1 inter inter >> logs/osprey.log 2>/dev/null
./run_osprey.sh OAI21_X1 rand random >> logs/osprey.log 2>/dev/null
./run_osprey.sh OAI21_X1 max maximum >> logs/osprey.log 2>/dev/null

#./run_osprey.sh HIDDEN1 min minimum >> logs/osprey.log 2>/dev/null
#./run_osprey.sh HIDDEN1 inter inter >> logs/osprey.log 2>/dev/null
#./run_osprey.sh HIDDEN1 rand random >> logs/osprey.log 2>/dev/null
#./run_osprey.sh HIDDEN1 max maximum >> logs/osprey.log 2>/dev/null

#./run_osprey.sh HIDDEN2 min minimum >> logs/osprey.log 2>/dev/null
#./run_osprey.sh HIDDEN2 inter inter >> logs/osprey.log 2>/dev/null
#./run_osprey.sh HIDDEN2 rand random >> logs/osprey.log 2>/dev/null
#./run_osprey.sh HIDDEN2 max maximum >> logs/osprey.log 2>/dev/null

#./run_osprey.sh HIDDEN3 min minimum >> logs/osprey.log 2>/dev/null
#./run_osprey.sh HIDDEN3 inter inter >> logs/osprey.log 2>/dev/null
#./run_osprey.sh HIDDEN3 rand random >> logs/osprey.log 2>/dev/null
#./run_osprey.sh HIDDEN3 max maximum >> logs/osprey.log 2>/dev/null

# Check unidirectionality on Metal 1 and Metal 2 where applicable
# FIXME
#echo "Done with DRC! Checking unidirectionality..."
#cp $DRC_TEMPLATES/runUniDirCheck.sh $DRC/
#cp $DRC_TEMPLATES/check_aspect_ratio.perl $DRC/
#cd $DRC
#./runUniDirCheck.sh
cd ..

echo "DONE!!!!!"
