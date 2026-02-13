# Lab 3: Scripting rule

Every cpp file requires a script to run it.

Scripts MUST be located in the scripts/ directory

A good script does the following:
- cleans up all compiled files, this includes the generated DesignLib and NangateLib directories as well as object filesa nd cmopiled files such as for example Lab3.cpp would have Lab3.o and Lab3, both of which need to be cleaned
- compiled hte code, including rebuilding DesignLib and NangateLib, and compiling the cpp file
- run the code, execute the code that was created.

Good file names for this:
- I ahve formatting like lab3-part2-test2-version.sh as a script
- I prefer camel case 
- I prefer VERY short names

To run Lab3.cpp, have a script called runLab3.sh

To run Lab3_greedy.cpp (named for the part 2 approach being a greedyt approach), have a script called runLab3Greedy.sh

And so on, you get the idea. 