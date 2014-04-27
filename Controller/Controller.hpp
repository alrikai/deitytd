#ifndef TD_CONTROLLER_HPP__
#define TD_CONTROLLER_HPP__

#include "InputListener.hpp"

#include <OIS/OIS.h>
#include <OGRE/Ogre.h>

#include <string>
#include <memory>



//listens for user input events
class Controller
{
public:
    Controller(Ogre::Root* root, Ogre::RenderWindow* ogre_window);
    ~Controller();

    void start();
    void window_resized(Ogre::RenderTarget* ogre_window);
private:
    void init();

    //not owned by the Controller
    Ogre::RenderWindow* window;

    std::unique_ptr<InputListener> listener;
    //OIS Input devices
    OIS::InputManager* ois_manager;
    OIS::Mouse* ois_mouse;
    OIS::Keyboard* ois_keyboard;
};

#endif
