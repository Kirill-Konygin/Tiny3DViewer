#include "Window.h"
#include <iostream>

int main()
{
    Window window("Tiny3DViewer");
    
  
    while (!window.shouldClose())
    {
        window.clear();
        window.update();
    }
    
    return 0;
}
