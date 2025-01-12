# cpptui

> still under heavy development

## description

A minimal library for creating TUI applications.
Uses macro magic (not intentionally, but it's needed to reduce code size and overcome some of `c++` 's difficulties).

## features

-   minimal
-   zero dependencies
-   cross-platform (mostly?)
-   c++11
-   low-level
-   performance: TODO

---

-   colors
-   styles
-   raw mode
-   alternate screen
-   performant screen size query
-   custom function for resize handling, **_NOTE_**: you should probably lock `stdout`, stdin when you use this feature, also, **doesn't work on Windows**

### input

a pretty handy header with functions and types to improve reading input from `stdin`.

**supported input**:

-   basic symbols, such as `['~', ';', '*', ...]`, but not `['$', '€', ...]`
-   <kbd>Ctrl</kbd>`+alphabetical characters`
-   arrows
-   touchpad/mouse scroll
-   <kbd>Backspace</kbd>, <kbd>(Shift)Tab</kbd>, <kbd>Enter/Return</kbd>, <kbd>Delete</kbd>, <kbd>Page[Up,Down]</kbd>, <kbd>Home,End</kbd>, <kbd>Insert</kbd>,
-   <kbd>F[1,2,3,4]</kbd> **_NOTE_**: the other F-keys are damn hard to implement, might be done in the future though
-   basic characters, be it upper or lowercase
-   **_NOTE_**: special ones like: `['ö', 'ä', 'á', ...]` are safely ignored

### coordinates

`coords.hpp` provides easy management of coordinates

## usage

you could use [poac](https://github.com/poac-dev/poac):

```sh
poac new my-tui-app
cd my-tui-app
poac add "csboo/cpptui"
```

see [examples](./examples), but basically just

```c++
#include "tui.hpp"
#include <utility> // for std::pair

using namespace tui::input;

int main() {
    tui::init(false);                       // alternate buffer, raw mode, no cursor
    auto screen_size = tui::screen::size(); // initially set

    const tui::string msg = "Hello Bello, TUI!"; // stylable extension of `std::string`
    const tui::string note = "press q or Ctrl+C to quit";

    Input input;                                      // this will handle special stuff like arrows, ctrl+c, ...
    while (input != SpecKey::CtrlC && input != 'q') { // main loop
        // NOTE: use `.size()` before styling, as it'd be completely crazy after applying styles
        // `msg` will be in the middle of the screen
        tui::cursor::set_position(screen_size.first / 2 - 1, screen_size.second / 2 - (msg.size() / 2));
        std::cout << msg.blue().link("https://github.com/csboo/cpptui").bold().underline();

        // `msg` will be in the middle of the screen
        tui::cursor::set_position(screen_size.first / 2 + 1, screen_size.second / 2 - (note.size() / 2));
        std::cout << note.on_rgb(106, 150, 137).rgb(148, 105, 117).italic().dim();

        std::cout.flush();
        input = Input::read(); // read into an `Input`
    }

    tui::reset(); // restore to where we came from
    return 0;
}
```

## why

-   see [features](#features)
-   for fun

## non-goals

-   full mouse support
-   huge number of widgets
-   high level functions
-   thousands of more features: just what's necessary

## kinda alternatives

-   [crossterm](https://github.com/crossterm-rs/crossterm): it's in Rust and (therefore) also a lot better
-   [Ratatui](https://ratatui.rs): Rust and the name speak for itself, uses crossterm

-   [FTXUI](https://github.com/ArthurSonzogni/FTXUI): pretty awesome indeed
-   [imtui](https://github.com/ggerganov/imtui): this guy is a c++ wizard
-   [tvision](https://github.com/magiblot/tvision): not bad
-   [finalcut](https://github.com/gansm/finalcut): super
-   [TermOx](https://github.com/a-n-t-h-o-n-y/TermOx): astonishing
-   [AppCUI](https://github.com/gdt050579/AppCUI): wasn't built in a week for sure
-   [TUI](https://github.com/jmicjm/TUI): nice
