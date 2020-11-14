/* Controller.hpp -- part of the DietyTD Controller subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_CONTROLLER_HPP
#define TD_CONTROLLER_HPP

#include "ControllerUtil.hpp"
#include "InputListener.hpp"

#include <OGRE/Ogre.h>
#include <OIS/OIS.h>

#include <memory>
#include <string>

// listens for user input events
class Controller {
public:
  typedef ControllerUtil::ControllerBufferType ControllerBufferType;

  Controller(Ogre::Root *root, Ogre::RenderWindow *ogre_window);
  ~Controller();

  void start();
  void window_resized(Ogre::RenderTarget *ogre_window);

  // for now: have clients register buffers with the controller, s.t. they'll
  // recieve all the events of that particular type. We may want more
  // fine-grained event filtering in the future, but start with this for now.
  // NOTE: what will we do for temporal information? i.e. if we need to know the
  // order of the events, how will we correlate the keyboard events and the mouse
  // events here? Do we need to supply timestamps for everything?

  bool register_input_listener(std::string id, ControllerBufferType *buffer);
  // this is sort of a hackish system, but it seems a lot of GUI libraries want
  // unfiltered access to the keypresses?
  inline void register_gui_listener(
      std::function<void(OIS::KeyEvent, bool)> &&gui_keyhandler,
      std::function<void(OIS::MouseEvent, OIS::MouseButtonID, int)>
          &&gui_mousehandler) {
    listener->register_gui_handler(std::move(gui_keyhandler),
                                   std::move(gui_mousehandler));
  }

private:
  void init();

  // not owned by the Controller
  Ogre::RenderWindow *window;

  std::unique_ptr<InputListener> listener;
  // OIS Input devices
  OIS::InputManager *ois_manager;
  OIS::Mouse *ois_mouse;
  OIS::Keyboard *ois_keyboard;
};

#endif
