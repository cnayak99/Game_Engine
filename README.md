Lillie Sharpe
Robbie Martin
Chinmay Nayak


***Team 13 HW1***

These are the instructions including the startup process for our base game engine application and the controls for the controllable rectangle in it.
We will begin by explaining the controls because they will be important for Step 6 of the base game engine application startup.


**Controls**

C = Changes the method by which the window can be resized.

Up Arrow Key = Propels the controllable rectangle upwards, similar to a jump. Can be pressed endlessly, resulting in the rectangle continuing to move upwards in a fashion similar to jumping.
Left Arrow Key = Moves the controllable rectangle left at a constant speed.
Right Arrow Key = Moves the controllable rectangle right at a constant speed.

O = Pauses the game.
P = Unpauses the game.

B = Sets the tic to .5x.
N = Sets the tic to 1x.
M = Sets the tic to 2x.


**Startup Process**

Step 1.) Download the .zip file including the resources for the game engine.

Step 2.) Unzip the .zip file in a location that would be easy to access through VSCode.

Step 3.) Access the folder containing the game engine resources through VSCode.

Step 4.) In a VSCode terminal, change directories to the folder containing the game engine resources (in this case, this folder should be "/CSC28_Team13").

Step 5.) Ensure that the zmq library is installed by running the following command in a system terminal: sudo apt install libzmq3-dev

Step 6.) If you would like to access *Section 6* functionality (and where our team has implemented Timelines more correctly so far), leave the first main function in main.cpp uncommented. If you would like to access *Section 2 and 3* functionality, comment the first main function and uncomment the second main function.

Step 7.) Open a second VSCode terminal.

Step 8.) Change directories to the "/server" folder in the folder containing the game engine resources ("/CSC28_Team13/server").

Step 9.) In the VSCode terminal located inside of the "/server" folder, enter the following command to compile the server: g++ -o server Server.cpp -lzmq

Step 10.) Enter the command "./server".

Step 11.) Without closing the terminal running "./server", return to the VSCode terminal that is located in the folder containing the game engine resources ("/CSC28_Team13").

Step 12.) Enter the command "make" in the terminal. The game engine should compile with no errors or warnings.

Step 13.) Enter the command "./game_engine_app" in the terminal. This should start up the base game engine application, but be careful! *If you do not move the controllable rectangle to a non-controllable rectangle soon after you open the game, then the controllable rectangle will accelerate with increasing speed downward off the screen.*

Thank you very much for viewing our game engine!
