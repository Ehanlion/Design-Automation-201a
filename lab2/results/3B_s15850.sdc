# ####################################################################

#  Created by Genus(TM) Synthesis Solution 21.19-s055_1 on Mon Feb 02 20:38:40 PST 2026

# ####################################################################

set sdc_version 2.0

set_units -capacitance 1fF
set_units -time 1000ps

# Set the current design
current_design s15850_bench

create_clock -name "blif_clk_net" -period 1.0 -waveform {0.0 0.5} [get_ports blif_clk_net]
set_clock_transition 0.1 [get_clocks blif_clk_net]
group_path -weight 1.000000 -name cg_enable_group_blif_clk_net -through [list \
  [get_pins RC_CG_HIER_INST0/enable]  \
  [get_pins RC_CG_HIER_INST0/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST0/enable]  \
  [get_pins RC_CG_HIER_INST0/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST0/enable]  \
  [get_pins RC_CG_HIER_INST0/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST0/enable]  \
  [get_pins RC_CG_HIER_INST0/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST1/enable]  \
  [get_pins RC_CG_HIER_INST1/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST2/enable]  \
  [get_pins RC_CG_HIER_INST2/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST0/enable]  \
  [get_pins RC_CG_HIER_INST0/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST1/enable]  \
  [get_pins RC_CG_HIER_INST1/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST2/enable]  \
  [get_pins RC_CG_HIER_INST2/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST0/enable]  \
  [get_pins RC_CG_HIER_INST0/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST1/enable]  \
  [get_pins RC_CG_HIER_INST1/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST2/enable]  \
  [get_pins RC_CG_HIER_INST2/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST0/enable]  \
  [get_pins RC_CG_HIER_INST0/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST1/enable]  \
  [get_pins RC_CG_HIER_INST1/RC_CGIC_INST/E]  \
  [get_pins RC_CG_HIER_INST2/enable]  \
  [get_pins RC_CG_HIER_INST2/RC_CGIC_INST/E] ]
set_clock_gating_check -setup 0.0 
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports blif_reset_net]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g18]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g27]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g109]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g741]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g742]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g743]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g744]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g872]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g873]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g877]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g881]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g1712]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g1960]
set_input_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g1961]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2355]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2601]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2602]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2603]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2604]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2605]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2606]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2607]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2608]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2609]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2610]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2611]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2612]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2648]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g2986]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g3007]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g3069]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g4172]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g4173]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g4174]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g4175]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g4176]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g4177]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g4178]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g4179]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g4180]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g4181]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g4887]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g4888]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g5101]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g5105]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g5658]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g5659]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g5816]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g6920]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g6926]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g6932]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g6942]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g6949]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g6955]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g7744]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8061]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8062]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8271]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8313]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8316]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8318]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8323]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8328]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8331]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8335]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8340]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8347]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8349]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8352]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8561]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8562]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8563]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8564]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8565]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8566]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8976]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8977]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8978]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8979]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8980]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8981]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8982]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8983]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8984]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8985]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g8986]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g9451]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g9961]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g10377]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g10379]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g10455]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g10457]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g10459]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g10461]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g10463]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g10465]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g10628]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g10801]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g11163]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g11206]
set_output_delay -clock [get_clocks blif_clk_net] -add_delay 0.0 [get_ports g11489]
set_max_leakage_power 0.0
set_max_dynamic_power 0.0
set_wire_load_mode "enclosed"
