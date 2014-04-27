#include <iostream>
#include <memory>
#include <chrono>
#include <string>

#include <opencv2/opencv.hpp>
#include <OGRE/Ogre.h>

#include "Views/OgreDisplay.hpp"
#include "Controller/Controller.hpp"

/*
 *  The View on its own is pretty dull -- a better test would be for combining the 
 *  Controller and the views
 */

int main()
{   
    OgreDisplay display;

    Controller controller (display.get_root(), display.get_render_window());
    
    display.start_display();

    return 0;
}


