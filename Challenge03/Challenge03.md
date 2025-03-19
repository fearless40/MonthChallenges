# Challenge 3
The goal of this challenge is to write an "ai" player to play against. The new program is simpler in requirements and the program will be required to read from the input and output a guess. The test application will check the user program for errors and record statistics about how good the algorithm is. The time take per move and the number of guesses. 


## Command line interface
- "--ai [number]" where the number is from 0 to 3. 0 is the most basic ai, 1 is the middle of the road, 2 is next best, 3 is the best. You do not need to implement them all. Just implement at least one.
- "--aicount" Return the number of AI's your application supports. Then exit. No other output allowed. 
- "--rows [number]" max number of rows. if not specified your program should default to 10
- "--cols [number]" max number of cols. if not specified your program should default to 10
- "--ships [startsize] [endsize]" startsize is the smallest ship possible and endsize is the largest ship possible. startsize default is 2, endsize default is 5

## Expected input and output
- Your program will write to the standard output stream (cout in c++, puts in c, whatever other language you choose to use) with its guess. The format of the guess is excel style base 26 guess. However A = 0. So B = 1. C = 2. For example Row 0, Col 0, is A0. Row 1, Col 1 is B1.
- In return the testing program will write the answer as follows "StatusCode:optional ShipID?"
- The number of characters returned will always be the largest amount of digits to represent ship size plus 1 for the status. 

### Status codes:
1. M = miss
2. H = hit
3. S = sink the ship: followed by :ID of ship
4. E = end current game. Start new game.
5. Q = quit


### Example:
`
AI: B0
Tester: M 
AI: C0
Tester: H
AI: C1
Tester: S2
`
The tester will run with multiple iterations to determine stats. Do not print anything to the screen when running. The test will take over the screen. 


