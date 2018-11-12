//
// Created by ??? on 12.01.18.
//

#ifndef RBGGAMEMANAGER_AUTOMATON_H
#define RBGGAMEMANAGER_AUTOMATON_H

#include<vector>

namespace fsm {
  using state_id_t = std::size_t;

  template<typename Letter>
  class Transition {
  public:
    Transition(state_id_t target_id, const Letter &letter)
        : target_id_(target_id), transition_letter_(letter) {}

    state_id_t target() const {
      return target_id_;
    }

    const Letter &letter() const {
      return transition_letter_;
    }

  private:
    state_id_t target_id_;
    Letter transition_letter_;
  };

  template<typename Letter>
  class State {
  public:
    State() = default;

    const std::vector<Transition<Letter> > &transitions() const {
      return state_transitions_;
    }

    void AddTransition(state_id_t target_id, const Letter &letter) {
      state_transitions_.emplace_back(target_id, letter);
    }

  private:
    std::vector<Transition<Letter> > state_transitions_;
  };

  template<typename Letter>
  class Nfa {
  public:
    Nfa()
        : initial_state_id_(0),
          final_state_id_(0) {}

    State<Letter> &operator[](state_id_t id) {
      return states_[id];
    }

    const State<Letter> &operator[](state_id_t id) const {
      return states_[id];
    }

    state_id_t initial() const {
      return initial_state_id_;
    }

    state_id_t final() const {
      return final_state_id_;
    }

    state_id_t NewState() {
      states_.emplace_back();
      return states_.size() - 1;
    }

    void set_initial(state_id_t id) {
      initial_state_id_ = id;
    }

    void set_final(state_id_t id) {
      final_state_id_ = id;
    }

    std::size_t StateCount() const {
      return states_.size();
    }

  private:
    state_id_t initial_state_id_;
    state_id_t final_state_id_;
    std::vector<State<Letter> > states_;
  };
}


#endif //RBGGAMEMANAGER_AUTOMATON_H
