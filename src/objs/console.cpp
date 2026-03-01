#include "objs/console.hpp"
#include "objs/inventory.hpp"
#include "util/format.hpp"
#include "util/render.hpp"
#include <sstream>

void Console::init() {
   input.rectangle = {500.0f, GetScreenHeight() - 25.0f, 1000.0f, 50.0f};
   input.fallback = "'help' for a list of commands.";
   input.wrapinput = false;
   input.maxChars = 512;
}

void Console::update(Map &map, Player &player, Inventory &inventory) {
   bool wastyping = input.typing;
   outputDelay -= GetFrameTime();
   input.update();

   if (wastyping && !input.typing && IsKeyPressed(KEY_ENTER)) {
      handleCommand(map, player, inventory);
   }
   shouldRender = outputDelay > 0.0f;
}

void Console::render() {
   drawRect(input.rectangle, Fade(BLACK, 0.75));
   input.render();

   if (shouldRender) {
      drawRect({input.rectangle.x, input.rectangle.y - 250.0f, input.rectangle.width, input.rectangle.height + 250.0f}, Fade(BLACK, 0.75));
      drawText({input.rectangle.x, input.rectangle.y - 250.0f}, output.c_str(), 35);
   }
}

// Commands

void Console::handleCommand(Map &map, Player &player, Inventory &inventory) {
   std::stringstream s (input.text);
   std::string temp;
   Args args;

   while (std::getline(s, temp)) {
      args.push_back(temp);
   }

   if (args.empty()) {
      return;
   }

   if (args[0] == "help") {
      help(args, map, player, inventory);
   } else {
      output = "Invalid command. See 'help' for a list of commands.";
   }

   wrapText(output, input.rectangle.width, 25, 1.0f);
   input.text.clear();
   outputDelay = 10.0f;
}

void Console::help(const Args&, Map&, Player&, Inventory&) {
   output = "Test123";
}
