#include <string>
#include <vector>
#include <map>
#include <memory>

#include "register.h"

namespace square_merge_game {

class ScoreMethod {
};

class MoveMethod {
};

class TextMethod {
};

class Goal {
};

struct GameOptions {
  int rand_seed;
  int game_size;
  int max_undo;
  std::unique_ptr<TextMethod> text_method;
  std::unique_ptr<ScoreMethod> score_method;
  std::unique_ptr<MoveMethod> move_method;
  std::unique_ptr<Goal> goal;
};

class GameBoard {
};

class GameState {
  std::vector<GameBoard> history;
};

class SquareMergeGame {
 public:
  SquareMergeGame(const GameOptions& options);
  const GameOptions& options() {
    return options_;
  }
  virtual void SetRules();

 protected:
  SquareMergeGame() {}

 private:
  GameOptions options_;
};

class Game2048: public SquareMergeGame {
};

};  // namespace square_merge_game
