/*
	This software is part of the Queens Generation Tool
	Copyright (C) 2025 Jonas Blackwood

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

#include <string>
#include <cassert>
#include <fstream>

const int dirsX[4] = {1, 0, -1, 0};
const int dirsY[4] = {0, 1, 0, -1};

// === DATA STRUCTURES ===
struct Point {
	int x;
	int y;

	const bool operator==(const Point& other) {
		return x == other.x && y == other.y;
	}

	bool inBoard (const int boardWidth) {
		return x >= 0 && x < boardWidth && y >= 0 && y < boardWidth;
	}
};

struct ColoredPoint {
	int x;
	int y;
	int color;

	bool operator==(const Point& other) {
		return x == other.x && y == other.y;
	}

	bool inBoard (const int boardWidth) {
		return x >= 0 && x < boardWidth && y >= 0 && y < boardWidth;
	}

	Point p () {
		Point _p = {x, y};
		return _p;
	}
};

using PointVector = std::vector<Point>;
using ColoredPointVector = std::vector<ColoredPoint>;

// === RANDOM UTILITIES ===
std::random_device rd;
std::mt19937 randomGenerator(rd());

template <typename T>
void shuffleVector (std::vector<T> &vec) {
	std::shuffle(vec.begin(), vec.end(), randomGenerator);
}

bool pointInVector (Point p, PointVector &vec) {
	auto it = std::find(vec.begin(), vec.end(), p);
	return it != vec.end();
}

// === COLOR MAP CLASS ===
class ColorMap {
private:
	int width;
	std::vector<int> matrix;

public:
	ColorMap (int _width) : width(_width) {}

	void addRow(std::vector<int> row) {
		for (int c : row) {
			matrix.push_back(c);
		}
	}

	int getPointColor(const Point &p) {return matrix.at(p.y*width + p.x);}
	void setMatrixValue(const Point &p, int color) {matrix.at(p.y*width + p.x) = color;}

	void fillColor(int oldColor, int newColor) {
		for (int &color : matrix) {
			if (color == oldColor) color = newColor;
		}
	}

	void fill(int color) {
		matrix.clear();
		for (int i = 0; i < width*width; i++) {
			matrix.push_back(color);
		}
	}
};

// === BOARD CLASS ===
// A completed game with all queens present
class Board {
private:
	int width;
	PointVector queens;
	std::vector<int> queenColors;
	ColorMap colorMap;

public:
	Board(ColorMap _colorMap, const PointVector _queens) : queens(_queens), colorMap(_colorMap) {
		width = queens.size();
		for (Point &queen : queens) {
			queenColors.push_back(getPointColor(queen));
		}
	};

	// --- Color Management ---
	ColorMap *getColorMap() {return &colorMap;}
	void fillColor(int color, int newColor) {colorMap.fillColor(color, newColor);}
	int getPointColor(const Point &p) {return colorMap.getPointColor(p);}
	void setPointColor(const Point &p, int color) {colorMap.setMatrixValue(p, color);}

	// --- Queen Management ---
	bool pointIsQueen(const Point &p) {
		auto it = std::find(queens.begin(), queens.end(), p);
		return it != queens.end();
	}

	// --- Board Data ---
	int getWidth() {return width;}

	// --- Connectivity Algorithms ---
	bool pointsAreConnected(Point &start, Point &target, PointVector &visitedPoints) {
		if (!start.inBoard(width)) return false;
		else if (getPointColor(start) != getPointColor(target)) return false; // If different colors, not connected
		else if (start == target) return true; // If they are the same point, connected

		for (int dir = 0; dir < 4; dir++) { // For every adjacent point
			Point adjP = {start.x+dirsX[dir], start.y+dirsY[dir]};
			if (!pointInVector(adjP, visitedPoints)) { // If the point is not visited
				visitedPoints.push_back(adjP);
				if (pointsAreConnected(adjP, target, visitedPoints)) { // If adj point connects to target, return true
					return true;
				}
			}
		}

		return false;
	}

	bool pointsAreConnected(Point start, Point target) {
		PointVector visitedPoints;
		return pointsAreConnected(start, target, visitedPoints);
	}

	bool pointConnectsToQueen(const Point &p) {
		// Get the color
		int color = getPointColor(p);

		// Find the queen
		Point queen;
		auto it = std::find(queenColors.begin(), queenColors.end(), color);
		if (it != queenColors.end()) {
			// Calculate the index
			size_t index = std::distance(queenColors.begin(), it);
			queen = queens.at(index);
		} else {
			std::cerr << "ERROR! " <<  color << " does not have a queen!!!" << std::endl;
		}

		// Return if the point connects to the queen
		return pointsAreConnected(p, queen);
	}

	bool colorChangeBreaksContinuity (const Point &p, int color) {
		int originalColor = getPointColor(p); // Replace the point with a new color
		setPointColor(p, color);

		for (int dir = 0; dir < 4; dir++) { // For every adjacent point
			Point adjP = {p.x+dirsX[dir], p.y+dirsY[dir]};
			if (adjP.inBoard(width)) if (getPointColor(adjP) != color) { // If the adjacent point is on the board and a different color
				if (!pointConnectsToQueen(adjP)) { // If it is cut off from its queen
					setPointColor(p, originalColor);
					return true;
				}
			}
		}

		setPointColor(p, originalColor);
		return false;
	}

	// --- Output Methods ---
	void print(bool hideQueens=false) {
		for (int y = 0; y < width; y++) {
			for (int x = 0; x < width; x++) {
				Point p = {x, y};
				if (pointIsQueen(p) && !hideQueens) std::cout << "\033[33m";
				std::cout << colorMap.getPointColor(p)<< " \033[0m";
			}
			std::cout << std::endl;
		}
	}

	void outputToCsv(std::string filepath) {
		std::ofstream file(filepath);
		if (file.is_open()) {
			int queensX[width];
			for (int y = 0; y < width; y++) {
				for (int x = 0; x < width; x++) {
					Point p = {x, y};
					if (pointIsQueen(p)) queensX[y] = x;
					file << colorMap.getPointColor(p) << ((x < width-1) ? ',' : '\n');
				}
			}
			for (int i = 0; i < width; i++) {
				file << queensX[i] << ((i < width-1) ? ',' : '\n');
			}
		}
	}
};

// === INCOMPLETE BOARD CLASS ===
// Used while solving / finding # of solutions of a board
class IncompleteBoard {
private:
	int width;
	ColorMap colorMap;
	PointVector queens;
	int xDepth = 0;

public:
	IncompleteBoard(int _width, const PointVector _queens = {}) : colorMap(_width), queens(_queens) {
		width = _width;
		xDepth = queens.size();
		colorMap.fill(-1);
	}

	IncompleteBoard(Board &board) : IncompleteBoard(board.getWidth()) {
		colorMap = *board.getColorMap();
	}

	// --- Color Management ---
	void setColorMap(const ColorMap &newMap) {colorMap = newMap;}
	void fillColor(int color, int newColor) {colorMap.fillColor(color, newColor);}
	int getPointColor(const Point &p) {return colorMap.getPointColor(p);}
	void setPointColor(const Point &p, int color) {colorMap.setMatrixValue(p, color);}

	// --- Completion ---
	bool isComplete() {
		return xDepth == width;
	}
	Board makeBoard() {
		if (!isComplete()) std::cerr << "ERROR: Tried to make board with unfinished IncompleteBoard" << std::endl;

		Board newBoard(colorMap, queens);
		return newBoard;
	}

	// --- Queen Management ---
	bool queenIsValid(const Point &newQueen, const bool checkColor=1) {
		// Not the same Y or color
		for (Point queen : queens) {
			if (newQueen.y == queen.y || (checkColor && getPointColor(newQueen) == getPointColor(queen))) return 0;
		}

		// Not touching
		if (xDepth > 0) {
			Point lastQueen = queens.at(xDepth-1);
			if (newQueen.y == lastQueen.y-1 || newQueen.y == lastQueen.y+1) return 0;
		}

		return 1;
	}

	const PointVector getQueens () {return queens;}
	bool pointIsQueen(const Point &p) {return std::find(queens.begin(), queens.end(), p) != queens.end();}
	void addQueen(const Point &newQueen) {
		queens.push_back(newQueen);
		xDepth += 1;
	}
	void removeQueen() {
		queens.pop_back();
		xDepth -= 1;
	}
	void removeAllQueens () {while (xDepth > 0) removeQueen();}

	// --- Algorithm ---
	void genRandomQueens () {
		if (isComplete()) return; // Return if the board is done

		// Get possible new queens
		PointVector newQueenVector;
		for (int newY = 0; newY < width; newY++) { // For every y position of the next column
			Point newQueen = {xDepth, newY};
			if (queenIsValid(newQueen, 0)) { // If the new queen is valid, add it
				newQueenVector.push_back(newQueen);
			}
		}

		shuffleVector(newQueenVector); // Shuffle new queens

		for (Point &queen : newQueenVector) {
			addQueen(queen);

			genRandomQueens();
			if (isComplete()) {
				return;
			}

			removeQueen();
		}
	}

	bool hasOneSolution (int &numSolutions) {
		if (isComplete()) {
			numSolutions++;
			return true;
		}

		for (int newY = 0; newY < width; newY++) { // For every y position of the next column
			Point newQueen = {xDepth, newY};
			if (queenIsValid(newQueen)) { // If valid, add to board
				addQueen(newQueen);
				hasOneSolution(numSolutions);
				removeQueen();

				if (numSolutions > 1) return false;
			}
		}

		return numSolutions == 1;
	}

	bool hasOneSolution() {
		int start = 0;
		return hasOneSolution(start);
	}

	// --- Output Methods ---
	void print() {
		for (int y = 0; y < width; y++) {
			for (int x = 0; x < width; x++) {
				Point p = {x, y};
				if (pointIsQueen(p)) std::cout << "\033[33m";
				std::cout << colorMap.getPointColor(p) << " \033[0m";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
};

// === GAME GENERATOR CLASS ===
class GameGenerator {
private:
	const int gameSize;
	const bool continuousBase;
	const int maxSpreadNumber = 120;

	bool isValidGame(Board &board) {
		IncompleteBoard working = IncompleteBoard(board);
		return working.hasOneSolution();
	}

	// --- Color Spreading Algorithms ---
	ColoredPointVector getPossibleColorSpreads(Board &board, const int excludedColor) {
		ColoredPointVector points;

		// All points
		for (int x = 0; x < gameSize; x++) for (int y = 0; y < gameSize; y++) { // For every point
			Point p = {x, y};
			int pColor = board.getPointColor(p);
			if (pColor != excludedColor) { // If the point color is not the excluded color: spread

				// Adjacent points
				for (int dir = 0; dir < 4; dir++) {
					Point adjPoint = {x+dirsX[dir], y+dirsY[dir]};
					if ( adjPoint.inBoard(gameSize) ) { // If in board
						int adjPointColor = board.getPointColor(adjPoint);
						if (
							adjPointColor == excludedColor
							&& !(board.colorChangeBreaksContinuity(adjPoint, pColor) && continuousBase)
							&& !board.pointIsQueen(adjPoint)
						) {
							points.push_back({adjPoint.x, adjPoint.y, pColor});
						}
					}
				}
			}
		}

		return points;
	}

	bool doOneColorSpread (Board &board, int excludedColor) {
		ColoredPointVector colorSpreadPoints = getPossibleColorSpreads(board, excludedColor);
		shuffleVector(colorSpreadPoints);
		// std::cout << colorSpreadPoints.size() << std::endl;

		for (ColoredPoint &p : colorSpreadPoints) {
			int replacedColor = board.getPointColor(p.p()); // Store the color we will replace

			board.setPointColor(p.p(), p.color);
			if (isValidGame(board)) {
				return 1;
			} else {
				board.setPointColor(p.p(), replacedColor);
			}
		}

		return 0;
	}

public:
	GameGenerator (int _gameSize, bool _continuousBase) : gameSize(_gameSize), continuousBase(_continuousBase) {

	}

	Board generateGame () {
		// Get the queen structure
		IncompleteBoard incompleteBoard(gameSize);
		incompleteBoard.genRandomQueens();

		// Apply colors
		for (Point queen : incompleteBoard.getQueens()) {
			incompleteBoard.setPointColor(queen, queen.x);
		}

		// Set excluded color
		std::uniform_int_distribution<> dist(0, gameSize-1);
		int excludedColor = dist(randomGenerator);
		incompleteBoard.fillColor(-1, excludedColor);
		std::cout << "Base color (excluded): " << excludedColor << std::endl;

		// Complete board
		Board board = incompleteBoard.makeBoard();
		std::cout << "Completed base board." << std::endl;
		// board.print();

		int numSpreads = gameSize*gameSize;
		if (numSpreads > maxSpreadNumber) numSpreads = maxSpreadNumber;
		int realSpreads = 0;
		std::cout << "Spreading colors around " << numSpreads << " times" << std::endl;
		for (int i = 0; i < numSpreads; i++) {
			doOneColorSpread(board, excludedColor);
			realSpreads++;
			std::cout << "\r\tDone: " << realSpreads << std::flush;
		}
		// std::cout << std::endl << "\tActually did " << realSpreads << std::endl;

		std::cout << std::endl << "Completed Board: " << std::endl;
		return board;
	}
};

// === UTILITY FUNCTIONS ===
void printUsage() {
	std::cout << "Usage: queens generate [BOARD_SIZE] [options]" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h, --help                     Print this help menu" << std::endl;
	std::cout << "  -nq, --hide-queens             Don't color the queens in board output" << std::endl;
	std::cout << "  -nc, --non-continuous          Allows the base to be non-continuous in generation" << std::endl;
	std::cout << "  -s [FILE], --to-csv [FILE]     Creates a csv file of the color map. Adds an array on last row of the X of queens according to Y ascending" << std::endl;
}

// === MAIN ===
int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << "INPUT ERROR" << std::endl;
		printUsage();
		return 1;
	} else if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
		printUsage();
		return 0;
	}

	std::string subcommand = argv[1];
	if (subcommand == "generate") {
		int boardSize = std::stoi(argv[2]);
		// Verify board size
		if (boardSize < 4) {
			std::cerr << "INPUT ERROR: Board size must be greater than 4" << std::endl;
			return 1;
		} else if (boardSize > 12) {
			std::cerr << "WARNING: A board size greater than 12 is not recommended. Continue? (y/N)" << std::endl;
			char cont;
			std::cin >> cont;
			if (!(cont == 'y' || cont == 'Y')) {
				std::cerr << "Abort." << std::endl;
				return 1;
			}
		}

		// Flags
		bool continuousBase = true;
		bool saveToCsv = false;
		std::string csvPath;
		bool hideQueens = false;
		for (int i = 3; i < argc; i++) {
			std::string flag = argv[i];
			if (flag == "--non-continuous" || flag == "-nc") {
				continuousBase = false;
				std::cout << "Generating with non-continuous base" << std::endl;
			} else if (flag == "--to-csv" || flag == "-s") {
				saveToCsv = true;
				csvPath = argv[i+1];
			} else if (flag == "--hide-queens" || flag == "-nq") {
				hideQueens = true;
			}
		}

		// Generate
		GameGenerator gameGen(boardSize, continuousBase);
		Board board = gameGen.generateGame();
		board.print(hideQueens);

		// Save
		if (saveToCsv) {
			std::cout << "Saving to " + csvPath << std::endl;
			board.outputToCsv(csvPath);

		}
	} else {
		std::cerr << "INPUT ERROR: No subcommand '"+subcommand+"'" << std::endl;
	}


	return 0;
}

/*
	Author: tux76
	Date: 5-16-2025
 */
