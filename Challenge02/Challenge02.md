# Challenge 2

## Basic Description
The goal is to be able to generate a basic game of battleship. Your program should be able to create and load a file that describes the battleship positions. The game should hold an arbitray number of players (just for added complexity). The program is required to generate a random board that is valid and the program must be able to take user input to build a board. Displaying the board is secondary but helpful.

## File Format
Similar to challenge 1. A few things are added to help achieve the goals. A player name is now included before each board data. This name is a string and can contain any sequence of characters to a max length of 64 charachters. All the boards in one game are the same size. However game boards can be different sizes like before. 

### Example of file format
<pre>
string: Player name
unsigned integer: number of rows
unsigned integer: number of cols
int16 (short) : grid of data comma seperated with a newline between rows.
string: player name
int16 : same as above
</pre>

### Data (int16)
 - 0 indicates empty cell
 - greater than 0 indicates a ship of some sort. For instance 2 indicates a ship of size 2 is in a cel. That indicates that the next 2 should be adjacent:  above, below, left or right to the existing 2. 
  

## Command line options must support these functions 
- verify <*path to file*> :  load the given filename tells the program that the given file should be checked for errors (and exit)
- create <*path to file*> : create a file 
    - --row <int> : number of rows to create
    - --col <int> : number of cols to create
    - --ships <int : smallest ship size> <int : largest ship size> 
    - --[player *player name* (all the next blocks repeat)
        -   --random : have program generate random ships that do not intersect.  
        -   --place [<<int:shipId>>:<<ColRow:location>>:<<char:orientation>>] ]
- display <*path to file> print the file to screen nicely

- shipId = integer a ship of id 1 is 1 cell large. A ship of id 2 is 2 cells large. The same id cannot be specified more than once.
- ColRow = Excel like col row: B0, B1. Base 26 encoded col with A = 0; Row is integer base 10 encoding.
- Orientation = V for vertical or H for Horizontal. 

## Expected output
### Create mode
- If failure then print "ERROR: <with some error message>". 
- If ships are invalid locations print: "Invalid <shipId> Intersection" or "Invalid <shipId> OffScreen"  
- If no failure than print "DONE: <Your greeting...>" and output the file specified

### Verify mode
- "Passed: <Your message>" if the file is valid.
- "Failed: <Your message>" if the file is invalid.

## Learning Points
 + Random algo and placement
 + Spacial placement 
 + Slightly more complex loading requirements and better internal data structures to keep up with the more specilized requirements. 


