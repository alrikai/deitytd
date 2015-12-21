#ifndef TD_GAME_GUI_HPP
#define TD_GAME_GUI_HPP

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/Renderer.h>
#include <OIS/OIS.h>

#include <OGRE/Ogre.h>


/*
//TODO: consolidate this and the OgreDisplay so we only have 1 framelistener on the View side of things
class UIFrameListener : public Ogre::FrameListener, public Ogre::WindowEventListener
{
public:
    
    UIFrameListener (Ogre::RenderWindow* rwin, Ogre::Camera* cam, CEGUI::Renderer* renderer)
         
    {}

private:    
    CEGUI::Renderer* gui_renderer;

};
*/

//this will hold all the information necessary for updating the passive game information
struct GameStateInformation
{
	int num_lives;
	int gold_amount;
	int essence_amount;

	//TODO: anything else around here?
};

/*
 * NOTE: this has to be initialized after the OgreDisplay is initialized (since CEGUI needs an Ogre 
 *       RenderWindow + general setup stuff to get started)
 */
class GameGUI
{
public:
    GameGUI(Ogre::RenderWindow* rwindow);
	~GameGUI();

	//Q: how involved do we want the inputs to be? i.e. I could register GUI handlers for key down
    //and key up events -- at the moment I'm just doing key down, but that doesn't seem sufficient 
    //in the long run. 
    template <class Controller>
	void register_controller(Controller* controller)
	{
        //this is where we set up the GUI event handlers
        std::function<void(OIS::KeyEvent, bool)> gui_keyhandler = [](OIS::KeyEvent key_arg, bool key_pressed)
        {
            CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
            if(key_pressed) {
                context.injectKeyDown((CEGUI::Key::Scan)key_arg.key);
                context.injectChar((CEGUI::Key::Scan)key_arg.text);
            } else {
                context.injectKeyUp((CEGUI::Key::Scan)key_arg.key);
            }
        };

        auto cvt_mouseevts = [](OIS::MouseButtonID button_id) {
            switch (button_id)
            {
                case OIS::MB_Left:
                    return CEGUI::LeftButton;
                case OIS::MB_Right:
                    return CEGUI::RightButton;
                case OIS::MB_Middle:
                    return CEGUI::MiddleButton;
                default:
                    return CEGUI::LeftButton;
            }
        };

        //mouse_state:
        //  0: clicked, 1: released, 2: neither (i.e. just movement)
        std::function<void(OIS::MouseEvent, OIS::MouseButtonID, bool)> gui_mousehandler = [cvt_mouseevts](OIS::MouseEvent mouse_evt, OIS::MouseButtonID mouse_id, int mouse_state)
        {
            std::cout << "mouse GUI event handler" << std::endl;
            auto& context = CEGUI::System::getSingleton().getDefaultGUIContext();
            //context.injectMouseMove(mouse_evt.state.X.rel, mouse_evt.state.Y.rel);
            context.injectMousePosition(mouse_evt.state.X.abs, mouse_evt.state.Y.abs);

            // Scroll wheel.
            if (mouse_evt.state.Z.rel) {
                context.injectMouseWheelChange(mouse_evt.state.Z.rel / 120.0f);
            }

            if(mouse_state == 0) {
               context.injectMouseButtonDown (cvt_mouseevts(mouse_id));
            } else if (mouse_state == 1){
               context.injectMouseButtonUp (cvt_mouseevts(mouse_id));
            }
        }; 
        controller->register_gui_listener(std::move(gui_keyhandler), std::move(gui_mousehandler));
	}

	inline void update_gamestate_info(const GameStateInformation& info) 
	{
        set_lives(info.num_lives);
        set_gold(info.gold_amount);
        set_essence(info.essence_amount);
	}

private:	
	void initialize();
	void set_lives(int amount);
	void set_gold(int amount);
	void set_essence(int amount);

	CEGUI::OgreRenderer* gui_renderer;
    CEGUI::System* gui_sys;
    CEGUI::Window* gui_window;

    CEGUI::RenderTarget *gui_rendertarget;
    CEGUI::GUIContext *gui_context;
};




#endif
