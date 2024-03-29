// TicTacToe.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TicTacToe.h"
#include <windowsx.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;									// Current instance
WCHAR szTitle[MAX_LOADSTRING];						// The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];				// The main window class name


													// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_TICTACTOE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TICTACTOE));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TICTACTOE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TICTACTOE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_TICTACTOESMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

// Variables and functions for the game
#define PLAYER 1
#define COMPUTER 2
#define MAX 1000
#define MIN -1000

HBRUSH GREEN_BRUSH;									// Variable for green brush
HICON PIcon, CIcon;									// Variable for icons
const int CELL_SIZE = 150;							// The size of each cell
int playerTurn = 1;									// Initialize player's turn
int gameBoard[9] = { 0, 0, 0,						// Initialize gameboard
					 0, 0, 0,
					 0, 0, 0 };
int cells[] = { 0,1,2, 3,4,5, 6,7,8,				// Winning rows
				0,3,6, 1,4,7, 2,5,8,				// Winning columns
				0,4,8, 2,4,6 };						// Winning diagonals
int winner = 0;										// Initialize winner
int wins[3];										// Variable to hold winning values
int cell_index;

BOOL GetGameBoardRect(HWND hwnd, RECT *pRect)
{
	RECT rc;
	if (GetClientRect(hwnd, &rc))
	{
		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;

		pRect->left = (width - CELL_SIZE * 3) / 2;
		pRect->top = (height - CELL_SIZE * 3) / 2;

		pRect->right = pRect->left + CELL_SIZE * 3;
		pRect->bottom = pRect->top + CELL_SIZE * 3;

		return TRUE;
	}
	SetRectEmpty(pRect);
	return FALSE;
}

void DrawLine(HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
}

int GetCellNumberFromPoint(HWND hwnd, int x, int y)
{
	POINT pt = { x, y };
	RECT rc;

	if (GetGameBoardRect(hwnd, &rc))
	{
		if (PtInRect(&rc, pt))
		{
			// User clicked inside gameboard
			// Normalize (0 to 3*CELL_SIZE)
			x = pt.x - rc.left;
			y = pt.y - rc.top;

			int column = x / CELL_SIZE;
			int row = y / CELL_SIZE;

			// Convert to index (0 to 8)
			return column + row * 3;
		}
	}
	return -1; // User clicked outside gameboard or failure
}

BOOL GetCellRect(HWND hWnd, int index, RECT *pRect)
{
	RECT rcBoard;

	SetRectEmpty(pRect);
	if (index < 0 || index > 8)
	{
		return FALSE;
	}

	if (GetGameBoardRect(hWnd, &rcBoard))
	{
		// Convert index from 0-8 into x, y pair
		int y = index / 3; // Row number
		int x = index % 3; // Column number

		pRect->left = rcBoard.left + x * CELL_SIZE + 1;
		pRect->top = rcBoard.top + y * CELL_SIZE + 1;
		pRect->right = pRect->left + CELL_SIZE - 1;
		pRect->bottom = pRect->top + CELL_SIZE - 1;

		return TRUE;
	}
	return FALSE;
}

/*
Returns:
0 - No winner
1 - Player wins
2 - Computer wins
3 - Draw
*/
int GetWinner(int wins[3])
{
	// Check for winner
	for (int i = 0; i < ARRAYSIZE(cells); i += 3)
	{
		if (gameBoard[cells[i]] != 0 &&
			gameBoard[cells[i]] == gameBoard[cells[i + 1]] &&
			gameBoard[cells[i]] == gameBoard[cells[i + 2]])
		{
			wins[0] = cells[i];
			wins[1] = cells[i + 1];
			wins[2] = cells[i + 2];

			return gameBoard[cells[i]];
		}
	}

	// See if we have any empty cells left
	for (int i = 0; i < ARRAYSIZE(gameBoard); i++)
	{
		if (gameBoard[i] == 0)
		{
			return 0;
		}
	}
	return 3;
}

// Display player's turn and announce winner
void ShowTurn(HWND hwnd, HDC hdc)
{
	const WCHAR szTurn1[] = L"PLAYER's turn...";
	const WCHAR szTurn2[] = L"COMPUTER's turn...";

	const WCHAR *pszTurnText = (playerTurn == 1) ? szTurn1 : szTurn2;

	switch (winner)
	{
		// Continue to play
	case 0:
		pszTurnText = (playerTurn == 1) ? szTurn1 : szTurn2;
		break;
		// Player 1 wins
	case 1:
		pszTurnText = L"PLAYER is the winner!!!";
		break;
		// Player 2 wins
	case 2:
		pszTurnText = L"COMPUTER is the winner!!!";
		break;
		// It's a draw
	case 3:
		pszTurnText = L"It's a draw!!!";
		break;
	}

	RECT rc;
	if (pszTurnText != NULL && GetClientRect(hwnd, &rc))
	{
		rc.top = rc.bottom - 100;
		FillRect(hdc, &rc, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
		SetTextColor(hdc, RGB(255, 255, 255));
		SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, pszTurnText, lstrlen(pszTurnText), &rc, DT_CENTER);
	}
}

void DrawIconCentered(HDC hdc, RECT *pRect, HICON hIcon)
{
	const int ICON_WIDTH = GetSystemMetrics(SM_CXICON);
	const int ICON_HEIGHT = GetSystemMetrics(SM_CYICON);

	if (pRect != NULL)
	{
		int left = pRect->left + ((pRect->right - pRect->left) - ICON_WIDTH) / 2;
		int top = pRect->top + ((pRect->bottom - pRect->top) - ICON_HEIGHT) / 2;
		DrawIcon(hdc, left, top, hIcon);
	}
}

void HighlightWinner(HWND hwnd, HDC hdc)
{
	RECT rcWin;
	for (int i = 0; i < 3; i++)
	{
		if (GetCellRect(hwnd, wins[i], &rcWin))
		{
			FillRect(hdc, &rcWin, GREEN_BRUSH);
			DrawIconCentered(hdc, &rcWin, (winner == 1) ? PIcon : CIcon);
		}
	}
}

BOOL IsMovesLeft(int (&board)[9])
{
	for (int i = 0; i < 9; i++)
	{
		if (board[i] == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

// Evaluate winning value for each player
int Evaluate(int (&board)[9])
{
	// Checking for rows
	for (int i = 0; i < 9; i += 3)
	{
		if (board[cells[i]] == board[cells[i + 1]] &&
			board[cells[i + 1]] == board[cells[i + 2]])
		{
			if (board[cells[i]] == COMPUTER)
			{
				return 10;
			}
			else if (board[cells[i]] == PLAYER)
			{
				return -10;
			}
		}
	}

	// Checking for columns
	for (int i = 9; i < 18; i += 3)
	{
		if (board[cells[i]] == board[cells[i + 1]] &&
			board[cells[i + 1]] == board[cells[i + 2]])
		{
			if (board[cells[i]] == COMPUTER)
			{
				return 10;
			}
			else if (board[cells[i]] == PLAYER)
			{
				return -10;
			}
		}
	}

	// Checking for diagonals
	for (int i = 18; i < 24; i += 3)
	{
		if (board[cells[i]] == board[cells[i + 1]] &&
			board[cells[i + 1]] == board[cells[i + 2]])
		{
			if (board[cells[i]] == COMPUTER)
			{
				return 10;
			}
			else if (board[cells[i]] == PLAYER)
			{
				return -10;
			}
		}
	}
	return 0;
}

// Minimax algorithm
int Minimax(int (&board)[9], int depth, BOOL isMax)
{
	int score = Evaluate(board);

	// If Maximizer has won the game return his/her evaluated score
	if (score == 10)
	{
		return score;
	}

	// If Minimizer has won the game return his/her evaluated score
	if (score == -10)
	{
		return score;
	}

	// If there are no more moves and no winner then it is a draw
	if (IsMovesLeft(board) == FALSE)
	{
		return 0;
	}

	// If this Maximizer's move
	if (isMax)
	{
		int best = MIN;

		// Traverse all cells
		for (int i = 0; i < 9; i++)
		{
			if (board[i] == 0)
			{
				board[i] = COMPUTER;

				best = max(best, Minimax(board, depth + 1, !isMax));

				board[i] = 0;
			}
		}
		return best;
	}
	else
	{
		int best = MAX;

		// Traverse all cells
		for (int i = 0; i < 9; i++)
		{
			if (board[i] == 0)
			{
				board[i] = PLAYER;

				best = min(best, Minimax(board, depth + 1, !isMax));

				board[i] = 0;
			}
		}
		return best;
	}
}

// Find the best move for computer
int findBestMove(int (&board)[9])
{
	int bestVal = MIN;
	int bestMove;
	for (int i = 0; i < 9; i++)
	{
		if (board[i] == 0)
		{
			board[i] = COMPUTER;

			int moveVal = Minimax(board, 0, FALSE);

			board[i] = 0;

			if (moveVal > bestVal)
			{
				bestMove = i;
				bestVal = moveVal;
			}
		}
	}
	return bestMove;
}

// Make player/computer moves and display message
void Move(HWND hwnd, HDC hdc, int index)
{
	if (NULL != hdc)
	{
		// Get cell dimension from its index
		if (index != -1)
		{
			RECT rcCell;
			if (GetCellRect(hwnd, index, &rcCell))
			{
				if ((gameBoard[index] == 0))
				{
					if (playerTurn == PLAYER)
					{
						gameBoard[index] = playerTurn;
						DrawIconCentered(hdc, &rcCell, PIcon);
					}
					else if (playerTurn == COMPUTER)
					{
						gameBoard[index] = playerTurn;
						DrawIconCentered(hdc, &rcCell, CIcon);
					}
					// Check for winner
					winner = GetWinner(wins);
					if (winner == 0 && playerTurn == PLAYER)
					{
						playerTurn = COMPUTER;
					}
					else if (winner == 0 && playerTurn == COMPUTER)
					{
						playerTurn = PLAYER;
					}
					else if (winner == 1)
					{
						HighlightWinner(hwnd, hdc);
						// We have a winner
						MessageBox(hwnd, L"CONGRATULATION!!!", L"You Won...", MB_OK | MB_ICONINFORMATION);
						playerTurn = 0;
					}
					else if (winner == 2)
					{
						HighlightWinner(hwnd, hdc);
						// We have a winner
						MessageBox(hwnd, L"BETTER LUCK NEXT TIME!!!", L"You Lost...", MB_OK | MB_ICONINFORMATION);
						playerTurn = 0;
					}
					else if (winner == 3)
					{
						// It's a draw
						MessageBox(hwnd, L"REMATCH?", L"It's a draw!!!", MB_OK | MB_ICONINFORMATION);
						playerTurn = 0;
					}
					// Display player turn
					ShowTurn(hwnd, hdc);
				}
			}
		}
		ReleaseDC(hwnd, hdc);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		GREEN_BRUSH = CreateSolidBrush(RGB(0, 255, 0));

		// Load player icons
		PIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PLAYER));
		CIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_COMPUTER));
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_NEWGAME:
		{
			int ret = MessageBox(hWnd, L"Are you sure you want to start a new game?", L"New Game", MB_YESNO | MB_ICONQUESTION);
			if (IDYES == ret)
			{
				// Reset and start a new game
				playerTurn = 1;
				winner = 0;
				ZeroMemory(gameBoard, sizeof(gameBoard));
				// Force a paint message
				InvalidateRect(hWnd, NULL, TRUE); // Post WM_PAINT to our windowProc. It gets queued in our msg queue
				UpdateWindow(hWnd); // Forces immediate handling of WM_PAINT
			}
		}
		break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_LBUTTONDOWN: // Player makes move
	{
		// Only handle clicks if it is player turn
		if (playerTurn == COMPUTER)
		{
			break;
		}
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		HDC hdc = GetDC(hWnd);
		int index = GetCellNumberFromPoint(hWnd, xPos, yPos);
		Move(hWnd, hdc, index);
	}
	break;
	case WM_LBUTTONUP: // Computer makes move
	{
		// Only handle if it is computer turn
		if (playerTurn == PLAYER)
		{
			break;
		}
		HDC hdc = GetDC(hWnd);
		int index = findBestMove(gameBoard);
		Move(hWnd, hdc, index);
	}
	break;
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO * pMinMax = (MINMAXINFO*)lParam;

		pMinMax->ptMinTrackSize.x = CELL_SIZE * 5;
		pMinMax->ptMinTrackSize.y = CELL_SIZE * 5;
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		RECT rc;

		if (GetGameBoardRect(hWnd, &rc))
		{
			RECT rcClient;

			// Display player text and turn
			if (GetClientRect(hWnd, &rcClient))
			{
				const WCHAR szPlayer1[] = L"PLAYER";
				const WCHAR szPlayer2[] = L"COMPUTER";

				SetBkMode(hdc, TRANSPARENT);
				// Draw Player text
				SetTextColor(hdc, RGB(255, 255, 255));
				TextOut(hdc, 16, 16, szPlayer1, ARRAYSIZE(szPlayer1));
				DrawIcon(hdc, 24, 40, PIcon);

				// Draw Computer text
				SetTextColor(hdc, RGB(255, 255, 255));
				TextOut(hdc, rcClient.right - 84, 16, szPlayer2, ARRAYSIZE(szPlayer2));
				DrawIcon(hdc, rcClient.right - 58, 40, CIcon);

				// Display player turn
				ShowTurn(hWnd, hdc);
			}

			FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

			for (int i = 0; i < 4; i++)
			{
				// Draw vertical line
				DrawLine(hdc, rc.left + CELL_SIZE * i, rc.top, rc.left + CELL_SIZE * i, rc.bottom);
				// Draw horizontal line
				DrawLine(hdc, rc.left, rc.top + CELL_SIZE * i, rc.right, rc.top + CELL_SIZE * i);
			}

			// Draw all occupied cells
			RECT rcCell;
			for (int i = 0; i < ARRAYSIZE(gameBoard); i++)
			{
				if ((gameBoard[i] != 0) && GetCellRect(hWnd, i, &rcCell))
				{
					DrawIconCentered(hdc, &rcCell, (gameBoard[i] == 2) ? CIcon : PIcon);
				}
			}
			if (winner == 1 || winner == 2)
			{
				// Highlight winner
				HighlightWinner(hWnd, hdc);
			}
		}
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		// Dispose of green brush
		DeleteObject(GREEN_BRUSH);
		// Dispose of icons images
		DestroyIcon(PIcon);
		DestroyIcon(CIcon);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
