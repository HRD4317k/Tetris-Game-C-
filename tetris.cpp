#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <iostream>

const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;
const int CELL_SIZE = 30;
const int WINDOW_WIDTH = BOARD_WIDTH * CELL_SIZE + 200;
const int WINDOW_HEIGHT = BOARD_HEIGHT * CELL_SIZE + 100;

enum class TetrominoType {
    I, O, T, S, Z, J, L, NONE
};

struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
};

class Tetromino {
public:
    TetrominoType type;
    std::vector<std::vector<Point>> shapes;
    int currentRotation;
    Point position;
    sf::Color color;

    Tetromino(TetrominoType t) : type(t), currentRotation(0), position(BOARD_WIDTH/2 - 1, 0) {
        initializeShapes();
        setColor();
    }

private:
    void initializeShapes() {
        switch(type) {
            case TetrominoType::I:
                shapes = {
                    {{0,1}, {1,1}, {2,1}, {3,1}},
                    {{2,0}, {2,1}, {2,2}, {2,3}},
                    {{0,2}, {1,2}, {2,2}, {3,2}},
                    {{1,0}, {1,1}, {1,2}, {1,3}}
                };
                break;
            case TetrominoType::O:
                shapes = {
                    {{0,0}, {1,0}, {0,1}, {1,1}},
                    {{0,0}, {1,0}, {0,1}, {1,1}},
                    {{0,0}, {1,0}, {0,1}, {1,1}},
                    {{0,0}, {1,0}, {0,1}, {1,1}}
                };
                break;
            case TetrominoType::T:
                shapes = {
                    {{1,0}, {0,1}, {1,1}, {2,1}},
                    {{1,0}, {1,1}, {2,1}, {1,2}},
                    {{0,1}, {1,1}, {2,1}, {1,2}},
                    {{1,0}, {0,1}, {1,1}, {1,2}}
                };
                break;
            case TetrominoType::S:
                shapes = {
                    {{1,0}, {2,0}, {0,1}, {1,1}},
                    {{1,0}, {1,1}, {2,1}, {2,2}},
                    {{1,1}, {2,1}, {0,2}, {1,2}},
                    {{0,0}, {0,1}, {1,1}, {1,2}}
                };
                break;
            case TetrominoType::Z:
                shapes = {
                    {{0,0}, {1,0}, {1,1}, {2,1}},
                    {{2,0}, {1,1}, {2,1}, {1,2}},
                    {{0,1}, {1,1}, {1,2}, {2,2}},
                    {{1,0}, {0,1}, {1,1}, {0,2}}
                };
                break;
            case TetrominoType::J:
                shapes = {
                    {{0,0}, {0,1}, {1,1}, {2,1}},
                    {{1,0}, {2,0}, {1,1}, {1,2}},
                    {{0,1}, {1,1}, {2,1}, {2,2}},
                    {{1,0}, {1,1}, {0,2}, {1,2}}
                };
                break;
            case TetrominoType::L:
                shapes = {
                    {{2,0}, {0,1}, {1,1}, {2,1}},
                    {{1,0}, {1,1}, {1,2}, {2,2}},
                    {{0,1}, {1,1}, {2,1}, {0,2}},
                    {{0,0}, {1,0}, {1,1}, {1,2}}
                };
                break;
            default:
                break;
        }
    }

    void setColor() {
        switch(type) {
            case TetrominoType::I: color = sf::Color::Cyan; break;
            case TetrominoType::O: color = sf::Color::Yellow; break;
            case TetrominoType::T: color = sf::Color::Magenta; break;
            case TetrominoType::S: color = sf::Color::Green; break;
            case TetrominoType::Z: color = sf::Color::Red; break;
            case TetrominoType::J: color = sf::Color::Blue; break;
            case TetrominoType::L: color = sf::Color(255, 165, 0); break; // Orange
            default: color = sf::Color::White; break;
        }
    }
};

class TetrisGame {
private:
    std::vector<std::vector<sf::Color>> board;
    Tetromino* currentPiece;
    Tetromino* nextPiece;
    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;
    sf::Clock dropTimer;
    sf::Clock keyTimer;
    float dropInterval;
    int score;
    int lines;
    int level;
    bool gameOver;

public:
    TetrisGame() : board(BOARD_HEIGHT, std::vector<sf::Color>(BOARD_WIDTH, sf::Color::Black)),
                   rng(std::random_device{}()), dist(0, 6), dropInterval(1.0f),
                   score(0), lines(0), level(1), gameOver(false) {
        currentPiece = createRandomPiece();
        nextPiece = createRandomPiece();
    }

    ~TetrisGame() {
        delete currentPiece;
        delete nextPiece;
    }

    Tetromino* createRandomPiece() {
        TetrominoType types[] = {TetrominoType::I, TetrominoType::O, TetrominoType::T,
                                TetrominoType::S, TetrominoType::Z, TetrominoType::J, TetrominoType::L};
        return new Tetromino(types[dist(rng)]);
    }

    std::vector<Point> getCurrentPieceBlocks() {
        std::vector<Point> blocks;
        for (const Point& p : currentPiece->shapes[currentPiece->currentRotation]) {
            blocks.push_back(Point(p.x + currentPiece->position.x, p.y + currentPiece->position.y));
        }
        return blocks;
    }

    bool isValidPosition(const std::vector<Point>& blocks) {
        for (const Point& block : blocks) {
            if (block.x < 0 || block.x >= BOARD_WIDTH || block.y >= BOARD_HEIGHT) {
                return false;
            }
            if (block.y >= 0 && board[block.y][block.x] != sf::Color::Black) {
                return false;
            }
        }
        return true;
    }

    void rotatePiece() {
        int oldRotation = currentPiece->currentRotation;
        currentPiece->currentRotation = (currentPiece->currentRotation + 1) % 4;
        
        if (!isValidPosition(getCurrentPieceBlocks())) {
            currentPiece->currentRotation = oldRotation;
        }
    }

    void movePiece(int dx, int dy) {
        currentPiece->position.x += dx;
        currentPiece->position.y += dy;
        
        if (!isValidPosition(getCurrentPieceBlocks())) {
            currentPiece->position.x -= dx;
            currentPiece->position.y -= dy;
        }
    }

    bool dropPiece() {
        currentPiece->position.y++;
        if (!isValidPosition(getCurrentPieceBlocks())) {
            currentPiece->position.y--;
            placePiece();
            return false;
        }
        return true;
    }

    void placePiece() {
        std::vector<Point> blocks = getCurrentPieceBlocks();
        for (const Point& block : blocks) {
            if (block.y >= 0) {
                board[block.y][block.x] = currentPiece->color;
            }
        }
        
        // Check for game over
        for (const Point& block : blocks) {
            if (block.y < 0) {
                gameOver = true;
                return;
            }
        }
        
        clearLines();
        
        delete currentPiece;
        currentPiece = nextPiece;
        nextPiece = createRandomPiece();
        currentPiece->position = Point(BOARD_WIDTH/2 - 1, 0);
    }

    void clearLines() {
        int linesCleared = 0;
        for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
            bool fullLine = true;
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (board[y][x] == sf::Color::Black) {
                    fullLine = false;
                    break;
                }
            }
            
            if (fullLine) {
                board.erase(board.begin() + y);
                board.insert(board.begin(), std::vector<sf::Color>(BOARD_WIDTH, sf::Color::Black));
                linesCleared++;
                y++; // Check the same line again
            }
        }
        
        if (linesCleared > 0) {
            lines += linesCleared;
            score += linesCleared * 100 * level;
            level = lines / 10 + 1;
            dropInterval = std::max(0.1f, 1.0f - (level - 1) * 0.1f);
        }
    }

    void handleInput(sf::Event& event) {
        if (gameOver) return;
        
        if (event.type == sf::Event::KeyPressed) {
            if (keyTimer.getElapsedTime().asMilliseconds() > 100) {
                switch (event.key.code) {
                    case sf::Keyboard::Left:
                        movePiece(-1, 0);
                        break;
                    case sf::Keyboard::Right:
                        movePiece(1, 0);
                        break;
                    case sf::Keyboard::Down:
                        dropPiece();
                        break;
                    case sf::Keyboard::Up:
                        rotatePiece();
                        break;
                    case sf::Keyboard::Space:
                        while (dropPiece()) {}
                        break;
                    default:
                        break;
                }
                keyTimer.restart();
            }
        }
    }

    void update() {
        if (gameOver) return;
        
        if (dropTimer.getElapsedTime().asSeconds() > dropInterval) {
            dropPiece();
            dropTimer.restart();
        }
    }

    void render(sf::RenderWindow& window) {
        window.clear(sf::Color::Black);
        
        // Draw board
        sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
        for (int y = 0; y < BOARD_HEIGHT; y++) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                cell.setPosition(x * CELL_SIZE + 50, y * CELL_SIZE + 50);
                cell.setFillColor(board[y][x]);
                if (board[y][x] == sf::Color::Black) {
                    cell.setOutlineThickness(1);
                    cell.setOutlineColor(sf::Color(64, 64, 64));
                } else {
                    cell.setOutlineThickness(0);
                }
                window.draw(cell);
            }
        }
        
        // Draw current piece
        if (!gameOver) {
            std::vector<Point> blocks = getCurrentPieceBlocks();
            cell.setFillColor(currentPiece->color);
            cell.setOutlineThickness(0);
            for (const Point& block : blocks) {
                if (block.y >= 0) {
                    cell.setPosition(block.x * CELL_SIZE + 50, block.y * CELL_SIZE + 50);
                    window.draw(cell);
                }
            }
        }
        
        // Draw UI
        sf::Font font;
        // Note: You'll need to load a font file or use the default font
        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::White);
        
        text.setPosition(BOARD_WIDTH * CELL_SIZE + 70, 50);
        text.setString("Score: " + std::to_string(score));
        window.draw(text);
        
        text.setPosition(BOARD_WIDTH * CELL_SIZE + 70, 80);
        text.setString("Lines: " + std::to_string(lines));
        window.draw(text);
        
        text.setPosition(BOARD_WIDTH * CELL_SIZE + 70, 110);
        text.setString("Level: " + std::to_string(level));
        window.draw(text);
        
        if (gameOver) {
            text.setCharacterSize(30);
            text.setPosition(100, BOARD_HEIGHT * CELL_SIZE / 2);
            text.setString("GAME OVER");
            window.draw(text);
        }
        
        window.display();
    }

    bool isGameOver() const { return gameOver; }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Tetris");
    window.setFramerateLimit(60);
    
    TetrisGame game;
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            game.handleInput(event);
        }
        
        game.update();
        game.render(window);
    }
    
    return 0;
}