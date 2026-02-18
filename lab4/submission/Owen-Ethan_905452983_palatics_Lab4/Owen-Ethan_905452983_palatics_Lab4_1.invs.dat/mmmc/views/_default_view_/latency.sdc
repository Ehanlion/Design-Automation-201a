set_clock_latency -source -early -min   -0.0001 [get_ports {blif_clk_net}] -clock blif_clk_net 
set_clock_latency -source -early -max   -0.0001 [get_ports {blif_clk_net}] -clock blif_clk_net 
set_clock_latency -source -late -min   -0.0001 [get_ports {blif_clk_net}] -clock blif_clk_net 
set_clock_latency -source -late -max   -0.0001 [get_ports {blif_clk_net}] -clock blif_clk_net 
