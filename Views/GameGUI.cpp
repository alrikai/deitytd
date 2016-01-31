/* GameGUI.cpp -- part of the DietyTD Views subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "GameGUI.hpp"

GameGUI::GameGUI(Ogre::RenderWindow* render_window)
    : shared_tower_info (nullptr)
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
	initialize_mainUI();
    initialize_wordcomboUI();
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

void GameGUI::initialize_wordcomboUI()
{
	gui_wordcombine_window = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("DTDWordCombineUI.layout"); 
    //gui_wordcombine_window->setParent(gui_window);
	gui_wordcombine_window->setVisible(false);

	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CombinedStatsEdit")->setText("");
	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CurrentStatsEdit")->setText("");

	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("PreviewButton")->subscribeEvent(
			CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&GameGUI::wordcombine_previewbtn, this));
	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CombineButton")->subscribeEvent(
			CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&GameGUI::wordcombine_combinebtn, this));
	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("ClearButton")->subscribeEvent(
			CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&GameGUI::wordcombine_clearbtn, this));
	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CancelButton")->subscribeEvent(
			CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&GameGUI::wordcombine_cancelbtn, this));
}

void GameGUI::initialize_mainUI()
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

    gui_window->getChild("TowerUpgradeWindow")->setVisible(false);

    gui_window->getChild("quit_button")->subscribeEvent(CEGUI::PushButton::EventClicked, 
            CEGUI::Event::Subscriber(&quit_button_clicked));
    gui_window->getChild("menu_button")->subscribeEvent(CEGUI::PushButton::EventClicked, 
            CEGUI::Event::Subscriber(&menu_button_clicked));

	gui_window->getChild("staticinfo_text")->getChild("stats_info")->setText("");
	gui_window->getChild("staticinfo_text")->getChild("description_info")->setText("");

    //gui_window->getChild("staticinfo_text")->getChild("portrait_info")->setProperty("Image", 

	//TODO: move these to somewhere else, such that when we start the level there will be some set of 
	//default values for these that will be used for the initialization of these values 
    set_lives(20);
    set_gold(0);
    set_essence(0);

    setup_animations();
    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(gui_window);
}

bool GameGUI::word_combination_evthandler(const CEGUI::EventArgs &e)
{
	auto tinfo = shared_tower_info->get_towerinfo(activetower_ID);
	std::string tower_info_str = tinfo.get_tower_info_string();

	this->gui_window->getChild("TowerUpgradeWindow")->setVisible(false);

	//populate the window with the information we currently have
	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CurrentStatsEdit")->setText(tower_info_str);
	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CombinedStatsEdit")->setText(tower_info_str);

	//TODO: load the tower image here as well...
    //...
	
	gui_wordcombine_window->enable();
	gui_wordcombine_window->activate();

	//populate the number of word slots for the menu
	auto word_dragpanel = gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CombinePanel")->getChild("WordDragContainer");
	const int num_word_slots = tinfo.num_wordslots;

    //NOTE: we want to have ~4 per panel view
	static constexpr int SLOTS_PER_PANEL = 4;
	auto panel_dims = word_dragpanel->getWidth();
    const float wordslot_width = panel_dims.d_offset / SLOTS_PER_PANEL;

	//TODO: need to destroy these widgets when this menu is closed
	for (int slot_idx = 0; slot_idx < num_word_slots; slot_idx++) {
		std::string word_slot_text = "Word_" + std::to_string(slot_idx) + "_Button";
        CEGUI::Window* word_slot_widget = CEGUI::WindowManager::getSingleton().createWindow("DTDLook/Button", word_slot_text);
        word_dragpanel->addChild(word_slot_widget);

		word_slot_widget->setText(word_slot_text);
		//have the width such that we have 4 per panel view, taking the entire height 
        word_slot_widget->setSize(CEGUI::USize(CEGUI::UDim(0, wordslot_width), CEGUI::UDim(1, 0)));

		session_word_slots.push_back(word_slot_widget);
	}
	

	gui_wordcombine_window->setVisible(true);
	
	//NOTE: need to switch back to the regular UI once this one is exitted
	CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(gui_wordcombine_window);

	//NOTE: also need to spawn the word combination window here...
	return true;
}


bool GameGUI::wordcombine_combinebtn(const CEGUI::EventArgs &e)
{

	return true;
}

bool GameGUI::wordcombine_previewbtn(const CEGUI::EventArgs &e)
{

	return true;
}

bool GameGUI::wordcombine_clearbtn(const CEGUI::EventArgs &e)
{

	return true;
}

bool GameGUI::wordcombine_cancelbtn(const CEGUI::EventArgs &e)
{
	gui_wordcombine_window->setVisible(false);
	
	//how to delete GUI widgets?
	for (auto wslot_widgetit : session_word_slots) {
        CEGUI::WindowManager::getSingleton().destroyWindow(wslot_widgetit);
	}
    session_word_slots.clear();
    
	//TBH, no idea what the difference between disable and deactivate is here
    gui_wordcombine_window->disable();
    gui_wordcombine_window->deactivate();
    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(gui_window);
	return true;
}

void GameGUI::display_information(const std::string& base_stats, const std::string& current_stats, const std::string& unit_info)
{
    //TODO: should colorize the current stats, maybre re-arrange the fields a bit?
	gui_window->getChild("staticinfo_text")->getChild("stats_info")->setText(base_stats);
	gui_window->getChild("staticinfo_text")->getChild("stats_info")->insertText(current_stats, base_stats.size()+1);
	gui_window->getChild("staticinfo_text")->getChild("description_info")->setText(unit_info);
}



