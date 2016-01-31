/* GameGUI.hpp -- part of the DietyTD Views subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */


#ifndef TD_GAME_GUI_HPP
#define TD_GAME_GUI_HPP

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/Renderer.h>
#include <OIS/OIS.h>

#include <OGRE/Ogre.h>

#include "shared/common_information.hpp"

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
            //std::cout << "mouse GUI event handler" << std::endl;
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

    void register_shared_towerinfor(std::shared_ptr<GameInformation<CommonTowerInformation>> shared_info)
	{
        shared_tower_info = shared_info;
	}


	inline void update_gamestate_info(const GameStateInformation& info) 
	{
        set_lives(info.num_lives);
        set_gold(info.gold_amount);
        set_essence(info.essence_amount);
	}

	//called by the main frontend class in response to user input
    void handle_tower_upgrade(uint32_t active_tower_ID)
    {
        activetower_ID = active_tower_ID;

        static bool initialized = false;
        if(!initialized) {
            gui_window->getChild("TowerUpgradeWindow")->getChild("OpenWordCombineButton")->subscribeEvent(CEGUI::PushButton::EventClicked, 
                CEGUI::Event::Subscriber(&GameGUI::word_combination_evthandler, this));
            initialized = true;
        }
        //this will be invisible by default, and it'll become visible when a tower is clicked
        gui_window->getChild("TowerUpgradeWindow")->setVisible(true);
    }

    void display_information(const std::string& base_stats, const std::string& current_stats, const std::string& unit_info);

private:	
	void initialize_mainUI();
	void initialize_wordcomboUI();

    void setup_animations();
	void set_lives(int amount);
	void set_gold(int amount);
	void set_essence(int amount);

	//need tp have the various button handlers here
	bool wordcombine_combinebtn(const CEGUI::EventArgs &e);
	bool wordcombine_previewbtn(const CEGUI::EventArgs &e);
	bool wordcombine_clearbtn(const CEGUI::EventArgs &e);
    bool wordcombine_cancelbtn(const CEGUI::EventArgs &e);
    
	bool word_combination_evthandler(const CEGUI::EventArgs &e);

	CEGUI::OgreRenderer* gui_renderer;
    CEGUI::System* gui_sys;
    CEGUI::Window* gui_window;

	CEGUI::Window* gui_wordcombine_window;
    std::vector<CEGUI::Window*> session_word_slots;

    CEGUI::RenderTarget *gui_rendertarget;
    CEGUI::GUIContext *gui_context;

    std::shared_ptr<GameInformation<CommonTowerInformation>> shared_tower_info;
    uint32_t activetower_ID;
};




#endif
