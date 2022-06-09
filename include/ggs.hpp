#ifndef GGS_HPP
#define GGS_HPP

#include <optional>
#include <cassert>

namespace ggs {

template<typename GameState, typename GameMove>
class OptimalMoveSolver {
private:

  struct ValuedMove;

  static ValuedMove find_optimal_move_impl(const GameState &state,
    ValuedMove alpha,
    ValuedMove beta
  );

public:

  static GameMove find_optimal_move(const GameState &state);

};

template<typename GameState, typename GameMove>
GameMove
OptimalMoveSolver<GameState, GameMove>::find_optimal_move(const GameState &state) {
  auto result = find_optimal_move_impl(
      state,
      (ValuedMove) {-std::numeric_limits<float>::infinity(), std::nullopt},
      (ValuedMove) {std::numeric_limits<float>::infinity(), std::nullopt}
    ).move;
  assert(result.has_value());
  return *result;
}

template<typename GameState, typename GameMove>
struct OptimalMoveSolver<GameState, GameMove>::ValuedMove {
  float value; // NOLINT
  std::optional<GameMove> move; // NOLINT

  bool operator<(const ValuedMove &other) const {
    return value < other.value;
  }
};

template<typename GameState, typename GameMove>
typename OptimalMoveSolver<GameState, GameMove>::ValuedMove
OptimalMoveSolver<GameState, GameMove>::find_optimal_move_impl(
    const GameState &state,
    ValuedMove alpha, ValuedMove beta) {
  if (state.is_terminal()) {
    float fv = state.get_value();
    return (ValuedMove){
      fv,
      std::nullopt
    };
  }
 
  auto value = (ValuedMove){
    state.is_min_player() ? std::numeric_limits<float>::infinity() : -std::numeric_limits<float>::infinity(),
    std::nullopt
  };

  auto update_bounds_and_value =
    [&value, &alpha, &beta, &state] (const ValuedMove &new_value) {
    if (state.is_min_player()) {
      value = std::min(new_value, value);
      beta = std::min(value, beta);
    } else {
      value = std::max(new_value, value);
      alpha = std::max(value, alpha);
    }
  };

  auto should_prune = [&alpha, &beta] () {
    // alpha >= beta
    return !(alpha < beta);
  };

  for (GameMove m : state.get_moves()) {
    GameState new_state = state;
    new_state.apply_move(m);
    ValuedMove new_move = find_optimal_move_impl(new_state, alpha, beta);
    new_move.move = m;
    update_bounds_and_value(new_move);
    if (should_prune()) {
      break;
    }
  }
  return value;
}

} // namespace ggs

#endif // GGS_HPP