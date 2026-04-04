#include "app/TerminalInput.hpp"

#include <poll.h>
#include <termios.h>
#include <unistd.h>

namespace rook::ui::app {

namespace {

class ScopedRawInput final {
 public:
  ScopedRawInput() {
    if (!isatty(STDIN_FILENO)) {
      return;
    }

    if (tcgetattr(STDIN_FILENO, &original_) != 0) {
      return;
    }

    termios raw = original_;
    raw.c_lflag &= static_cast<unsigned long>(~(ICANON | ECHO));
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) {
      active_ = true;
    }
  }

  ~ScopedRawInput() {
    if (active_) {
      tcsetattr(STDIN_FILENO, TCSANOW, &original_);
    }
  }

  [[nodiscard]] bool active() const { return active_; }

 private:
  termios original_{};
  bool active_ = false;
};

InputCommand map_character(char character) {
  switch (character) {
    case 'w':
    case 'W':
      return InputCommand::Up;
    case 's':
    case 'S':
      return InputCommand::Down;
    case 'a':
    case 'A':
      return InputCommand::Left;
    case 'd':
    case 'D':
      return InputCommand::Right;
    case '\n':
    case '\r':
    case ' ':
    case 'e':
    case 'E':
      return InputCommand::Confirm;
    case 'b':
    case 'B':
    case 127:
      return InputCommand::Back;
    case 'q':
    case 'Q':
      return InputCommand::Exit;
    default:
      return InputCommand::None;
  }
}

}  // namespace

InputCommand TerminalInput::read_command(std::optional<std::chrono::milliseconds> timeout) const {
  ScopedRawInput input;
  if (!input.active()) {
    return InputCommand::Exit;
  }

  if (timeout.has_value()) {
    pollfd descriptor{
        .fd = STDIN_FILENO,
        .events = POLLIN,
        .revents = 0,
    };
    const int result = ::poll(&descriptor, 1, static_cast<int>(timeout->count()));
    if (result == 0) {
      return InputCommand::Tick;
    }
    if (result < 0) {
      return InputCommand::Exit;
    }
  }

  char character = '\0';
  if (read(STDIN_FILENO, &character, 1) != 1) {
    return InputCommand::Exit;
  }

  if (character == 27) {
    char sequence[2] = {'\0', '\0'};
    const auto read_count = read(STDIN_FILENO, sequence, 2);
    if (read_count == 2 && sequence[0] == '[') {
      switch (sequence[1]) {
        case 'A':
          return InputCommand::Up;
        case 'B':
          return InputCommand::Down;
        case 'C':
          return InputCommand::Right;
        case 'D':
          return InputCommand::Left;
        default:
          return InputCommand::Exit;
      }
    }

    return InputCommand::Exit;
  }

  return map_character(character);
}

}  // namespace rook::ui::app
