# Lab 1 Information and Training

Information from a Piazza forum. Discussions between TA/Professor and Students.

## Make Issue:
When running 'make', I get an error with the compiler file path:

Commands: make
/grid/common/pkgs/gcc/v4.1.2p2/bin/g++   -Wno-ctor-dtor-privacy -O -o lab1.o -I/w/class.1/ee/ee201o/ee201ota/oa/include/oa \
 -I/w/class.1/ee/ee201o/ee201ota/oa/include \
 -c lab1.cpp
make: /grid/common/pkgs/gcc/v4.1.2p2/bin/g++: Command not found
make: *** [Makefile:34: lab1.o] Error 127

Solution:
make lab1 CCPATH=g++ (add the CCPATH=g++ part I think)

## Fanout issue:
Hi, when calculating the fanout, should we find the input and output on a net and then calculate or just get the number of connections on a net. If we need to deal with input and output, how could we deal with inputandoutput? Thanks.
hw1

Solution:
Roughly, fanout of a net is the number of its outputs.

## Make isn't working
After completing Part 1 of the lab, I encountered the following error after running "make lab1 CCPATH=g++", indicating that a few variables such as "fanout" are undefined. Is this to be expected? 

Solution:
Yes, this is expected. You need to complete Problems 2 and 3 or comment out the corresponding parts to avoid errors.

## Hint for oaNet:
As per OpenAccess documentation, oaNet is associated directly with oaInstTerm and oaTerm but not with oaInst. 

## Fixes for setup scripts:

TA for course posted this info:
I have added a file lab1_setup in the /w/class.1/ee/ee201o/ee201ot2/2024_labs/lab1 . This contains below three lines which can be added to .bashrc . 

export OA_HOME="/w/class.1/ee/ee201o/ee201ota/oa"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$OA_HOME/lib/linux_rhel40_64/opt"
export PATH="$PATH:$OA_HOME/bin/linux_rhel40_64/opt"

Add these lines and source .bashrc . This should resolve issues with setup (lef2oa etc).

Let me know if the paths are not accessible.

Follow up was this:
I am getting command not found when running below first command 
$ lef2oa -lib foundryLib -lef foundry18u7lm.lef

How can I modify the .bashrc file and where can I find it?

Thank you

(But this could be bogus, I don't trust this response very much)