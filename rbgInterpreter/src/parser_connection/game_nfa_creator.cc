//
// Created by ??? on 12.01.18.
//

#include <algorithm>
#include "game_nfa_creator.h"
#include "arithmetic_creator.h"

extern unsigned int kShiftTableClearLength;

class IsModifierFn : public rbg_parser::abstract_dispatcher {
public:
  void dispatch(const rbg_parser::sum&) override { result_ = false; }
  void dispatch(const rbg_parser::concatenation&) override { result_ = false; }
  void dispatch(const rbg_parser::star_move&) override { result_ = false; }
  void dispatch(const rbg_parser::shift&) override { result_ = false; }
  void dispatch(const rbg_parser::ons&) override { result_ = false; }
  void dispatch(const rbg_parser::off&) override { result_ = false; }
  void dispatch(const rbg_parser::assignment&) override { result_ = false; }
  void dispatch(const rbg_parser::player_switch&) override { result_ = false; }
  void dispatch(const rbg_parser::keeper_switch&) override { result_ = false; }
  void dispatch(const rbg_parser::move_check&) override { result_ = false; }
  void dispatch(const rbg_parser::arithmetic_comparison&) override { result_ = false; }
  void dispatch(const rbg_parser::integer_arithmetic&) override { result_ = false; }
  void dispatch(const rbg_parser::variable_arithmetic&) override { result_ = false; }
  void dispatch(const rbg_parser::arithmetic_operation&) override { result_ = false; }
  bool result() const {return result_;}
private:
  bool result_;
};

bool IsModifier(const rbg_parser::game_move& m)
{
  IsModifierFn fn;
  m.accept(fn);
  return fn.result();
}

class ShiftTableCreator : public rbg_parser::abstract_dispatcher{
public:
  explicit ShiftTableCreator(const NameResolver& resolver, const GraphBoard& board, const EdgeResolver& edge_resolver)
      : resolver_(resolver), board_(board), edge_resolver_(edge_resolver),
        clear_length_(0), result_(board.size())
  {
    ResetResult();
  }

  uint clear_length() const
  {
    return clear_length_;
  }

  void ResetResult()
  {
    for(ssize_t i = 0; i < static_cast<ssize_t >(result_.size()); i++)
    {
      result_[i] = {i};
    }
    clear_length_ = 0;
    concat_ends_ = {};
    concat_results_ = {};
    concat_clear_lengths_ = {};
  }

  std::vector<std::unordered_set<vertex_t > > ExtractResult()
  {
    auto tmp = std::move(result_);
    result_ = std::vector<std::unordered_set<vertex_t> >(tmp.size());
    ResetResult();
    return tmp;
  }

  const std::vector< std::vector< std::unordered_set<vertex_t> > >& concat_results() const
  {
    return concat_results_;
  }

  const std::vector<unsigned int>& concat_clear_lengths() const
  {
    return concat_clear_lengths_;
  }

  const std::vector<unsigned int>& concat_begins() const
  {
    return concat_ends_;
  }

private:
  void dispatch(const rbg_parser::sum &sum) override {
    unsigned int result_clear_length = 0;
    std::vector < std::unordered_set< vertex_t > > tmp_result = std::vector<std::unordered_set<vertex_t> >(board_.size());
    for(auto& item : sum.get_content())
    {
      item->accept(*this);
      if(!clear_length_)
        return;
      result_clear_length = std::max(clear_length_+1, result_clear_length);
      auto rec_result = ExtractResult();
      for(vertex_t v = 0; v < static_cast<ssize_t >(rec_result.size()); v++)
      {
        tmp_result[v].insert(rec_result[v].begin(),rec_result[v].end());
      }
      ResetResult();
    }
    result_ = tmp_result;
    clear_length_ = result_clear_length;
  }

  void dispatch(const rbg_parser::concatenation &concat) override {
    unsigned int result_clear_length = 0;
    std::vector < std::unordered_set< vertex_t > > result_table = std::vector<std::unordered_set<vertex_t> >(board_.size());

    std::vector< std::vector < std::unordered_set<vertex_t> > > concat_tables;
    std::vector< unsigned int> result_concat_clear_lengths;
    std::vector< unsigned int> result_concat_begins;

    for(ssize_t j = 0; j < static_cast<ssize_t >(result_table.size()); j++)
    {
      result_table[j] = {j};
    }
    for(size_t i = 0; i < concat.get_content().size(); i++)
    {
      auto& item = concat.get_content()[i];
      item->accept(*this);
      if(!clear_length_)
      {
        if (i == 0) {
          ResetResult();
          continue;
        }
        concat_tables.push_back(std::move(result_table));
        result_concat_clear_lengths.push_back(result_clear_length);
        result_concat_begins.push_back(static_cast<unsigned int &&>(i));
        result_clear_length = 0;
        result_table = std::vector< std::unordered_set<vertex_t> >(board_.size());
        for(ssize_t j = 0; j < static_cast<ssize_t >(result_table.size()); j++)
        {
          result_table[j] = {j};
        }
        ResetResult();
        continue;
      }
      else if(result_clear_length == 0 && clear_length_ && i!=0)
      {
        concat_tables.push_back(std::move(result_table));
        result_concat_clear_lengths.push_back(result_clear_length);
        result_concat_begins.push_back(static_cast<unsigned int &&>(i));
        result_clear_length = 0;
        result_table = std::vector< std::unordered_set<vertex_t> >(board_.size());
        for(ssize_t j = 0; j < static_cast<ssize_t >(result_table.size()); j++)
        {
          result_table[j] = {j};
        }
      }
      result_clear_length += clear_length_;
      auto tmp_rec_result = ExtractResult();
      std::vector < std::unordered_set< vertex_t > > next_result(result_table.size());
      for(vertex_t v = 0; v < static_cast<ssize_t >(result_table.size()); v++)
      {
        for(auto neighbour : result_table[v])
        {
          if(neighbour != -1)
            next_result[v].insert(tmp_rec_result[neighbour].begin(),tmp_rec_result[neighbour].end());
        }
      }
      result_table = next_result;
      ResetResult();
    }
    concat_tables.push_back(result_table);
    result_concat_clear_lengths.push_back(result_clear_length);
    result_concat_begins.push_back(static_cast<unsigned int &&>(concat.get_content().size()));
    result_ = result_table;
    clear_length_ = *std::min_element(result_concat_clear_lengths.begin(),result_concat_clear_lengths.end());

    concat_results_ = concat_tables;
    concat_clear_lengths_ = result_concat_clear_lengths;
    concat_ends_ = result_concat_begins;
  }

  void dispatch(const rbg_parser::star_move& move) override {
    unsigned int tmp_clear_length = 0;
    std::vector < std::unordered_set< vertex_t > > tmp_result = std::vector<std::unordered_set<vertex_t> >(board_.size());
    for(ssize_t j = 0; j < static_cast<ssize_t >(tmp_result.size()); j++)
    {
      tmp_result[j] = {j};
    }

    move.get_content()->accept(*this);
    if(!clear_length_)
      return;
    tmp_clear_length += clear_length_;
    auto tmp_rec_result = ExtractResult();

    for(uint i = 0; i < board_.size(); i++)
    {
      for(vertex_t v = 0; v < static_cast<ssize_t >(tmp_result.size()); v++)
      {
        for(auto neighbour : tmp_result[v])
        {
          if(neighbour != -1)
            tmp_result[v].insert(tmp_rec_result[neighbour].begin(),tmp_rec_result[neighbour].end());
        }
      }
    }
    ResetResult();
    result_ = tmp_result;
    clear_length_ = tmp_clear_length;
  }

  void dispatch(const rbg_parser::shift & action) override {
    ResetResult();
    for(vertex_t v = 0; v < static_cast<ssize_t >(result_.size()); v++)
    {
      result_[v] = {board_.Next(v,edge_resolver_.Id(action.get_content().to_string()))};
    }
    clear_length_ = 1;
  }

  void dispatch(const rbg_parser::ons &) override { ResetResult(); }
  void dispatch(const rbg_parser::off &) override { ResetResult(); }
  void dispatch(const rbg_parser::assignment &) override { ResetResult();}
  void dispatch(const rbg_parser::player_switch &) override { ResetResult();}
  void dispatch(const rbg_parser::keeper_switch&) override { ResetResult();}
  void dispatch(const rbg_parser::arithmetic_comparison &) override { ResetResult();}
  void dispatch(const rbg_parser::move_check &move) override {
    unsigned int tmp_clear_length = 0;
    std::vector < std::unordered_set< vertex_t > > tmp_result = std::vector<std::unordered_set<vertex_t> >(board_.size());
    for(ssize_t j = 0; j < static_cast<ssize_t >(tmp_result.size()); j++)
    {
      tmp_result[j] = {j};
    }

    move.get_content()->accept(*this);
    if(!clear_length_)
      return;
    tmp_clear_length = clear_length_;
    auto tmp_rec_result = ExtractResult();

    if(!move.is_negated()) {
        for (vertex_t v = 0; v < static_cast<ssize_t >(tmp_result.size()); v++) {
          bool exists = false;
          for (auto neighbour : tmp_rec_result[v]) {
            if (neighbour != -1) {
              exists = true;
              tmp_result[v] = {v};
              break;
            }
          }
          if(!exists)
          {
            tmp_result[v] = {-1};
          }
        }
    }
    else {
        for (vertex_t v = 0; v < static_cast<ssize_t >(tmp_result.size()); v++) {
          bool exists = false;
          for (auto neighbour : tmp_rec_result[v]) {
            if (neighbour != -1) {
              exists = true;
              tmp_result[v] = {-1};
              break;
            }
          }
          if (!exists) {
            tmp_result[v] = {v};
          }
        }
    }

    ResetResult();
    result_ = tmp_result;
    clear_length_ = tmp_clear_length;

  }
  void dispatch(const rbg_parser::integer_arithmetic &) override { ResetResult();};
  void dispatch(const rbg_parser::variable_arithmetic &) override { ResetResult();};
  void dispatch(const rbg_parser::arithmetic_operation &) override { ResetResult();};
  const NameResolver& resolver_;
  const GraphBoard& board_;
  const EdgeResolver& edge_resolver_;

  unsigned int clear_length_;
  std::vector<std::unordered_set<vertex_t> > result_;

  std::vector< std::vector < std::unordered_set<vertex_t> > > concat_results_;
  std::vector< unsigned int> concat_clear_lengths_;
  std::vector< unsigned int> concat_ends_;
};


void GameNfaCreator::dispatch(const rbg_parser::sum &move) {
  ShiftTableCreator creator(resolver_, graph_board_, edge_resolver_);
  move.accept(creator);
  if(creator.clear_length() >= kShiftTableClearLength)
  {
    std::string move_identifier = move.to_rbg();
    if (used_actions_.find(move_identifier) == used_actions_.end()) {
      std::vector< std::vector<vertex_t > > table(graph_board_.size());
      auto result = creator.ExtractResult();
      for(vertex_t v = 0; v < static_cast<unsigned int>(graph_board_.size()); v++)
      {
        table[v] = std::vector<vertex_t >(result[v].begin(),result[v].end());
        std::sort(table[v].begin(),table[v].end());
      }
      std::unique_ptr<Action> action(
          new actions::ShiftTable(std::move(table)));
      used_actions_[move_identifier] = action.get();
      actions_.push_back(std::move(action));
    }
    fsm::state_id_t initial_id = NewInitial();
    fsm::state_id_t final_id = nfa_result_->NewState();
    (*nfa_result_)[initial_id].AddTransition(final_id,
                                             used_actions_[move_identifier]);
    nfa_result_->set_initial(initial_id);
    nfa_result_->set_final(final_id);

    last_final_ = final_id;
    return;
  }

  if (used_actions_.find("epsilon") == used_actions_.end()) {
    std::unique_ptr<Action> empty_action(new actions::Empty());
    used_actions_["epsilon"] = empty_action.get();
    actions_.push_back(std::move(empty_action));
  }

  fsm::state_id_t new_initial_id = NewInitial();
  fsm::state_id_t new_final_id = nfa_result_->NewState();

  for (const std::unique_ptr<rbg_parser::game_move> &child : move.get_content()) {
    child->accept(*this);
    (*nfa_result_)[new_initial_id].AddTransition(nfa_result_->initial(),
                                                 used_actions_["epsilon"]);
    (*nfa_result_)[nfa_result_->final()].AddTransition(new_final_id,
                                                       used_actions_["epsilon"]);
  }
  nfa_result_->set_initial(new_initial_id);
  nfa_result_->set_final(new_final_id);
  last_final_ = new_final_id;
}


void GameNfaCreator::dispatch(const rbg_parser::concatenation &move) {
  ShiftTableCreator creator(resolver_, graph_board_, edge_resolver_);
  move.accept(creator);
  if(creator.clear_length() >= kShiftTableClearLength)
  {
    std::string move_identifier = move.to_rbg();
    if (used_actions_.find(move_identifier) == used_actions_.end()) {
      std::vector< std::vector<vertex_t > > table(graph_board_.size());
      auto result = creator.ExtractResult();
      for(vertex_t v = 0; v < static_cast<unsigned int>(graph_board_.size()); v++)
      {
        table[v] = std::vector<vertex_t >(result[v].begin(),result[v].end());
        std::sort(table[v].begin(),table[v].end());
      }
      std::unique_ptr<Action> action(
          new actions::ShiftTable(std::move(table)));
      used_actions_[move_identifier] = action.get();
      actions_.push_back(std::move(action));
    }
    fsm::state_id_t initial_id = NewInitial();
    fsm::state_id_t final_id = nfa_result_->NewState();
    (*nfa_result_)[initial_id].AddTransition(final_id,
                                             used_actions_[move_identifier]);
    nfa_result_->set_initial(initial_id);
    nfa_result_->set_final(final_id);

    last_final_ = final_id;
    return;
  }

  unsigned int concat_pos = 0;
  unsigned int concat_table_pos = 0;

  fsm::state_id_t initial = 0;
  unsigned int shift_clear_length = creator.concat_clear_lengths()[concat_table_pos];
  if(shift_clear_length >= kShiftTableClearLength)
  {
    std::vector< std::vector<vertex_t > > table(graph_board_.size());
    auto result = creator.concat_results()[concat_table_pos];
    for(vertex_t v = 0; v < static_cast<unsigned int>(graph_board_.size()); v++)
    {
      table[v] = std::vector<vertex_t >(result[v].begin(),result[v].end());
      std::sort(table[v].begin(),table[v].end());
    }
    std::unique_ptr<Action> action(
        new actions::ShiftTable(std::move(table)));
    actions_.push_back(std::move(action));

    initial = NewInitial();
    fsm::state_id_t final_id = nfa_result_->NewState();
    (*nfa_result_)[initial].AddTransition(final_id, actions_.back().get());
    nfa_result_->set_initial(initial);
    nfa_result_->set_final(final_id);

    last_final_ = final_id;

    concat_pos = creator.concat_begins()[concat_table_pos];
    concat_table_pos++;
  }
  else {
    if(!IsModifier(*move.get_content()[concat_pos]))
      StopBlock();
    move.get_content()[concat_pos]->accept(*this);
    if(IsModifier(*move.get_content()[concat_pos]))
      StartBlock();
    initial = nfa_result_->initial();
    concat_pos++;
    concat_table_pos++;
  }

  for (; concat_pos < move.get_content().size(); concat_pos++) {
    const auto &child = move.get_content()[concat_pos];
    ReuseFinal();
    unsigned int shift_clear_length = creator.concat_clear_lengths()[concat_table_pos];
    if(shift_clear_length >= kShiftTableClearLength)
    {
      std::vector< std::vector<vertex_t > > table(graph_board_.size());
      auto result = creator.concat_results()[concat_table_pos];
      for(vertex_t v = 0; v < static_cast<unsigned int>(graph_board_.size()); v++)
      {
        table[v] = std::vector<vertex_t >(result[v].begin(),result[v].end());
        std::sort(table[v].begin(),table[v].end());
      }
      std::unique_ptr<Action> action(
          new actions::ShiftTable(std::move(table)));
      ReuseFinal();
      actions_.push_back(std::move(action));

      fsm::state_id_t initial_id = NewInitial();
      fsm::state_id_t final_id = nfa_result_->NewState();
      (*nfa_result_)[initial_id].AddTransition(final_id, actions_.back().get());
      nfa_result_->set_initial(initial_id);
      nfa_result_->set_final(final_id);

      last_final_ = final_id;

      concat_pos = creator.concat_begins()[concat_table_pos]-1;
      concat_table_pos++;
    }
    else {
      if(!IsModifier(*child))
        StopBlock();
      child->accept(*this);

      if(IsModifier(*child))
        StartBlock();
//      if((*nfa_result_)[nfa_result_->StateCount()-2].transitions().front().letter()->IsModifier())
//        StartBlock();
//      else
//        StopBlock();

      while(concat_table_pos < creator.concat_begins().size() && concat_pos + 1 >= creator.concat_begins()[concat_table_pos])
      {
        concat_table_pos++;
      }
    }
  }
  StopBlock();
  nfa_result_->set_initial(initial);
  last_final_ = nfa_result_->final();
}

void GameNfaCreator::dispatch(const rbg_parser::shift &move) {
  std::string move_identifier = move.to_rbg();
  if (used_actions_.find(move_identifier) == used_actions_.end()) {
    std::unique_ptr<Action> action(
        new actions::Shift(edge_resolver_.Id(move.get_content().to_string())));
    used_actions_[move_identifier] = action.get();
    actions_.push_back(std::move(action));
  }
  fsm::state_id_t initial_id = NewInitial();
  fsm::state_id_t final_id = nfa_result_->NewState();
  (*nfa_result_)[initial_id].AddTransition(final_id,
                                           used_actions_[move_identifier]);
  nfa_result_->set_initial(initial_id);
  nfa_result_->set_final(final_id);

  last_final_ = final_id;
}

void GameNfaCreator::dispatch(const rbg_parser::ons &move) {
  std::string move_identifier = move.to_rbg();
  if (used_actions_.find(move_identifier) == used_actions_.end()) {
    std::vector<bool> pieces(piece_id_threshold_, false);
    for (const auto &piece_token : move.get_legal_ons()) {
      token_id_t piece_id = resolver_.Id(piece_token.to_string());
      pieces[piece_id] = true;
    }
    std::unique_ptr<Action> action(new actions::On(std::move(pieces)));
    used_actions_[move_identifier] = action.get();
    actions_.push_back(std::move(action));
  }
  fsm::state_id_t initial_id = NewInitial();
  fsm::state_id_t final_id = nfa_result_->NewState();
  (*nfa_result_)[initial_id].AddTransition(final_id,
                                           used_actions_[move_identifier]);
  nfa_result_->set_initial(initial_id);
  nfa_result_->set_final(final_id);

  last_final_ = final_id;
}

void GameNfaCreator::dispatch(const rbg_parser::off &move) {
  fsm::state_id_t initial_id = NewInitial();
  fsm::state_id_t final_id = nfa_result_->NewState();
  RegisterModifier(initial_id);
  token_id_t piece_id = resolver_.Id(move.get_piece().to_string());
  std::unique_ptr<Action> letter(
      new actions::Off(piece_id, (unsigned int) blocks_states_.size() - 1));
  Action *letter_ptr = letter.get();
  actions_.push_back(std::move(letter));
  (*nfa_result_)[initial_id].AddTransition(final_id, letter_ptr);
  nfa_result_->set_initial(initial_id);
  nfa_result_->set_final(final_id);

  last_final_ = final_id;
}

void GameNfaCreator::RegisterModifier(fsm::state_id_t initial_id) {
  if (!block_started_ && register_modifiers_)
    blocks_states_.push_back(initial_id);
}

void GameNfaCreator::dispatch(const rbg_parser::move_check &move) {
  std::string move_identifier = move.to_rbg();
  if (used_actions_.find(move_identifier) == used_actions_.end()) {

    auto r = StartMovePattern();
    move.get_content()->accept(*this);
    std::unique_ptr<Action> action;
    if(move.is_negated())
    {
      action = std::unique_ptr<Action>(new actions::NegatedConditionCheck(move_pattern_count_++,ExtractNfa()));
    }
    else
    {
      action = std::unique_ptr<Action>(new actions::ConditionCheck(move_pattern_count_++,ExtractNfa()));
    }
    used_actions_[move_identifier] = action.get();
    actions_.push_back(std::move(action));
    StopMovePattern(std::move(r));
  }
  fsm::state_id_t initial_id = NewInitial();
  fsm::state_id_t final_id = nfa_result_->NewState();
  (*nfa_result_)[initial_id].AddTransition(final_id,
                                           used_actions_[move_identifier]);
  nfa_result_->set_initial(initial_id);
  nfa_result_->set_final(final_id);

  last_final_ = final_id;
}

void GameNfaCreator::StopBlock() { block_started_ = false; }

void GameNfaCreator::StartBlock() { block_started_ = true; }

void GameNfaCreator::dispatch(const rbg_parser::assignment &move) {
  fsm::state_id_t initial_id = NewInitial();
  fsm::state_id_t final_id = nfa_result_->NewState();
  RegisterModifier(initial_id);
  token_id_t variable_id = resolver_.Id(move.get_left_side().to_string());
  std::unique_ptr<Action> letter;
  letter = std::unique_ptr<Action>(
      new actions::Assignment(variable_id, CreateOperation(*move.get_right_side(),resolver_),
                              (unsigned int) blocks_states_.size() - 1));

  Action *letter_ptr = letter.get();
  actions_.push_back(std::move(letter));
  (*nfa_result_)[initial_id].AddTransition(final_id, letter_ptr);
  nfa_result_->set_initial(initial_id);
  nfa_result_->set_final(final_id);
  last_final_ = final_id;
}

void GameNfaCreator::dispatch(const rbg_parser::player_switch &move) {
  fsm::state_id_t initial_id = NewInitial();
  fsm::state_id_t final_id = nfa_result_->NewState();
  RegisterModifier(initial_id);
  std::unique_ptr<Action> letter;
  token_id_t player_id = resolver_.Id(move.get_player().to_string());
  letter = std::unique_ptr<Action>(new actions::PlayerSwitch(player_id,
                                                               (unsigned int) blocks_states_.size() -
                                                               1));
  Action *letter_ptr = letter.get();
  actions_.push_back(std::move(letter));
  (*nfa_result_)[initial_id].AddTransition(final_id, letter_ptr);
  nfa_result_->set_initial(initial_id);
  nfa_result_->set_final(final_id);

  last_final_ = final_id;
}

void GameNfaCreator::dispatch(const rbg_parser::keeper_switch &) {
  fsm::state_id_t initial_id = NewInitial();
  fsm::state_id_t final_id = nfa_result_->NewState();
  RegisterModifier(initial_id);
  std::unique_ptr<Action> letter;
  token_id_t player_id = resolver_.Id(">");
  letter = std::unique_ptr<Action>(new actions::PlayerSwitch(player_id,
                                                             (unsigned int) blocks_states_.size() -
                                                             1));
  Action *letter_ptr = letter.get();
  actions_.push_back(std::move(letter));
  (*nfa_result_)[initial_id].AddTransition(final_id, letter_ptr);
  nfa_result_->set_initial(initial_id);
  nfa_result_->set_final(final_id);

  last_final_ = final_id;
}

void GameNfaCreator::dispatch(const rbg_parser::star_move &move) {
  ShiftTableCreator creator(resolver_, graph_board_, edge_resolver_);
  move.accept(creator);
  if(creator.clear_length() >= kShiftTableClearLength)
  {
    std::string move_identifier = move.to_rbg();
    if (used_actions_.find(move_identifier) == used_actions_.end()) {
      std::vector< std::vector<vertex_t > > table(graph_board_.size());
      auto result = creator.ExtractResult();
      for(vertex_t v = 0; v < static_cast<unsigned int>(graph_board_.size()); v++)
      {
        table[v] = std::vector<vertex_t >(result[v].begin(),result[v].end());
        std::sort(table[v].begin(),table[v].end());
      }
      std::unique_ptr<Action> action(
          new actions::ShiftTable(std::move(table)));
      used_actions_[move_identifier] = action.get();
      actions_.push_back(std::move(action));
    }
    fsm::state_id_t initial_id = NewInitial();
    fsm::state_id_t final_id = nfa_result_->NewState();
    (*nfa_result_)[initial_id].AddTransition(final_id,
                                             used_actions_[move_identifier]);
    nfa_result_->set_initial(initial_id);
    nfa_result_->set_final(final_id);

    last_final_ = final_id;
    return;
  }

  if (used_actions_.find("epsilon") == used_actions_.end()) {
      std::unique_ptr<Action> empty_action(new actions::Empty());
      used_actions_["epsilon"] = empty_action.get();
      actions_.push_back(std::move(empty_action));
    }

  fsm::state_id_t new_initial_id = NewInitial();
  fsm::state_id_t new_final_id = nfa_result_->NewState();

  move.get_content()->accept(*this);

  (*nfa_result_)[new_initial_id].AddTransition(new_final_id,
                                               used_actions_["epsilon"]);
  (*nfa_result_)[new_initial_id].AddTransition(nfa_result_->initial(),
                                               used_actions_["epsilon"]);
  (*nfa_result_)[nfa_result_->final()].AddTransition(new_final_id,
                                                     used_actions_["epsilon"]);
  (*nfa_result_)[nfa_result_->final()].AddTransition(nfa_result_->initial(),
                                                     used_actions_["epsilon"]);

  nfa_result_->set_initial(new_initial_id);
  nfa_result_->set_final(new_final_id);
  last_final_ = new_final_id;
}

void GameNfaCreator::dispatch(const rbg_parser::arithmetic_comparison &comparison) {
  std::string move_identifier = comparison.to_rbg();

  if (used_actions_.find(move_identifier) == used_actions_.end()) {
    switch (comparison.get_kind_of_comparison()) {
      case rbg_parser::eq: {
        std::unique_ptr<Action> action(
            new actions::ArithmeticEqualComparison(CreateOperation(*comparison.get_left_side(), resolver_),
                                                   CreateOperation(*comparison.get_right_side(), resolver_)));
        used_actions_[move_identifier] = action.get();
        actions_.push_back(std::move(action));
        break;
      }
      case rbg_parser::neq: {
        std::unique_ptr<Action> action(
            new actions::ArithmeticNotEqualComparison(CreateOperation(*comparison.get_left_side(), resolver_),
                                                   CreateOperation(*comparison.get_right_side(), resolver_)));
        used_actions_[move_identifier] = action.get();
        actions_.push_back(std::move(action));
        break;
      }
      case rbg_parser::le: {
        std::unique_ptr<Action> action(
            new actions::ArithmeticLessComparison(CreateOperation(*comparison.get_left_side(), resolver_),
                                                   CreateOperation(*comparison.get_right_side(), resolver_)));
        used_actions_[move_identifier] = action.get();
        actions_.push_back(std::move(action));
        break;
      }
      case rbg_parser::leq: {
        std::unique_ptr<Action> action(
            new actions::ArithmeticLessEqualComparison(CreateOperation(*comparison.get_left_side(), resolver_),
                                                   CreateOperation(*comparison.get_right_side(), resolver_)));
        used_actions_[move_identifier] = action.get();
        actions_.push_back(std::move(action));
        break;
      }
      case rbg_parser::ge: {
        std::unique_ptr<Action> action(
            new actions::ArithmeticLessComparison(CreateOperation(*comparison.get_right_side(), resolver_),
                                                   CreateOperation(*comparison.get_left_side(), resolver_)));
        used_actions_[move_identifier] = action.get();
        actions_.push_back(std::move(action));
        break;
      }
      case rbg_parser::geq: {
        std::unique_ptr<Action> action(
            new actions::ArithmeticLessEqualComparison(CreateOperation(*comparison.get_right_side(), resolver_),
                                                   CreateOperation(*comparison.get_left_side(), resolver_)));
        used_actions_[move_identifier] = action.get();
        actions_.push_back(std::move(action));
        break;
      }
    }
  }
  fsm::state_id_t initial_id = NewInitial();
  fsm::state_id_t final_id = nfa_result_->NewState();
  (*nfa_result_)[initial_id].AddTransition(final_id,
                                           used_actions_[move_identifier]);
  nfa_result_->set_initial(initial_id);
  nfa_result_->set_final(final_id);

  last_final_ = final_id;
}
