#!/bin/bash

#lef2oa -lib TechLib -lef NangateOpenCellLibrary.lef
#verilogAnnotate -refLibs TechLib -verilog s1196_postrouting.v
#verilog2oa -lib DesignLib -refLibs TechLib -view layout -viewType maskLayout -verilog s1196_postrouting.v
#def2oa -lib DesignLib -cell s1196_bench -view layout -def s1196_postrouting.def -refLibs TechLib -techLib TechLib

#lef2oa -lib TechLib -lef NangateOpenCellLibrary.lef
#verilog2oa -lib DesignLib -refLibs TechLib -verilog s1196_postrouting.v
#def2oa -lib DesignLib -def s1196_postrouting.def


lef2oa -lib DesignLib -lef NangateOpenCellLibrary.lef
verilog2oa -lib DesignLib -refLibs DesignLib -verilog s1196_postrouting.v
def2oa -lib DesignLib -def s1196_postrouting.def
