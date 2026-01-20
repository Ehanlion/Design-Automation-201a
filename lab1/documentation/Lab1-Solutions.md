Report for Lab1
Ethan Owen
SID: 905452983

# Part 1: Import

Source the Setup File
`source lab1_setup`

Import the LEF file to create reference library
`lef2oa -lib NangateLib -lef NangateOpenCellLibrary.lef`

Import Verilog netlist to create the design library for NangateLib
`verilog2oa -lib DesignLib -refLibs NangateLib -view layout -viewType maskLayout -verilog s1196_postrouting.v`

Import the DEF file to add physical layout information
`def2oa -lib DesignLib -cell s1196_bench -view layout -def s1196_postrouting.def -refLibs NangateLib`

# Part 2: Fanout

Assumtions here:
- Filtered out Power Nets: `VDD, VSS`
- Filtered out Clock Nets: `blif_clk_net`
- Filtered out Reset Nets: `blif_reset_net`
- Filtered out Tie Nets: `tie1, tie0`
- Filtered out by Net name and by Net type
- Net types were not defined, so check was superficial, but still done
- Fanout defined as sum of all connections on net, including `oaInstTerm` and `oaTerm`

# Part 3: HPWL (half-perimeter wire length)

Assumtions here:
- Filtered out Power Nets: `VDD, VSS`
- Filtered out Clock Nets: `blif_clk_net`
- Filtered out Reset Nets: `blif_reset_net`
- Filtered out Tie Nets: `tie1, tie0`
- Filtered out everything but 2-terminal nets
- Filtered out by Net name and by Net type
- Net types were not defined, so check was superficial, but still done
- Process all metal layers

Methodology:
- Initialize a bounding box to track the shape
- Check over all metal layers 
- Expand box where needed 
- Track the min and max X & Y values for the box
- Compute the HPWL as $(maxX - minX) + (maxY - minY)$

Plotting techniques:
- 2 approaches were used to generate plotting outputs
- First generates HTML files in `plotting/`
- The second roughly prints plots to the console via a header file
- Neither are included in the final code because we are forced to include only a certain subset of files

# Plotting Here

I saved images of the plots after writing this in markdown, later joining them with a PDF merging tool.