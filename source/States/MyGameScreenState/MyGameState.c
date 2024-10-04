/**
 * VUEngine Barebone
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MyGameState.h>

#include <Camera.h>
#include <CameraEffectManager.h>
#include <I18n.h>
#include <Languages.h>
#include <Printing.h>
#include <VUEngine.h>
#include <VIPManager.h>
#include <string.h>

extern StageROMSpec MyGameStageSpec;
extern const uint32 AlignmentCheckButtonSequence[__PLUGIN_ALIGNMENT_CHECK_BUTTON_SEQUENCE_LENGTH];
char board[3][3];
char buffer[20];
const char* dashes=" -----";
char winner, turn, turns;
int cursorRow = 0;
int cursorCol = 0;

void MyGameState::resetBoard()
{
	winner=0;
	turn='X';
	turns=0;
	for(int i=0; i<3; ++i)
		for(int j=0; j<3; ++j)
			board[i][j]=' ';
}

// class's constructor
void MyGameState::constructor()
{
	Base::constructor();

	MyGameState::resetLastInputs(this);
}

// class's destructor
void MyGameState::destructor()
{
	// destroy base
	Base::destructor();
}

// state's enter
void MyGameState::enter(void* owner __attribute__ ((unused)))
{
	// call base
	Base::enter(this, owner);

	// reset last button inputs
	MyGameState::resetLastInputs(this);

	// load stage
	GameState::loadStage(GameState::safeCast(this), (StageSpec*)&MyGameStageSpec, NULL, true, false);

	// start clocks to start animations
	GameState::startClocks(GameState::safeCast(this));
	MyGameState::resetBoard(this);
	// print text
	MyGameState::print(this);

	// enable user input
	VUEngine::enableKeypad(VUEngine::getInstance());

	// start fade in effect
	Camera::startEffect(Camera::getInstance(), kHide);
	Camera::startEffect(Camera::getInstance(),
		kFadeTo, // effect type
		0, // initial delay (in ms)
		NULL, // target brightness
		__FADE_DELAY, // delay between fading steps (in ms)
		NULL, // callback function
		NULL // callback scope
	);
}

void MyGameState::suspend(void* owner)
{
	if(!VUEngine::isInToolState(VUEngine::getInstance()))
	{
		// do a fade out effect
		Camera::startEffect(Camera::getInstance(), kFadeOut, __FADE_DELAY);
	}

	// call base
	Base::suspend(this, owner);
}

void MyGameState::resume(void* owner)
{
	// call base
	Base::resume(this, owner);

	// reset last button inputs
	MyGameState::resetLastInputs(this);

	// print text
	MyGameState::print(this);

	// enable user input
	VUEngine::enableKeypad(VUEngine::getInstance());

	if(!VUEngine::isExitingToolState(VUEngine::getInstance()))
	{
		// start a fade in effect
		Camera::startEffect(Camera::getInstance(), kHide);
		Camera::startEffect(Camera::getInstance(),
			kFadeTo, // effect type
			0, // initial delay (in ms)
			NULL, // target brightness
			__FADE_DELAY, // delay between fading steps (in ms)
			NULL, // callback function
			NULL // callback scope
		);
	}
}

void MyGameState::processUserInput(const UserInput* userInput)
{
    if (userInput->pressedKey & ~K_PWR)
    {
        MyGameState::recordLastInput(this, userInput);
        MyGameState::matchButtonCode(this);
    }
	char moved=0;
    // Handle D-pad input to move cursor
    if (userInput->pressedKey & K_LL)
    {
        cursorCol = (cursorCol > 0) ? cursorCol - 1 : 2; // Move left
		moved=1;
    }
    else if (userInput->pressedKey & K_LR)
    {
        cursorCol = (cursorCol < 2) ? cursorCol + 1 : 0; // Move right
		moved=1;
    }
    if (userInput->pressedKey & K_LU)
    {
        cursorRow = (cursorRow > 0) ? cursorRow - 1 : 2; // Move up
		moved=1;
    }
    else if (userInput->pressedKey & K_LD)
    {
        cursorRow = (cursorRow < 2) ? cursorRow + 1 : 0; // Move down
		moved=1;
    }
	if ( userInput->pressedKey & K_A && !winner && board[cursorRow][cursorCol]==' ')
	{
		board[cursorRow][cursorCol]=turn;
		moved=1;
		if((board[0][cursorCol]==turn&&board[1][cursorCol]==turn&&board[2][cursorCol]==turn)||
			(board[cursorRow][0]==turn&&board[cursorRow][1]==turn&&board[cursorRow][2]==turn)||
			(board[1][1]==turn&&((board[0][0]==turn&&board[2][2]==turn)||(board[0][2]==turn&&board[2][0]==turn))))
			winner=turn;
		else if(++turns==9)
			winner='T';
		else
			turn=turn=='X'?'O':'X';
	}
	if( userInput->pressedKey & K_B && winner)
	{
		MyGameState::resetBoard(this);
		moved=1;
	}
	if(moved)
		MyGameState::print(this);
}

void MyGameState::resetLastInputs()
{
	for(uint8 i = 0; i < __PLUGIN_ALIGNMENT_CHECK_BUTTON_SEQUENCE_LENGTH; i++)
	{
		this->lastInputs[i] = 0;
	}
}

void MyGameState::recordLastInput(const UserInput* userInput)
{
	for(uint8 i = 0; i < (__PLUGIN_ALIGNMENT_CHECK_BUTTON_SEQUENCE_LENGTH - 1); i++)
	{
		this->lastInputs[i] = this->lastInputs[i + 1];
	}
	this->lastInputs[__PLUGIN_ALIGNMENT_CHECK_BUTTON_SEQUENCE_LENGTH - 1] = userInput->pressedKey;
}

void MyGameState::matchButtonCode()
{
	uint8 numberOfMatchingButtons = 0;
	
	for(uint8 i = 0; i < __PLUGIN_ALIGNMENT_CHECK_BUTTON_SEQUENCE_LENGTH; i++)
	{
		numberOfMatchingButtons += (AlignmentCheckButtonSequence[i] == this->lastInputs[i]);
	}

	if(numberOfMatchingButtons == __PLUGIN_ALIGNMENT_CHECK_BUTTON_SEQUENCE_LENGTH)
	{
		VUEngine::pause(VUEngine::getInstance(), GameState::safeCast(AlignmentCheckScreenState::getInstance()));
	}
}

void MyGameState::print()
{
    // Clear the screen first
    Printing::clear(Printing::getInstance());
	switch(winner)
	{
		case 'X':
			Printing::text(Printing::getInstance(), "X won!", 12, 6, "VirtualBoyExt");
			break;
		case 'O':
			Printing::text(Printing::getInstance(), "O won!", 12, 6, "VirtualBoyExt");
			break;
		case 'T':
			Printing::text(Printing::getInstance(), "Tie!", 12, 6, "VirtualBoyExt");
			break;
		default:
			Printing::text(Printing::getInstance(), "      ", 12, 6, "VirtualBoyExt");
	}
    for(int i = 0; i < 3; i++)
    {
        // Manually construct the output for each row
		buffer[0] = ' ';
        buffer[1] = board[i][0];  // First cell
        buffer[2] = '|';           // Separator
        buffer[3] = board[i][1];  // Second cell
        buffer[4] = '|';           // Separator
        buffer[5] = board[i][2];  // Third cell
		buffer[6] = ' ';
        buffer[7] = '\0';          // Null-terminate the string
		if(cursorRow == i)
		{
			buffer[cursorCol*2] = '<';
			buffer[cursorCol*2+2] = '>';
		}
        // Print the constructed row
        Printing::text(Printing::getInstance(), buffer, 12, 10 + i * 6, "VirtualBoyExt");

        // Print dashes between rows
        if(i < 2)
            Printing::text(Printing::getInstance(), dashes, 12, 13 + i * 6, "VirtualBoyExt");
    }
}
