#include "InputListener.hpp"

bool InputListener::frameRenderingQueued (const Ogre::FrameEvent& frame_evt)
{
    mouse->capture();
    keyboard->capture();
    return true;
}

bool InputListener::frameStarted (const Ogre::FrameEvent& event) 
{
    //std::cout << "frame " << frame_count << " started" << std::endl;
    frame_count++;
    return true;
}

bool InputListener::frameEnded (const Ogre::FrameEvent& event)
{
    //std::cout << "frame " << frame_count << " ended" << std::endl;
    return true;
}

void InputListener::windowClosed (Ogre::RenderWindow* window)
{
    //do we need to do anything prior to the window being closed? -- this method is called 
    //when the user clicks the window "x" button, but hasn't actually been closed yet.
    //hence, this is the place to trigger any cleanup methods that require the renderwindow 
    //to still exist
 
    //...
    Ogre::Root::getSingletonPtr()->queueEndRendering();
}

bool InputListener::windowClosing(Ogre::RenderWindow* window)
{
    std::cout << "final window closing" << std::endl; 
    return true;
}
void InputListener::windowResized(Ogre::RenderWindow* window)
{
    window_resized(window);
}

void InputListener::window_resized(Ogre::RenderTarget* ogre_window)
{
    unsigned int width, height, depth; 
    ogre_window->getMetrics(width, height, depth);

    auto& mouse_state = mouse->getMouseState();
    mouse_state.width = width;
    mouse_state.height = height;    
}

bool InputListener::keyPressed (const OIS::KeyEvent& key_arg)
{
	std::cout << "key pressed" << std::endl;
    if (key_arg.key == OIS::KC_ESCAPE)
        std::cout << "ESCAPE key" << std::endl;
    return true;
}

bool InputListener::keyReleased (const OIS::KeyEvent& key_arg)
{
	std::cout << "key released" << std::endl;
	return true;
}

bool InputListener::mouseMoved (const OIS::MouseEvent& mouse_arg)
{
	std::cout << "mouse moved" << std::endl;
	return true;
}

bool InputListener::mousePressed (const OIS::MouseEvent& mouse_arg, OIS::MouseButtonID mouse_id)
{
	std::cout << "mouse pressed" << std::endl;
	return true;
}

bool InputListener::mouseReleased (const OIS::MouseEvent& mouse_arg, OIS::MouseButtonID mouse_id)
{
	std::cout << "mouse released" << std::endl;
	return true;
}

