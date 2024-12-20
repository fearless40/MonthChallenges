# First Challenge

## Basic Description
Inital challenge project is to load an array of integers from a file and be able to answer basic questions about the data. The data can be malformed and the program should exit gracefully rather than crashing. Make sure that you include a readme.md file that indicates how the progam can be built.  

### Description of file format
1. Line 1: number or rows unsigned 16bit integer (0-65535)
2. Line 2: number of columns unsigned 16bit integer 0-65535)
3. Line 3-...: signed 16bit integers (-32768 to 32767) seperated by "," for as many rows and columns as specified above

### Example file
<pre>
 2
 2
 0,1
 2,3
</pre>

## User interaction
The program loads the file and if not given the command line option of --guess will become interactive. The program will ask the user what the values they would like to look up.  Guesses are in excel *ColRow* format with *col* represented by letters and *rows* by numbers. _A1_ is the first col and row. *C10* is the 3rd col and 10th row. Above 26 columns the *col* numbering becomes _AA_, _AB_, _AC_ ... 
An alternative format is col,row with each represented by numbers. 1,1 is equivalent to B1.

### Expected Output 
<pre>
 User      >> challenge1.exe --load example.txt 
 Program   >> "Loaded file please ask the values at a location (quit) to quit": 
 User      >> A1 
 Program   >> 0 
 Program   >> "Please enter a new value to check (quit to quit): 
 User      >> B2
 Program   >> 3 
 Program   >> "Please enter a new value to check (quit to quit): 
 User      >> quit
</pre>

 ## Supported command line options
- *--load* : read the given file
- *--guess*: given the column-row with spaces between each entry (A1 B1 C1 AA340) the guess can be capitalized or lowercase for the letters. If *--guess* is not included run in interactive mode. 
 > challenge --load _filename_  --guess _A1_ _B2_ _C4_ 
 > Expected output: 0, 3, OOB   
 > OOB = Out of bounds
 > If *--guess* is not included run the interacive mode

 ## Error Conditions
 The files can be malformed. If the program encourters an error in file parsing or in user input the program should ouput ERROR: "Whatever error you want to write". For instance if the file has a row amount of 9999999 the program should output _ERROR: row too large_. Or if the file has a column missing in a row the program should output: _ERROR: missing column on line ..._  
 
 ## Learning Points
 + Read input from the command line
 + Read input from a file
 + Interactive vs non interactive mode
 + Deal with errors from the user or in the file without crashing
 + Learning about tests (the tests are part of the challenge) The challenge will include a program to test your program.
 + Dynamic memory allocation 
    
 

 

