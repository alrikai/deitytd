/* GameGUI.cpp -- part of the DietyTD Views subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "GameGUI.hpp"

GameGUI::GameGUI(Ogre::RenderWindow* render_window)
{
    gui_renderer = &CEGUI::OgreRenderer::bootstrapSystem(*render_window);
    gui_sys = CEGUI::System::getSingletonPtr();

    //can be: {Standard, Errors, Informative, Insane}
    CEGUI::Logger::getSingleton().setLoggingLevel(CEGUI::Standard);

    //names set in resources.cfg
    CEGUI::ImageManager::setImagesetDefaultResourceGroup("Imagesets");
    CEGUI::Font::setDefaultResourceGroup("Fonts");
    CEGUI::Scheme::setDefaultResourceGroup("Schemes");
    CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
    CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

	//sets up the GUI itself based on our layout 
	initialize();
}


GameGUI::~GameGUI()
{
    //TODO: do we destroy the singletons or anything?
}

void GameGUI::set_lives(int amount)
{
	static const std::string lives_string {"Lives: "};
	auto new_string = amount > 0 ? lives_string + std::to_string(amount) : "You're Dead!";
   	gui_window->getChild("numlives_edit")->setText(new_string);
}

void GameGUI::set_gold(int amount)
{
	static const std::string gold_string {"Gold: "};
    auto new_string = gold_string + std::to_string(amount);
	gui_window->getChild("numgold_edit")->setText(new_string);
}

void GameGUI::set_essence(int amount)
{
	static const std::string essence_string {"Essence: "};
    auto new_string = essence_string + std::to_string(amount);
	gui_window->getChild("numessence_edit")->setText(new_string);
}

void GameGUI::setup_animations()
{
    CEGUI::AnimationManager& animation_mgmt = CEGUI::AnimationManager::getSingleton();
    animation_mgmt.loadAnimationsFromXML("DTDLook.anims");

    CEGUI::Animation* right_spinning_anim = animation_mgmt.getAnimation("RotateRightLoop");
    CEGUI::Animation* left_spinning_anim = animation_mgmt.getAnimation("RotateLeftLoop");

    CEGUI::AnimationInstance* menu_anim = animation_mgmt.instantiateAnimation(right_spinning_anim);
    CEGUI::Window* menu_button = gui_window->getChild("menu_button");
    menu_anim->setTargetWindow(menu_button);
    menu_anim->start();

    CEGUI::AnimationInstance* quit_anim = animation_mgmt.instantiateAnimation(left_spinning_anim);
    CEGUI::Window* quit_button = gui_window->getChild("quit_button");
    quit_anim->setTargetWindow(quit_button);
    quit_anim->start();

}

void GameGUI::initialize()
{
    //this call sets up all the CEGUI system objects with Ogre3D defaults. Namely, 
    //This will create and initialise the following objects for you:
    //- CEGUI::OgreRenderer
    //- CEGUI::OgreResourceProvider
    //- CEGUI::OgreImageCodec
    //- CEGUI::System
    //Can get the created objects later via singletons, e.g. CEGUI::System::getSingletonPtr()

    CEGUI::SchemeManager::getSingleton().createFromFile("DTDLook.scheme");
    //CEGUI::SchemeManager::getSingleton().createFromFile("GameMenu.scheme");
    //CEGUI::SchemeManager::getSingleton().createFromFile("VanillaCommonDialogs.scheme");
    //CEGUI::SchemeManager::getSingleton().createFromFile("VanillaSkin.scheme");
    //CEGUI::SchemeManager::getSingleton().createFromFile("Generic.scheme");

    CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("DTDLook/MouseArrow");

#if 1    
    //TODO: when we make our layout file, we'll load it here...
    gui_window = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("DTDMainUI.layout"); 
 
    //TODO: this should spawn the main menu
    auto menu_button_clicked = [](const CEGUI::EventArgs &e)
    {
        std::cout << "NOTE: menu button was clicked, but nothing will happen" << std::endl;
        return true;
    };

    //TODO: should move this out to another function that'll trigger the shutdown sequence
    auto quit_button_clicked = [](const CEGUI::EventArgs &e)
    {
        std::cout << "NOTE: quit button was clicked, but nothing will happen" << std::endl;
        return true;
    };
    /*
    gui_window->getChild("quit_button")->subscribeEvent(CEGUI::PushButton::EventClicked, 
            CEGUI::Event::Subscriber(&menu_button_clicked));
    gui_window->getChild("quit_button")->subscribeEvent(CEGUI::PushButton::EventMouseButtonDown, 
            CEGUI::Event::Subscriber(&menu_button_clicked));
    */

    gui_window->getChild("quit_button")->subscribeEvent(CEGUI::PushButton::EventClicked, 
            CEGUI::Event::Subscriber(&quit_button_clicked));
    gui_window->getChild("menu_button")->subscribeEvent(CEGUI::PushButton::EventClicked, 
            CEGUI::Event::Subscriber(&menu_button_clicked));



	gui_window->getChild("staticinfo_text")->getChild("stats_info")->setText("N/A");
	gui_window->getChild("staticinfo_text")->getChild("description_info")->setText("N/A");

    //gui_window->getChild("staticinfo_text")->getChild("portrait_info")->setProperty("Image", 

	//TODO: move these to somewhere else, such that when we start the level there will be some set of 
	//default values for these that will be used for the initialization of these values 
    set_lives(20);
    set_gold(0);
    set_essence(0);

#else
    //... but in the meantime...
    gui_window = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "TDGUI/Sheet");  

    //proof of concept for adding GUI elements
    CEGUI::Window *quit = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Button", "CEGUIDemo/QuitButton");
    quit->setText("Quit");
    quit->setSize(CEGUI::USize(CEGUI::UDim(0.15, 0), CEGUI::UDim(0.05, 0)));
    gui_window->addChild(quit);

#endif

    setup_animations();
    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(gui_window);
}



void GameGUI::display_information(const std::string& base_stats, const std::string& current_stats, const std::string& unit_info)
{
    //TODO: should colorize the current stats, maybre re-arrange the fields a bit?
	gui_window->getChild("staticinfo_text")->getChild("stats_info")->setText(base_stats);
	gui_window->getChild("staticinfo_text")->getChild("stats_info")->insertText(current_stats, base_stats.size()+1);
	gui_window->getChild("staticinfo_text")->getChild("description_info")->setText(unit_info);
}



