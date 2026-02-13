# Rule: Outputs and Formatting

Outputs must only ever use ASCII characters.

Follow all outputs in the lab3.cpp file as follows:

This is the output format for the part 1 and part 2 outputs
```cpp
cout << endl
			 << "----- Firstname Lastname: Problem 1 -----" << endl;

		// EE 201A Lab 3 Problem 2 starts here
		cout << endl
			 << "----- Firstname Lastname: Problem 2 -----" << endl;
```

This is the output format for the summary at the end:
```cpp
cout << "Problem 1 -- Total wirelength of original design: " << "*YOUR VALUE HERE*" << endl;
		cout << "Problem 2 -- Total wirelength AFTER my incremental placement algorithm:  " << "*YOUR VALUE HERE*" << endl;
		cout << "Problem 2 -- Total number of swaps used:  " << "*YOUR VALUE HERE*" << endl;
		cout << "Problem 2 -- Time taken:  " << fixed
			 << time_taken << setprecision(6);
		cout << " sec" << endl;
```

These outputs are required for the assignment to be graded and must be included in this format for every attempt at the problem please.

If you need to add additional outputs, feel free to add them, BUT add them into the section where they belong.

## Outputs I always want to see:

For part 1, I want to see at least the following:
(nested in the part 1 output section)
```cpp
cout << endl
			 << "----- Firstname Lastname: Problem 1 -----" << endl;
cout << "Problem 1 -- Nets counted in total HPWL: " << netsCounted
			 << " (skipped " << skippedNets << ")" << endl;
		cout << "Problem 1 -- Total HPWL (DBU): " << fixed << setprecision(0)
			 << totalHPWL << endl;
```

For part 2, I NEED to see an additional output here for the Originalk HPWL, the reduced HPWL, the time that was taken, adn teh score, which is calcualted as $HPWL^2 \text{times} time$, where time is the execution of the placement algorithm. 
```cpp
cout << endl
			 << "----- Firstname Lastname: Problem 2 -----" << endl;
             // add the info here beneath 
```