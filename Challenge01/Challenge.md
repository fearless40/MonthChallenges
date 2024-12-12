# First Challenge

## Basics
Inital code project is to load an array of integers from a file. The integers are arranged in rows and columns. 

### Description of file format

On the first line is the number or rows (single integer value)

On the 2nd line is the number of columns (single integer value)

The following lines are the data seperate by ","


### Example file

2

2

0,1

2,3


## User interaction
The program must load the file and then ask the user what the values they would like to look up.  Guess are in excel format with col represented by letters and rows by numbers. _A1_ is the first col and row. *C10* is the 3rd col and 10th row. For columns once 26 columns are surpassed the numbering becomes _AA_, _AB_, _AC_ ... 

### Example 

challenge1.exe --load example.txt 

 Program   >> "Loaded file please ask the values at a location (quit) to quit": 
 
 User      >> A1 
 
 Program   >> 0 
 
 Program   >> "Please enter a new value to check (quit to quit): 
 
 User      >> B2
 
 Program   >> 3 
 
 Program   >> "Please enter a new value to check (quit to quit): 
 
 User      >> quit


 ## Supported command line options
 
 --load _filename_  --guess _A1_ _B2_ _C4_ 
 
 Expected output: 0, 3, OOB   
 
 > OOB = Out of bounds

 If --guess is not included run the interacive mode 
 

 

