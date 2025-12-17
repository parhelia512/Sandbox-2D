#ifndef UTIL_INPUT_HPP
#define UTIL_INPUT_HPP

void resetInput();

void resetMouseUIInput(int mouse);
void setMouseOnUI(int mouse);
bool isMousePressedUI(int mouse);

bool isMousePressedOutsideUI(int mouse);
bool isMouseDownOutsideUI(int mouse);

#endif
