#include "uart.h"
#include "gpio.h"
#include "systimer.h"
#include "mailbox.h"

// HTML RGB color codes.  These can be found at:
// https://htmlcolorcodes.com/
#define BLACK     0x00000000
#define WHITE     0x00FFFFFF
#define RED       0x00FF0000
#define LIME      0x0000FF00
#define BLUE      0x000000FF
#define AQUA      0x0000FFFF
#define FUCHSIA   0x00FF00FF
#define YELLOW    0x00FFFF00
#define GRAY      0x00808080
#define MAROON    0x00800000
#define OLIVE     0x00808000
#define GREEN     0x00008000
#define TEAL      0x00008080
#define NAVY      0x00000080
#define PURPLE    0x00800080
#define SILVER    0x00C0C0C0

// Frame buffer constants
#define FRAMEBUFFER_WIDTH      1024  // in pixels
#define FRAMEBUFFER_HEIGHT     768   // in pixels
#define FRAMEBUFFER_DEPTH      32    // bits per pixel (4 bytes per pixel)
#define FRAMEBUFFER_ALIGNMENT  4     // framebuffer address preferred alignment
#define VIRTUAL_X_OFFSET       0
#define VIRTUAL_Y_OFFSET       0
#define PIXEL_ORDER_BGR        0     // needed for the above color codes

// bit checker
#define CHKBIT(var,pos) (((var)>>(pos)) & 1)

unsigned short get_SNES();
void init_GPIO9_to_output();
void init_GPIO11_to_output();
void set_GPIO(int reg);
void clear_GPIO(int reg);
void init_GPIO10_to_input();
unsigned int get_GPIO(int reg);
void initSNES();
void initFrameBuffer();
void displayFrameBuffer();
void displayMazeInit();
void updateMaze(int X, int Y);

unsigned short data;
unsigned short currentState;
int pgt;

int PlayerX, PlayerY, OldplayerX, OldplayerY, PlayerState, PlayerStartX, PlayerStartY, BtnPressed;

// Frame buffer global variables
unsigned int frameBufferWidth, frameBufferHeight, frameBufferPitch;
unsigned int frameBufferDepth, frameBufferPixelOrder, frameBufferSize;
unsigned int *frameBuffer;

int maze[12][16] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {2, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 3},
    {1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

void main()
{
  // set up initial controller and player states
  data = 0xFFFF;
  currentState = 0xFFFF;
  // set up player state: 0 if has not won, 1 if has won, 99 if game has not started yet
  PlayerState = 99;
  pgt = 0;
  BtnPressed = 0;

// set up initial player coordinates
for(int i = 0; i < 12; i++){
  for(int j = 0; j < 16; j++){
    if(maze[i][j] == 2){
      PlayerX = i;
      PlayerStartX = i;
      PlayerY = j;
      PlayerStartY = j;
    }
  }
}
    // initialization functions for UART link, frame buffer, and SNES controller
    uart_init();
    initFrameBuffer();
    initSNES();
    displayMazeInit();

    // Loop forever, reading from the SNES controller 30 times per second
    while (1) {
      // Draw on the frame buffer and display it
      displayFrameBuffer();
      // Read data from the SNES controller
      data = get_SNES();

  // Write out data if the state of the controller has changed
  if (data != currentState) {
      // Write the data out to the console in hexadecimal
      //uart_puts("0x");
      //uart_puthex(data);
      //uart_puts("\n");

      // Record the state of the controller
      currentState = data;
  }
  // check if in game. If not, check if start button has been pressed.
  // If start button has been pressed, start the game.
  if(PlayerState != 0){
    int startButton = CHKBIT(currentState, 3);
    if(startButton == 1){
      uart_puts("Game started!");
      PlayerState = 0;
      updateMaze(PlayerX, PlayerY);
      PlayerX = PlayerStartX;
      PlayerY = PlayerStartY;
    }
  }
  if(PlayerState != 99){
    // game logic here, for when player state equals 0 or 1.
    // get gamepad data
    int gamepadUp = CHKBIT(currentState, 4);
    int gamepadDown = CHKBIT(currentState, 5);
    int gamepadLeft = CHKBIT(currentState, 6);
    int gamepadRight = CHKBIT(currentState, 7);

    // check if gamepad has been pressed in a direction
    if(gamepadUp == 1){
      //uart_puts("up");
      // "place going to" integer
      pgt = maze[PlayerX - 1][PlayerY];
      // make sure where we're going is not a wall
      if(pgt != 1 && BtnPressed == 0){
          updateMaze(PlayerX, PlayerY);
          OldplayerX = PlayerX;
          // the delay fixes a ghosting bug
          microsecond_delay(10);
          PlayerX--;
      }else{
        uart_puts("Direction entered is illegal...");
      }
      // update button press integer
      BtnPressed = 1;
    }else if (gamepadDown == 1){
      //uart_puts("down");
      // "place going to" integer
      pgt = maze[PlayerX + 1][PlayerY];
      // make sure where we're going is not a wall
      if(pgt != 1 && BtnPressed == 0){
        updateMaze(PlayerX, PlayerY);
          OldplayerX = PlayerX;
          // the delay fixes a ghosting bug
          microsecond_delay(10);
          PlayerX++;
      }else{
        uart_puts("Direction entered is illegal...");
      }
      // update button press integer
      BtnPressed = 1;
    }else if(gamepadLeft == 1){
      //uart_puts("left");
      if(PlayerY != 0){
        // "place going to" integer
      pgt = maze[PlayerX][PlayerY - 1];
    }else{
      pgt = PlayerY;
    }
    // make sure where we're going is not a wall
    if(pgt != 1 && BtnPressed == 0){
      updateMaze(PlayerX, PlayerY);
      OldplayerY = PlayerY;
      // the delay fixes a ghosting bug
      microsecond_delay(10);
      PlayerY--;
    }else{
      uart_puts("Direction entered is illegal...");
    }
    // update button press integer
    BtnPressed = 1;
    }else if(gamepadRight == 1){
      //uart_puts("right");
      // "place going to" integer
      pgt = maze[PlayerX][PlayerY + 1];
      // make sure where we're going is not a wall
      if(pgt != 1  && BtnPressed == 0 && PlayerY != 15){
        updateMaze(PlayerX, PlayerY);
          OldplayerY = PlayerY;
          // the delay fixes a ghosting bug
          microsecond_delay(10);
          PlayerY++;
      }else{
        uart_puts("Direction entered is illegal...");
      }
      // update button press integer
      BtnPressed = 1;
    }else{
      // reset button press int
      BtnPressed = 0;
      //uart_puts("Dpad reset.");
    }
    if(maze[PlayerX][PlayerY] == 3){
      PlayerState = 1;
    }

    // bounding checks
    if(PlayerX < 0){
      PlayerX = 0;
    }
    if(PlayerY < 0){
      PlayerY = 0;
    }
  }

      // Delay 1/30th of a second
      microsecond_delay(33333);
    }
}

void initSNES()
{
    // Set up GPIO pin #9 for output (LATCH output)
    init_GPIO9_to_output();

    // Set up GPIO pin #11 for output (CLOCK output)
    init_GPIO11_to_output();

    // Set up GPIO pin #10 for input (DATA input)
    init_GPIO10_to_input();

    // Clear the LATCH line (GPIO 9) to low
    clear_GPIO(9);

    // Set CLOCK line (GPIO 11) to high
    set_GPIO(11);


    // Print out a message to the console
    uart_puts("SNES Controller Program starting.\n");
}

unsigned short get_SNES()
{
    int i;
    unsigned short data = 0;
    unsigned int value;


    // Set LATCH to high for 12 microseconds. This causes the controller to
    // latch the values of button presses into its internal register. The
    // first serial bit also becomes available on the DATA line.
    set_GPIO(9);
    microsecond_delay(12);
    clear_GPIO(9);

    // Output 16 clock pulses, and read 16 bits of serial data
    for (i = 0; i < 16; i++) {
	// Delay 6 microseconds (half a cycle)
	microsecond_delay(6);

	// Clear the CLOCK line (creates a falling edge)
	clear_GPIO(11);

	// Read the value on the input DATA line
	value = get_GPIO(10);

	// Store the bit read. Note we convert a 0 (which indicates a button
	// press) to a 1 in the returned 16-bit integer. Unpressed buttons
	// will be encoded as a 0.
	if (value == 0) {
	    data |= (0x1 << i);
	}

	// Delay 6 microseconds (half a cycle)
	microsecond_delay(6);

	// Set the CLOCK to 1 (creates a rising edge). This causes the
	// controller to output the next bit, which we read half a
	// cycle later.
	set_GPIO(11);
    }

    // Return the encoded data
    return data;
}

// Okay, i'm getting tired of commenting, so here we go:
//    This function sets GPIO pin 9 to output, via setting FSEL9 to 000,
//    then setting it to 001, then writing that to GPFSEL0, and then
//    updating the clock and control lines to reflect the fact it is a
//    clocked output line. Yay! This is all shown in the broadcom manual,
//    starting around page 101
void init_GPIO9_to_output()
{
    register unsigned int r;
    r = *GPFSEL0;
    r &= ~(0x7 << 27);
    r |= (0x1 << 27);
    *GPFSEL0 = r;
    *GPPUD = 0x0;
    microsecond_delay(1);
    *GPPUDCLK0 = (0x1 << 9);
    microsecond_delay(1);
    *GPPUDCLK0 = 0;
}

// Okay, i'm getting tired of copy/pasting comments, so here we go:
//    This function sets GPIO pin 11 to output, via setting FSEL11 to 000,
//    then setting it to 001, then writing that to GPFSEL1, and then
//    updating the clock and control lines to reflect the fact it is a
//    clocked output line. Yay!
void init_GPIO11_to_output()
{
    register unsigned int r;
    r = *GPFSEL1;
    r &= ~(0x7 << 3);
    r |= (0x1 << 3);
    *GPFSEL1 = r;
    *GPPUD = 0x0;
    microsecond_delay(1);
    *GPPUDCLK0 = (0x1 << 11);
    microsecond_delay(1);
    *GPPUDCLK0 = 0;
}

void set_GPIO(int reg)
{
	  register unsigned int r;
	  // Put a 1 into the SETxx field of the GPIO Pin Output Set Register 0
	  r = (0x1 << reg);
	  *GPSET0 = r;
}

void clear_GPIO(int reg)
{
	  register unsigned int r;
	  // Put a 1 into the CLRxx field of the GPIO Pin Output Clear Register 0
	  r = (0x1 << reg);
	  *GPCLR0 = r;
}

// GPIO status getter
unsigned int get_GPIO(int reg)
{
    register unsigned int r;
    r = *GPLEV0;
    return ((r >> reg) & 0x1);
}

void init_GPIO10_to_input()
{
    register unsigned int r;
    r = *GPFSEL1;
    // Clear the field FSEL10
    r &= ~(0x7 << 0);
    // set as input
    *GPFSEL1 = r;
    //disable pullup/pulldown
    *GPPUD = 0x0;
    // slightly longer than looping nop 300 times
    microsecond_delay(1);
    // write to pullup/pulldown
    *GPPUDCLK0 = (0x1 << 10);
    // slightly longer than looping nop 300 times
    microsecond_delay(1);
    //clear pullup/pulldown
    *GPPUDCLK0 = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function:       initFrameBuffer
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function uses the mailbox request/response protocol
//                  to allocate and set the frame buffer. This includes the
//                  width, height, and depth of the framebuffer, plus the
//                  desired pixel order (BGR). The mailbox response is used
//                  to set the frame buffer global variables that can be used
//                  later on when drawing to the screen. The most important of
//                  these is the frame buffer address.
//
////////////////////////////////////////////////////////////////////////////////

void initFrameBuffer()
{
    // Initialize the mailbox data structure.
    // It contains a series of tags that specify the
    // desired settings for the frame buffer.
    mailbox_buffer[0] = 35 * 4;
    mailbox_buffer[1] = MAILBOX_REQUEST;

    mailbox_buffer[2] = TAG_SET_PHYSICAL_WIDTH_HEIGHT;
    mailbox_buffer[3] = 8;
    mailbox_buffer[4] = 0;
    mailbox_buffer[5] = FRAMEBUFFER_WIDTH;
    mailbox_buffer[6] = FRAMEBUFFER_HEIGHT;

    mailbox_buffer[7] = TAG_SET_VIRTUAL_WIDTH_HEIGHT;
    mailbox_buffer[8] = 8;
    mailbox_buffer[9] = 0;
    mailbox_buffer[10] = FRAMEBUFFER_WIDTH;
    mailbox_buffer[11] = FRAMEBUFFER_HEIGHT;

    mailbox_buffer[12] = TAG_SET_VIRTUAL_OFFSET;
    mailbox_buffer[13] = 8;
    mailbox_buffer[14] = 0;
    mailbox_buffer[15] = VIRTUAL_X_OFFSET;
    mailbox_buffer[16] = VIRTUAL_Y_OFFSET;

    mailbox_buffer[17] = TAG_SET_DEPTH;
    mailbox_buffer[18] = 4;
    mailbox_buffer[19] = 0;
    mailbox_buffer[20] = FRAMEBUFFER_DEPTH;

    mailbox_buffer[21] = TAG_SET_PIXEL_ORDER;
    mailbox_buffer[22] = 4;
    mailbox_buffer[23] = 0;
    mailbox_buffer[24] = PIXEL_ORDER_BGR;

    mailbox_buffer[25] = TAG_ALLOCATE_BUFFER;
    mailbox_buffer[26] = 8;
    mailbox_buffer[27] = 0;
    // Request: alignment; Response: frame buffer address
    mailbox_buffer[28] = FRAMEBUFFER_ALIGNMENT;
    mailbox_buffer[29] = 0;    // Response: Frame buffer size

    mailbox_buffer[30] = TAG_GET_PITCH;
    mailbox_buffer[31] = 4;
    mailbox_buffer[32] = 0;
    mailbox_buffer[33] = 0;    // Response: Pitch

    mailbox_buffer[34] = TAG_LAST;


    // Make a mailbox request using the above mailbox data structure
    if (mailbox_query(CHANNEL_PROPERTY_TAGS_ARMTOVC)) {
	// If here, the query succeeded, and we can check the response

	// Get the returned frame buffer address, masking out 2 upper bits
        mailbox_buffer[28] &= 0x3FFFFFFF;
        frameBuffer = (void *)((unsigned long)mailbox_buffer[28]);

	// Read the frame buffer settings from the mailbox buffer
        frameBufferWidth = mailbox_buffer[5];
        frameBufferHeight = mailbox_buffer[6];
        frameBufferPitch = mailbox_buffer[33];
	frameBufferDepth = mailbox_buffer[20];
	frameBufferPixelOrder = mailbox_buffer[24];
	frameBufferSize = mailbox_buffer[29];

	// Display frame buffer settings to the terminal
	uart_puts("Frame buffer settings:\n");

	uart_puts("    width:       0x");
	uart_puthex(frameBufferWidth);
	uart_puts(" pixels\n");

	uart_puts("    height:      0x");
	uart_puthex(frameBufferHeight);
	uart_puts(" pixels\n");

	uart_puts("    pitch:       0x");
	uart_puthex(frameBufferPitch);
	uart_puts(" bytes per row\n");

	uart_puts("    depth:       0x");
	uart_puthex(frameBufferDepth);
	uart_puts(" bits per pixel\n");

	uart_puts("    pixel order: 0x");
	uart_puthex(frameBufferPixelOrder);
	uart_puts(" (0=BGR, 1=RGB)\n");

	uart_puts("    address:     0x");
	uart_puthex(mailbox_buffer[28]);
	uart_puts("\n");

	uart_puts("    size:        0x");
	uart_puthex(frameBufferSize);
	uart_puts(" bytes\n");

    } else {
        uart_puts("Cannot initialize frame buffer\n");
    }
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function:       drawSquare
//
//  Arguments:      rowStart:        Top left pixel y coordinate
//                  columnStart:     Top left pixel x coordinate
//                  squareSize:      Square size in pixels per side
//                  color:           RGB color code
//
//  Returns:        void
//
//  Description:    This function function draws a single square into the
//                  frame buffer. The top left pixel of the square is given,
//                  and it is drawn downwards and to the right on the display.
//                  The size of the square is given in terms of pixels per side,
//                  and the pixels in the square are given the same specified
//                  color.
//
////////////////////////////////////////////////////////////////////////////////

void drawSquare(int rowStart, int columnStart, int squareSize, unsigned int color)
{
    int row, column, rowEnd, columnEnd;
    unsigned int *pixel = frameBuffer;


    // Calculate where the row and columns end
    rowEnd = rowStart + squareSize;
    columnEnd = columnStart + squareSize;

    // Draw the square row by row, from the top down
    for (row = rowStart; row < rowEnd; row++) {
	// Draw each pixel in the row from left to right
        for (column = columnStart; column < columnEnd; column++) {
	    // Draw the individual pixel by setting its
	    // RGB value in the frame buffer
            pixel[(row * frameBufferWidth) + column] = color;
        }
    }
}

// Maze array mapping:
// 1: Wall, WHITE
// 2: Entrance, GRAY
// 0: Hallway, GRAY
// 3: Exit, GRAY
// Default: FUCHSIA
/////////////////////////////
// Player color mapping:
// 0: RED
// 1: GREEN
// 99: GRAY    // aka hide player
// Default: FUCHSIA
void updateMaze(int X, int Y){
  int squareSize;
  // set square size to 64 pixels
  squareSize = 64;

  // set up color integer
  unsigned int color;

      // using the maze matrix, choose a color for this square
      switch (maze[X][Y]) {
        case 0:
          color = WHITE;
          break;
        case 1:
          color = GRAY;
          break;
        case 2:
          color = WHITE;
          break;
        case 3:
          color = WHITE;
          break;
        default:
          color = FUCHSIA;
          break;
      }
  // draw the square with the chosen color from the maze matrix
  drawSquare(X * squareSize, Y * squareSize, squareSize, color);
}

// Maze array mapping:
// 1: Wall, WHITE
// 2: Entrance, GRAY
// 0: Hallway, GRAY
// 3: Exit, GRAY
// Default: FUCHSIA
/////////////////////////////
// Player color mapping:
// 0: RED
// 1: GREEN
// 99: GRAY    // aka hide player
// Default: FUCHSIA
void displayMazeInit(){
  int squareSize, numberOfRows, numberOfColumns;
  // set square size to 64 pixels
  squareSize = 64;
  // set size of player square
  numberOfRows = 12;
  // set column count to 16.
  numberOfColumns = 16;

  // set up for loop ints
  int i, j;
  // set up color integer
  unsigned int color;

  // Draw the rows from the top down
  for (i = 0; i < numberOfRows; i++) {
    for (j = 0; j < numberOfColumns; j ++) {
      // using the maze matrix, choose a color for this square
      switch (maze[i][j]) {
        case 0:
          color = WHITE;
          break;
        case 1:
          color = GRAY;
          break;
        case 2:
          color = WHITE;
          break;
        case 3:
          color = WHITE;
          break;
        default:
          color = FUCHSIA;
          break;
      }
  // draw the square with the chosen color from the maze matrix
  drawSquare(i * squareSize, j * squareSize, squareSize, color);
    }
}
}

// Maze array mapping:
// 1: Wall, WHITE
// 2: Entrance, GRAY
// 0: Hallway, GRAY
// 3: Exit, GRAY
// Default: FUCHSIA
/////////////////////////////
// Player color mapping:
// 0: RED
// 1: GREEN
// 99: GRAY    // aka hide player
// Default: FUCHSIA
void displayFrameBuffer()
{
  int squareSize, playerSize, playerOffset;
  // set square size to 64 pixels
  squareSize = 64;
  // set player square size
  playerSize = 32;
  // set player offset to center the smaller square
  playerOffset = 16;
  // set up color integer
  unsigned int color;

  // using the passed value, choose player color
  switch (PlayerState) {
    case 0:
      color = RED;
      break;
    case 1:
      color = GREEN;
      break;
    case 99:
      color = WHITE;
      break;
    default:
      color = FUCHSIA;
      break;
  }
  // draw player square
  drawSquare((PlayerX * squareSize) + playerOffset, (PlayerY * squareSize) + playerOffset, playerSize, color);
  // draw over old player square
  // using the maze matrix, choose a color for this square
  switch (maze[OldplayerX][OldplayerY]) {
    case 0:
      color = WHITE;
      break;
    case 1:
      color = GRAY;
      break;
    case 2:
      color = WHITE;
      break;
    case 3:
      color = WHITE;
      break;
    default:
      color = FUCHSIA;
      break;
  }
  drawSquare(OldplayerX * squareSize, OldplayerY * squareSize, squareSize, color);
}
