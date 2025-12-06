#include "mngr/resource.hpp"
#include "util/format.hpp"

void wrapText(std::string &string, float maxWidth, float fontSize, float spacing) {
   Font &font = getFont("andy");

   auto wrap = [=]() -> bool {
      return MeasureTextEx(font, string.c_str(), fontSize, spacing).x > maxWidth;
   };

   if (!wrap()) {
      return;
   }

   std::string original = string;
   std::string_view split = original;
   std::stringstream result;

   while (wrap()) {
      size_t left = 0, right = split.size();
      std::string_view truncated;

      while (left < right) {
         size_t mid = (left + right) / 2;
         truncated = split.substr(0, mid);
         string = std::string(truncated) + "-";

         if (wrap()) {
            right = mid;
         } else {
            left = mid + 1;
         }
      }
      truncated = split.substr(0, left - 1);
      split = split.substr(left - 1);

      bool dash = std::isalpha(truncated.back()) && std::isalpha(split.front());
      result << truncated << (dash ? "-\n" : "\n");
      string = std::string(split);

      if (!wrap()) {
         result << split;
         break;
      }
   }
   string = result.str();
}
