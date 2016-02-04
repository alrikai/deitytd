/* InputListener.hpp -- part of the DietyTD Controller subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */



#ifndef TD_CONTROLLER_INPUT_LISTENER_HPP__ 
#define TD_CONTROLLER_INPUT_LISTENER_HPP__

#include "ControllerUtil.hpp"

#include <OIS/OIS.h>
#include <OGRE/Ogre.h> 

#include <iostream>
#include <unordered_map>
#include <functional>

class InputListener : public OIS::KeyListener, public OIS::MouseListener, public Ogre::FrameListener, public Ogre::WindowEventListener
{
public:

    InputListener(OIS::Mouse* ois_mouse, OIS::Keyboard* ois_keyboard)
        : mouse(ois_mouse), keyboard(ois_keyboard), frame_count(0), 
          current_key(OIS::KeyCode::KC_UNASSIGNED), current_mouse(OIS::MouseButtonID::MB_Button7)
    {
        std::cout << "Made the Input Listener..." << std::endl;
        mouse_dragging = false;
        drag_pos_y = 0;
        drag_pos_x = 0;

        //default (empty) handlers
        gui_key_evthandler = [](OIS::KeyEvent key_arg, bool key_up){};
        gui_mouse_evthandler = [](OIS::MouseEvent, OIS::MouseButtonID mouse_id, int mouse_flag){};
    }

    //frame listeners 
    bool frameStarted (const Ogre::FrameEvent& event); 
    bool frameEnded (const Ogre::FrameEvent& event);
    bool frameRenderingQueued (const Ogre::FrameEvent& event);

    //windowevent listeners
    void windowClosed (Ogre::RenderWindow* window);
    bool windowClosing(Ogre::RenderWindow* window);
    void windowResized(Ogre::RenderWindow* window);

    //keyboard listeners
    bool keyPressed (const OIS::KeyEvent& key_arg);
    bool keyReleased(const OIS::KeyEvent& key_arg);

    //mouse listeners
    bool mouseMoved   (const OIS::MouseEvent& mouse_arg) override;
    bool mousePressed (const OIS::MouseEvent& mouse_arg, OIS::MouseButtonID mouse_id) override;
    bool mouseReleased(const OIS::MouseEvent& mouse_arg, OIS::MouseButtonID mouse_id) override;

    void window_resized(Ogre::RenderTarget* ogre_window);

    inline bool add_input_listener(std::string id, ControllerUtil::ControllerBufferType* buffer)
    {
        return input_listeners.insert(std::make_pair(id, buffer)).second;;
    }

    inline void register_gui_handler(std::function<void(OIS::KeyEvent, bool)>&& gui_keyhandler, std::function<void(OIS::MouseEvent, OIS::MouseButtonID, int)>&& gui_mousehandler)
    {
        gui_key_evthandler = std::move(gui_keyhandler);
        gui_mouse_evthandler = std::move(gui_mousehandler);
    }

private:
    std::unordered_map<std::string, ControllerUtil::ControllerBufferType*> input_listeners;

    // .... any state required? maybe for buffering inputs?
    OIS::Mouse* mouse;
    OIS::Keyboard* keyboard;
    int frame_count;

    bool mouse_dragging;
    int drag_pos_x;
    int drag_pos_y;

    //initialize it to be empty -- will be registered later after the GUI is set up
    std::function<void(OIS::KeyEvent key_arg, bool)> gui_key_evthandler;
    std::function<void(OIS::MouseEvent mouse_arg, OIS::MouseButtonID mouse_id, int)> gui_mouse_evthandler;

    //eventually we might want to support having multiple keys pressed (e.g. alt + tab or something)
//    std::vector<OIS::KeyEvent> current_keys;
    //the question is, what should the default states be for these?
    OIS::KeyCode current_key;
    OIS::MouseButtonID current_mouse;
};

#endif
