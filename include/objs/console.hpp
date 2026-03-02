#ifndef OBJS_CONSOLE_HPP
#define OBJS_CONSOLE_HPP

#include "ui/input.hpp"
#include <vector>

// Commands

#define VArgs std::vector<std::string>
#define ArgsList struct Console &console, const std::string &original, const VArgs &args, struct Map &map, struct Player &player, struct Inventory &inventory

void c_help(ArgsList);
void c_tp(ArgsList);
void c_crds(ArgsList);
void c_clear(ArgsList);
void c_exit(ArgsList);
void c_quine(ArgsList);

// Console

enum class ConsoleColor: char {white, gray, yellow, red, green, blue, orange, purple, pink, count};

struct Console {
   void init();   
   void update(struct Map &map, struct Player &player, struct Inventory &inventory);
   void render();

   void output(const std::string &string, ConsoleColor color = ConsoleColor::white);
   void handleCommand(struct Map &map, struct Player &player, struct Inventory &inventory);

   // Members

   std::vector<std::string> text;
   std::vector<ConsoleColor> textColors;

   Input input;
   int scrollback = 0;
};

#endif
