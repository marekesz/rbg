//
// Created by ??? on 01/02/18.
//

#include <iostream>
#include <chrono>
#include <iomanip>

#include "networking/client.h"
#include <game_items.hpp>
#include "game_components/game_description.h"
#include "parser_connection/parser_connection.h"
#include "game_components/game_state.h"
#include "utils/utilities.h"

static void collect_move_usages(token_id_t previous_player, const GameState &s,
                                const Move &m,
                                std::vector<std::vector<size_t> > *result) {
  token_id_t player = s.player();
  if (player == s.description().keeper_player_id() ||
      player == s.description().keeper_player_id()) {
    player = previous_player;
  }
  if (player == s.description().keeper_player_id() ||
      player == s.description().keeper_player_id()) {
    return;
  }
  for (const auto &block : m.blocks()) {
    for (const auto &action : get_actions_in_block(
        s.description().moves_description(),
        block.id())) {
      if (action->type() == ActionType::kOffType) {
        auto *off_action = dynamic_cast<const actions::Off *>(action);
        token_id_t piece = off_action->piece();
        (*result)[piece][player]++;
      }
    }
  }
}

static std::vector<std::vector<size_t> >
pieces_player_usages(const GameDescription &gd, size_t iters = 10) {
  std::vector<std::vector<size_t> > result(gd.resolver().NamesCount(),
                                           std::vector<size_t>(
                                               gd.resolver().NamesCount(), 0));
  SearchContext context;
  token_id_t previous_player = gd.keeper_player_id();
  for (size_t i = 0; i < iters; i++) {
    GameState s(gd);
    auto moves = s.FindMoves(&context);
    for (const auto &move : moves)
      collect_move_usages(previous_player, s, move, &result);
    while (!moves.empty()) {
      previous_player = s.player();
      s.MakeMove(moves[rand() % moves.size()]);
      moves = s.FindMoves(&context);
      for (const auto &move : moves)
        collect_move_usages(previous_player, s, move, &result);
    }
  }
  return result;
}

static PerftResult
perft(SearchContext *context, GameState *state, size_t depth) {
  if (depth == 0)
    return {1, 1};

  std::vector<Move> moves;
  size_t new_depth = depth - 1;
  if (state->player() ==
      state->description().keeper_player_id()) {
    moves = state->FindFirstMove(context);
    new_depth = depth;
  } else {
    moves = state->FindMoves(context);
  }
  size_t leaf_count = 0;
  size_t node_count = 1;
  for (const auto &move : moves) {
    auto revert_info = state->MakeRevertibleMove(move);
    auto rec_res = perft(context, state, new_depth);
    leaf_count += rec_res.leaf_count;
    node_count += rec_res.node_count;
    state->RevertMove(revert_info);
  }
  return {leaf_count, node_count};
}

static size_t estimate_depth(const GameDescription &gd, double secs) {
  SearchContext c;
  GameState state(gd);
  size_t depth = 0;

  double duration = 0;
  do {
    depth++;
    auto begin = std::chrono::system_clock::now();
    volatile auto result = perft(&c, &state, depth);
    auto end = std::chrono::system_clock::now();
    duration = std::chrono::duration<double>(end - begin).count();
  } while (duration < secs);
  return depth - 1;
}

/* TODO(???): make easy interface for move selectors and write more of them.*/
class MaterialHeuristicSelector {
  token_id_t player;
  std::vector<std::vector<double> > probabilities;
  SearchContext c;
  size_t alpha_beta_depth;

  void
  calculate_owner_probability(const GameDescription &gd, size_t iters = 10) {
    auto pieces_player_uses = pieces_player_usages(gd, iters);
    for (auto piece : gd.declarations().pieces_ids()) {
      size_t all_uses = 0;
      for (auto player : gd.declarations().players_ids()) {
        all_uses += pieces_player_uses[piece][player];
      }
      for (auto player : gd.declarations().players_ids()) {
        probabilities[player][piece] =
            static_cast<double>(pieces_player_uses[piece][player]) / all_uses;
        probabilities[player][piece] = std::max(0.0,
                                                probabilities[player][piece]);
      }
    }
  }

  double heu(token_id_t player,
             GameState *s,
             const std::vector<std::vector<double> > &pieces_own_probability) {
    double result = 0;
    for (const auto &piece : s->description().declarations().pieces_ids()) {
      result += s->Value(piece) * pieces_own_probability[player][piece];
    }
    return result;
  }

  double alphabeta(GameState *s, size_t depth, double alpha, double beta) {
    if (depth == 0)
      return heu(player, s, probabilities);
    auto moves = s->FindMoves(&c);
    if (moves.empty())
      return heu(player, s, probabilities);
    if (player == s->player()) {
      double value = std::numeric_limits<double>::min();
      for (auto &move : moves) {
        auto r = s->MakeRevertibleMove(move);
        value = std::max(value, alphabeta(s, depth - 1, alpha, beta));
        alpha = std::max(alpha, value);
        s->RevertMove(r);
        if (beta <= alpha)
          break;
      }
      return value;
    } else {
      double value = std::numeric_limits<double>::max();
      for (auto &move : moves) {
        auto r = s->MakeRevertibleMove(move);
        value = std::min(value, alphabeta(s, depth - 1, alpha, beta));
        beta = std::min(beta, value);
        s->RevertMove(r);
        if (beta <= alpha)
          break;
      }
      return value;
    }
  }


public:
  MaterialHeuristicSelector(const GameDescription &d, token_id_t player,
                            size_t alpha_beta_depth)
      : player(player),
        probabilities(d.resolver().NamesCount(),
                      std::vector<double>(d.resolver().NamesCount())),
        alpha_beta_depth(alpha_beta_depth) {
    calculate_owner_probability(d);
  }

  Move select(GameState *s, const std::vector<Move> &moves) {
    Move best;
    double best_value = std::numeric_limits<double>::min();
    for (const auto &move : moves) {
      auto r = s->MakeRevertibleMove(move);
      double value = alphabeta(s, alpha_beta_depth,
                               -std::numeric_limits<double>::min(),
                               std::numeric_limits<double>::max());
      s->RevertMove(r);
      if (value > best_value) {
        best_value = value;
        best = move;
      }
    }
    return best;
  }

  const std::vector<std::vector<double>> &get_probabilities() const {
    return probabilities;
  }
};

int main(int argc, char *argv[]) {
  /* TODO(???): Make proper program options with boost. */
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <host> <port>\n";
    return 1;
  }

  try {
    rbg_parser::messages_container msg;
    SynchronousClient client(argv[argc - 2], argv[argc - 1]);
    std::stringstream buffer(client.description());

    std::unique_ptr<rbg_parser::parsed_game> pg;
    try {
      std::vector<rbg_parser::token> result = tokenize(buffer.str(), msg);
      rbg_parser::game_items g = input_tokens(result, msg);
      pg = std::unique_ptr<rbg_parser::parsed_game>(
          new rbg_parser::parsed_game(g.parse_game(msg)));
    } catch (rbg_parser::message &m) {
      std::cout << "Game description is bad. Here is the error: " << "\n";
      std::cout << m.as_error() << std::endl;
      return 1;
    }
    GameDescription gd(CreateDescription(*pg));

    SearchContext context;
    GameState state(gd);

    token_id_t assigned_player = gd.resolver().Id(client.player());

    size_t depth = estimate_depth(gd, 1.0);
    std::cout << "Will count minmax to depth: " << depth << std::endl;
    MaterialHeuristicSelector selector(gd, assigned_player, depth);

    std::cout << "Ownerships: " << std::endl;
    std::cout << std::setw(5) << " " << " ";
    for (const auto piece : gd.declarations().pieces_ids()) {
      std::cout << std::setw(5) << gd.resolver().Name(piece).substr(0, 5)
                << " ";
    }
    std::cout << "\n";
    for (const auto player : gd.declarations().players_ids()) {
      std::cout << std::setw(5) << gd.resolver().Name(player).substr(0, 5)
                << " ";
      for (const auto piece : gd.declarations().pieces_ids()) {
        std::cout << std::setw(5) << std::setprecision(2)
                  << selector.get_probabilities()[player][piece] << " ";
      }
      std::cout << std::endl;
    }

    std::cout << "I am player: " << gd.resolver().Name(assigned_player)
              << std::endl;

    auto moves = state.FindMoves(&context);
    while (!moves.empty()) {
      if (state.player() == assigned_player) {
        auto move = selector.select(&state, moves);
        state.MakeMove(move);
        client.Write(move);
        moves = state.FindMoves(&context);
      } else {
        auto move = client.Read();
        state.MakeMove(move);
        moves = state.FindMoves(&context);
      }
    }
  }
  catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
