#ifndef GAME_H_
#define GAME_H_

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "register.h"

class GameBoard;
class GameState;

class Move: RegisterBase<Move> {
 public:
  virtual const char* info() override {
    return "Type of possible interractions for the game.";
  }

  // Returns true if input captured. Cleans up the input buffer.
  virtual bool check() {
    return false;
  }
};

class MoveMethod: RegisterBase<MoveMethod> {
 public:
  virtual const char* info() override {
    return "The method to perform different moves.";
  }
};

class TextMethod: RegisterBase<TextMethod> {
 public:
  virtual const char* info() override {
    return "Versions of all text contents in the game.";
  }

  // Use own texts by a set of custom entries in the game.
  virtual const char* get_text(std::string entry) {
    return "";
  }
};

class Event: RegisterBase<Event> {
 public:
  virtual const char* info() override {
    return "Different events in the game.";
  }

  // Returns true if event captured.
  virtual bool check(const GameState&) = 0;
};

class WinEvent: Event {
 public:
  virtual const char* info() override {
    return "Game winning conditions.";
  }

  // Returns true if win.
  virtual bool check(const GameState&) override {
    return false;
  }
};

class LoseEvent: Event {
 public:
  virtual const char* info() override {
    return "Game losing conditions.";
  }

  // Returns true if lose.
  virtual bool check(const GameState&) override {
    return false;
  }
};

class ScoreEvent: Event {
 public:
  virtual const char* info() override {
    return "Methods to score the game.";
  }

  // Returns true if score need to update.
  virtual bool check(const GameState&) override {
    return false;
  }
};

struct GameOptions {
  int rand_seed;
  int game_size;
  int max_undo;
  TextMethod::ptr text_method;
  MoveMethod::ptr move_method;
  WinEvent::ptr win_event;
  LoseEvent::ptr lose_event;
  std::vector<Move::ptr> move;
};

class GameBoard {
 public:
  std::vector<int> board;
};

class GameState {
 public:
  GameOptions options;
  std::vector<GameBoard> history;

  const char* save_state();

  bool load_state(std::string state_string);
};

class SquareMergeGame {
 public:
  SquareMergeGame(const GameOptions& options) {
    state.options = options;
  }
  const GameOptions& options() {
    return state.options;
  }

  bool advance();

  const char* help();

 protected:
  SquareMergeGame() {}

 private:
  GameState state;
};

#endif  // GAME_H_
