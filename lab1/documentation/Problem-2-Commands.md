# Problem 2 Setup Complete

**See `Problem-2-Info.md` for complete code examples!**

**Compile**
```bash
./scripts/problem2-compile.sh
```

**Step 4: Run**
```bash
./lab1
```

**What To Do Next**

1. **Read** `Problem-2-Info.md` for detailed implementation guidance
2. **Edit** `lab1.cpp` to implement the fanout computation (lines 253-293)
3. **Compile** using `./scripts/problem2-compile.sh`
4. **Run** `./lab1` to get results
5. **Export** fanout data and create distribution plot
6. **Update** `Problem-2-Solution.md` with your actual results


**Troubleshooting**

"Command not found" errors
```bash
source lab1_setup
```

Compilation fails with undefined variables
- Complete the Problem 2 code section in lab1.cpp
- OR comment out incomplete sections temporarily

DesignLib not found
```bash
./scripts/problem1-import.sh
```

Need to start over?
```bash
./scripts/problem1-clean.sh   # Remove OA libraries
./scripts/problem2-clean.sh   # Remove binaries
./scripts/problem1-import.sh  # Reimport design
```
