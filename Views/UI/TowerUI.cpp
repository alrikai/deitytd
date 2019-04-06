#include "TowerUI.hpp"

//helper function for clearing out the word combination UI panel
namespace detail {
void clear_combinepanel(CEGUI::HorizontalLayoutContainer* gui_wordcombine_layout)
{
    const std::string blankletter_imgname = "DTDLetters/BlankTile";
   
    auto word_panel = gui_wordcombine_layout->getChild("DTDWordCombinePanel")->getChild("WordDropperPanel");
    const int num_words = word_panel->getChildCount();
    for (int letter_slotidx = 0; letter_slotidx < num_words; letter_slotidx++) {
        std::string slot_id = "letter_" + std::to_string(letter_slotidx);
        auto target_letter_image = word_panel->getChild(slot_id)->getChild("ldragger")->getChild("limage");
        target_letter_image->setProperty("Image", blankletter_imgname);
    }
}

void draw_inventory(const PlayerInventory& inventory_snapshot, CEGUI::Window* gui_inventory_window)
{
    //std::cout << "Updating UI..." << std::endl;
    for (int row = 0; row < PlayerInventory::NUM_INVENTORY_ROWS; row++) {
        for (int col = 0; col < PlayerInventory::NUM_INVENTORY_COLS; col++) {

            int inventory_idx = row*PlayerInventory::NUM_INVENTORY_COLS+col;

            auto target_letter_info = inventory_snapshot.get_item(inventory_idx);
            if(target_letter_info.first) {
                //std::cout << "inventory[" << row << "][" << col << "] occupied -- " 
                //<< inventory_snapshot.inventory_data[row*PlayerInventory::NUM_INVENTORY_COLS+col].letter << std::endl;
                std::string slot_id = "slot_" + std::to_string(inventory_idx);
                auto target_inventory_slot = gui_inventory_window->getChild("InventoryPanel")->getChild(slot_id);

                auto slot_dragger = reinterpret_cast<CEGUI::DragContainer*>(target_inventory_slot->getChild("dragger"));
                //by default we don't want to have the backgrounds be draggable
                slot_dragger->setDraggingEnabled(true);

                //Q: can we just set the text? on the staticimage? or do we need to have an imageset of letters?
                auto target_letter = target_letter_info.second.letter;
                std::transform(target_letter.begin(), target_letter.end(), target_letter.begin(), ::toupper);
                std::string letter_tilename = "DTDLetters/" + target_letter;

                auto slot_simage = slot_dragger->getChild("image");
                slot_simage->setProperty("Image", letter_tilename); 
            }
        }
    }
}

} //namespace detail

void TowerUpgradeUI::initialize_wordcomboUI()
{
    gui_wordcombine_window = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("DTDWordCombineUI.layout"); 
    //gui_wordcombine_window->setParent(gui_window);
    gui_wordcombine_window->setVisible(false);

    gui_wordcombine_layout = reinterpret_cast<CEGUI::HorizontalLayoutContainer*>(gui_wordcombine_window->getChild("WordCombineBackground"));

    auto word_combine_window = gui_wordcombine_layout->getChild("DTDWordCombinePanel");
    word_combine_window->getChild("CombinedStatsEdit")->setText("");
    word_combine_window->getChild("CurrentStatsEdit")->setText("");

    word_combine_window->getChild("PreviewButton")->subscribeEvent(
            CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TowerUpgradeUI::wordcombine_previewbtn, this));
    word_combine_window->getChild("CombineButton")->subscribeEvent(
            CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TowerUpgradeUI::wordcombine_combinebtn, this));
    word_combine_window->getChild("ClearButton")->subscribeEvent(
            CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TowerUpgradeUI::wordcombine_clearbtn, this));
    word_combine_window->getChild("CancelButton")->subscribeEvent(
            CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TowerUpgradeUI::wordcombine_cancelbtn, this));


    gui_inventory_window = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("DTDInventory.layout");
    gui_inventory_window->setVisible(true); //(false);

    //NOTE: you can make this more robust by getting the position and size, then placing the inventory after it.
    //it works as it currently is though
    //auto wcombo_position = gui_wordcombine_layout->getPositionOfChild(word_combine_window);
    //std::cout << "word combine panel position: " << wcombo_position << std::endl;
    gui_wordcombine_layout->addChild(gui_inventory_window);
    gui_inventory_window->setPosition(CEGUI::UVector2(CEGUI::UDim(0.6,0),CEGUI::UDim(0,0)));

    constexpr int inventory_rows = PlayerInventory::NUM_INVENTORY_ROWS;
    constexpr int inventory_cols = PlayerInventory::NUM_INVENTORY_COLS;
    int inventory_idx = 0;
    for (int row = 0; row < inventory_rows; row++) {
        for (int col = 0; col < inventory_cols; col++) {
            std::string slot_id = "slot_" + std::to_string(inventory_idx);
            std::cout << "subscribing slot " << slot_id << std::endl;
            auto target_inventory_slot = gui_inventory_window->getChild("InventoryPanel")->getChild(slot_id);


            //for testing: enable the slot. Also note that you should have the background be non-draggable, and only enable dragging capabilities if 
            //there's an item in that inventory slot.
            auto slot_dragger = reinterpret_cast<CEGUI::DragContainer*>(target_inventory_slot->getChild("dragger"));
/*            
            slot_dragger->subscribeEvent(CEGUI::DragContainer::EventDragEnded,
                    CEGUI::Event::Subscriber(&TowerUpgradeUI::handle_inventory_item_dropped, this));
*/
            slot_dragger->subscribeEvent(CEGUI::DragContainer::EventDragStarted,
                    CEGUI::Event::Subscriber(&TowerUpgradeUI::handle_inventory_item_dragging, this));

            //by default we don't want to have the backgrounds be draggable
            slot_dragger->setDraggingEnabled(false);
            //NOTE: the ID's here will overlap with the IDs in the word combo slots. This shouldn't (?) be an issue, but we'll have to be aware of it..
            slot_dragger->setID(inventory_idx);
            inventory_idx += 1;
        }
    }

    //load the imageset for handling the letter tiles -- Q: do we even need to do this?
    //CEGUI::ImagesetManager::getSingleton().createImageset("DTDLetters.imageset");
    //letter_imgset = CEGUI::ImagesetManager::getSingleton().getImageset("DTDLetters");

    auto disable_letter_slot = [] (CEGUI::Window* target_letter_slot) {
        //This is to disable the word slot
        //NOTE: just call enable() on it to re-activate the window
        target_letter_slot->disable();
        target_letter_slot->setDragDropTarget(false);
    };

    //Initially, (i.e. when we first make this window), we just want to have EVERYTHING deactivated, 
    //since there will be no towers or anything. (and it wont even be visible)
    auto word_panel = gui_wordcombine_layout->getChild("DTDWordCombinePanel")->getChild("WordDropperPanel");
    for (int letter_slotidx = 0; letter_slotidx < MAX_NUM_LETTERS; letter_slotidx++) {
        std::string slot_id = "letter_" + std::to_string(letter_slotidx);
        auto target_letter_slot = word_panel->getChild(slot_id);

        //we never want to be able to drag FROM these slots (I think...)
        auto slot_dragger = reinterpret_cast<CEGUI::DragContainer*>(target_letter_slot->getChild("ldragger"));
        slot_dragger->setDraggingEnabled(false);
        slot_dragger->setID(letter_slotidx);

        slot_dragger->subscribeEvent(CEGUI::Window::EventDragDropItemDropped,
                CEGUI::Event::Subscriber(&TowerUpgradeUI::handle_letter_item_dropped, this));

        disable_letter_slot(target_letter_slot);
    }

    /* 
       auto inventory_slot_dragged = [](const CEGUI::EventArgs &e)
       {
       std::cout << "NOTE: inventory slot dragging..." << std::endl;
       return true;
       };
       target_inventory_slot->getChild("dragger")->subscribeEvent(CEGUI::DragContainer::EventDragStarted, 
       CEGUI::Event::Subscriber(&inventory_slot_dragged));
       */
}

void TowerUpgradeUI::activate_towerUI(uint32_t active_tID) {
  activetower_ID = active_tID;
  gui_window->getChild("TowerUpgradeWindow")->setVisible(true);
}

//Q: how best to handle the inventory updating? We would need to have some sort of 'previous' inventory state, and
//just change parts of it? Or just wholesale re-create the inventory each time?
void TowerUpgradeUI::update_inventory(const PlayerInventory* inventory)
{
    //NOTE: we don't want to call this function from another thread if the tower UI window is already open, 
    //as then we'll have a race condition on our hands... does this happen as-is?
    //

    auto window_visible = gui_wordcombine_window->isVisible();
    if(window_visible) {
        std::cout << "ERROR -- we should not update the inventory if the Tower UI window is open!" << std::endl;
    }

    //make a copy of the current inventory state
    inventory_snapshot = *inventory;
    //this version is to revert to in the event the user clears the word set 
    pristine_inventory_snapshot = *inventory;

    detail::clear_combinepanel(gui_wordcombine_layout);
    detail::draw_inventory(inventory_snapshot, gui_inventory_window);
}

//TODO: need to figure out a good way to manage the inventory. How to handle swapping items, 
//reorganizing them, and maintaining the state between the player inventory and how the user 
//re-arranges the items in the UI (i.e. how to keep track of both)
//--> maybe just have the inventory NOT be user modifiable?
bool TowerUpgradeUI::handle_inventory_item_dropped(const CEGUI::EventArgs& args)
{
    //window --> the drag container the user has released
    //dragDropItem --> the window being dropped onto?
    const CEGUI::DragDropEventArgs& dd_args = static_cast<const CEGUI::DragDropEventArgs&>(args);    
    std::cout << "@inventory dragged handler -- window #child: " << dd_args.window->getChildCount() << " id: " << dd_args.window->getID() << std::endl;
    std::cout << "@inventory dragged handler -- droppeditem #child: " << dd_args.dragDropItem->getChildCount() << " id: " << dd_args.dragDropItem->getID() << std::endl;

    if (!dd_args.window->getChildCount())
    {
        // add dragdrop item as child of target if target has no item already
        dd_args.window->addChild(dd_args.dragDropItem);
        // Now we must reset the item position from it's 'dropped' location,
        // since we're now a child of an entirely different window
        dd_args.dragDropItem->setPosition(
                CEGUI::UVector2(CEGUI::UDim(0.05f, 0),CEGUI::UDim(0.05f, 0)));
    }
    return true;
}

bool TowerUpgradeUI::handle_inventory_item_dragging(
    const CEGUI::EventArgs &args) {
  const CEGUI::DragDropEventArgs &dd_args =
      static_cast<const CEGUI::DragDropEventArgs &>(args);
  auto dragging_slot = dd_args.window;
  auto slot_id = dragging_slot->getName();

  std::cout << "Starting to drag slot " << slot_id << " -- "
            << dragging_slot->getID() << std::endl;

  return true;
}

bool TowerUpgradeUI::handle_letter_item_dropped(const CEGUI::EventArgs &args) {
  const CEGUI::DragDropEventArgs &dd_args =
      static_cast<const CEGUI::DragDropEventArgs &>(args);

    //the inventory dragger being dropped
    auto dragged_slot = dd_args.dragDropItem;
    //the letter slot being dropped on
    auto letter_slot = dd_args.window;
    auto dragged_idx = dragged_slot->getID();

    //Q: can we just set the text? on the staticimage? or do we need to have an imageset of letters?
    auto target_letter_info = inventory_snapshot.get_item(dragged_idx); 
    assert(target_letter_info.first);
    auto target_letter = target_letter_info.second.letter;
    std::transform(target_letter.begin(), target_letter.end(), target_letter.begin(), ::toupper);
    std::string letter_tilename = "DTDLetters/" + target_letter;

  // need to move the item from the inventory to the word slot
  auto letter_img = letter_slot->getChild("limage");
  letter_img->setProperty("Image", letter_tilename);

  // reset the inventory tile  -- note that we need to write this back to the
  // backend state, as otherwise this will get overwritten upon the next
  // inventory update
  std::string defaultinventory_tilename = "DTDLetters/InventoryTile";
  auto dragged_img = dragged_slot->getChild("image");
  dragged_img->setProperty("Image", defaultinventory_tilename);

    //also update the inventory state
    inventory_snapshot.remove_item(dragged_idx);

    //TODO: I need the ID of the letter element that the letter was dropped ONTO for this

    const auto letter_slot_id = letter_slot->getID();
    assert(letter_slot_id >= 0 && letter_slot_id < MAX_NUM_LETTERS);
    //mark down what letters are active
    word_letters[letter_slot_id] = target_letter; 
    word_letter_count++;

  return true;
}

bool TowerUpgradeUI::word_combination_evthandler(const CEGUI::EventArgs &e)
{
    auto tinfo = shared_gamestate_info->get_towerinfo(activetower_ID);
    std::string tower_info_str = tinfo.get_tower_info_string();

    this->gui_window->getChild("TowerUpgradeWindow")->setVisible(false);

    //populate the window with the information we currently have
    gui_wordcombine_layout->getChild("DTDWordCombinePanel")->getChild("CurrentStatsEdit")->setText(tower_info_str);
    gui_wordcombine_layout->getChild("DTDWordCombinePanel")->getChild("CombinedStatsEdit")->setText(tower_info_str);

    //TODO: load the tower image here as well... this will have to be a dynamic image, as we are 
    //procedurally generating the tower models. Should be something where we take a 2D snapshot
    //of the tower's 3D model when we generate it, and store it in a buffer. Then the first time
    //we display it, we need to make a Ogre/CEGUI texture and write the buffer data to it (and finally,
    //display it when we get to here)
    //...

    //activate the word combination window...
    gui_wordcombine_window->enable();
    gui_wordcombine_window->activate();
    //... and activate the player inventory window
    gui_inventory_window->enable();
    gui_inventory_window->activate();

    //populate the number of word slots for the menu
    auto word_panel = gui_wordcombine_layout->getChild("DTDWordCombinePanel")->getChild("WordDropperPanel");
    
    //reset any letter state
    for (int letter_idx = 0; letter_idx < MAX_NUM_LETTERS; letter_idx++) {
        word_letters[letter_idx] = "";
    }
    word_letter_count = 0;

    std::cout << "activating the first " << MAX_NUM_LETTERS << " word slots" << std::endl;

    //this is to enable the word slot again -- so we can drag letters to enabled slots
    auto enable_letter_slot = [] (CEGUI::Window* target_letter_slot) {
        target_letter_slot->enable();
        target_letter_slot->setDragDropTarget(true);
    };

    //NOTE: we will have ALL N letter slots present in the UI, but only M <= N slots will be active,
    //based on the tower itself (and maybe the player? i.e. upgrade to make higher #slots?)
    for (int letter_slotidx = 0; letter_slotidx < MAX_NUM_LETTERS; letter_slotidx++) {
        std::string slot_id = "letter_" + std::to_string(letter_slotidx);
        auto target_letter_slot = word_panel->getChild(slot_id);

        //we never want to be able to drag FROM these slots (I think...)
        //auto slot_dragger = reinterpret_cast<CEGUI::DragContainer*>(target_letter_slot->getChild("ldragger"));
        //slot_dragger->setDraggingEnabled(false);

        enable_letter_slot(target_letter_slot);
    }

    //NOTE: we assume that the inventory will have already been updated with the player state, 
    //since that happens on every iteration of the frontend (so we don't need to explicitly populate
    //the player inventory)

#if 0
    //NOTE: we want to have ~4 per panel view
    static constexpr int SLOTS_PER_PANEL = 4;
    auto panel_dims = word_panel->getWidth();
    const float wordslot_width = 400; //panel_dims.d_offset / SLOTS_PER_PANEL;

    //this should have always been destroyed BEFORE we reach here
    if(wordslot_layout) {
        std::cout << "ERROR -- check your logic, wordslot layout should have been destroyed at this point " << __FILE__ << __LINE__ << std::endl; 
    }
    wordslot_layout = static_cast<CEGUI::HorizontalLayoutContainer*>(CEGUI::WindowManager::getSingleton().createWindow("HorizontalLayoutContainer", "WordSlotLayout"));


    float running_slotpos = 0;
    //TODO: need to destroy these widgets when this menu is closed
    for (int slot_idx = 0; slot_idx < num_word_slots + 5; slot_idx++) {

        std::string word_slot_conttext = "Word_" + std::to_string(slot_idx) + "_Container";
        std::string word_slot_btntext = "Word_" + std::to_string(slot_idx) + "_Button";

        //CEGUI::DragContainer* word_dragslot = static_cast<CEGUI::DragContainer*>(CEGUI::WindowManager::getSingleton().createWindow("DragContainer", word_slot_conttext));

        //have the width such that we have 4 per panel view, taking the entire height 
        CEGUI::Window* word_slot_widget = CEGUI::WindowManager::getSingleton().createWindow("DTDLook/Button", word_slot_btntext);
        word_slot_widget->setSize(CEGUI::USize(CEGUI::UDim(0, wordslot_width), CEGUI::UDim(1, 0)));
        word_slot_widget->setPosition(CEGUI::UVector2(CEGUI::UDim(0, running_slotpos), CEGUI::UDim(0,0)));
        running_slotpos += wordslot_width;
        word_slot_widget->setText(word_slot_btntext);
        //word_dragslot->addChild(word_slot_widget);

        wordslot_layout->addChild(word_slot_widget);
        //wordslot_layout->addChild(word_dragslot);
        //session_word_slots.push_back(word_slot_widget);
    }
    //attach the layout to the scrollable panel 
    word_scrollpanel->addChild(wordslot_layout); 
    word_scrollpanel->initialiseComponents();

    //-------------------
    std::cout << "layout visible: " << wordslot_layout->isVisible() << " size: " << wordslot_layout->getSize() << std::endl; 

    for (int slot_idx = 0; slot_idx < num_word_slots; slot_idx++) {
        std::string child_id = "Word_" + std::to_string(slot_idx) + "_Button";
        std::cout << "word slot " << slot_idx << " visible: " <<  wordslot_layout->getChild(child_id)->isVisible() << " size: " << wordslot_layout->getChild(child_id)->getSize() << std::endl;
    }

    //-------------------
#endif

  gui_wordcombine_window->setVisible(true);
  // NOTE: need to switch back to the regular UI once this one is exitted
  CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(
      gui_wordcombine_window);

  // NOTE: also need to spawn the word combination window here...
  return true;
}

std::pair<bool, std::string> TowerUpgradeUI::combine_currentletters() {
  // concatenate the letters to form a word
  bool is_structurally_valid = true;
  std::string combine_word = "";
  for (int letter_idx = 0; letter_idx < word_letter_count; letter_idx++) {
    if (!word_letters[letter_idx].empty()) {
      combine_word += word_letters[letter_idx];
    } else {
      // TODO: do we throw up an error or something if there's a gap?
      //...
      std::cout
          << "ERROR -- this isn't a valid word because there's a space in it"
          << std::endl;
      is_structurally_valid = false;
    }
  }

  return std::make_pair(is_structurally_valid, combine_word);
}

bool TowerUpgradeUI::wordcombine_combinebtn(const CEGUI::EventArgs &e) {
  // see what letters we have active, and do the word combination logic

  // TODO: figure out how best to handle structurally invalid words (i.e. if
  // there are spaces) auto tinfo =
  // shared_gamestate_info->get_towerinfo(activetower_ID);
  bool is_structurally_valid = false;
  std::string combine_word = "";
  std::tie(is_structurally_valid, combine_word) = combine_currentletters();

  if (is_structurally_valid) {
    // see if the combined word is a valid one
    std::cout << "combining word: " << combine_word << std::endl;

    // TODO: need to invoke the combination stuff...
    auto tower_cmbmgmt = get_towercombiner();
    auto valid_word = tower_cmbmgmt.check_combination(combine_word);
    if (valid_word) {
      std::cout << "@COMBINE -- word " << combine_word << " is valid word"
                << std::endl;
      tower_properties cmb_stats =
          tower_cmbmgmt.make_wordcombination(combine_word);

      // TODO: need to take this tower_properties stat and generate a tower
      // modify event
      active_tower_mods.emplace_back(
          std::make_pair(activetower_ID, std::move(cmb_stats)));
    } else {
        //TODO: will we ever add the ability to do sentences? That would be pretty neat... in which case we
        //would need spaces as a character. For now though, I guess it's just an error
        std::cout << "@COMBINE -- word " << combine_word << " is NOT a valid word" << std::endl;
    }

    //TODO: we need to be able to write the changes to this inventory BACK to the player inventory
    std::cout << "TODO: write local inventory state changes back to the global player inventory" << std::endl;


    //NOTE: we should also close the UI
    gui_wordcombine_window->setVisible(false);

    //CEGUI::WindowManager::getSingleton().destroyWindow(wordslot_layout);
    //wordslot_layout = nullptr;
    //how to delete GUI widgets?
    for (auto wslot_widgetit : session_word_slots) {
    CEGUI::WindowManager::getSingleton().destroyWindow(wslot_widgetit);
    }
  } else {
    // TODO: will we ever add the ability to do sentences? That would be pretty
    // neat... in which case we would need spaces as a character. For now though,
    // I guess it's just an error
    std::cout << "@COMBINE -- word " << combine_word << " is NOT a valid word"
              << std::endl;
  }

  // NOTE: we should also close the UI
  gui_wordcombine_window->setVisible(false);

  // CEGUI::WindowManager::getSingleton().destroyWindow(wordslot_layout);
  // wordslot_layout = nullptr;
  /*
  //how to delete GUI widgets?
  for (auto wslot_widgetit : session_word_slots) {
  CEGUI::WindowManager::getSingleton().destroyWindow(wslot_widgetit);
  }
  session_word_slots.clear();
  */
  // TBH, no idea what the difference between disable and deactivate is here
  gui_wordcombine_window->disable();
  gui_wordcombine_window->deactivate();

  gui_inventory_window->disable();
  gui_inventory_window->deactivate();

  CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(
      gui_window);

  return true;
}

bool TowerUpgradeUI::wordcombine_previewbtn(const CEGUI::EventArgs &e) {
  std::cout << "previewing word combination...." << std::endl;

  bool is_structurally_valid = false;
  std::string combine_word = "";
  std::tie(is_structurally_valid, combine_word) = combine_currentletters();

  if (is_structurally_valid) {
    // see if the combined word is a valid one
    std::cout << "combining word: " << combine_word << std::endl;

    // TODO: need to invoke the combination stuff...
    auto tower_cmbmgmt = get_towercombiner();
    auto valid_word = tower_cmbmgmt.check_combination(combine_word);
    if (valid_word) {
      std::cout << "@COMBINE -- word " << combine_word << " is valid word"
                << std::endl;
      tower_properties cmb_stats =
          tower_cmbmgmt.make_wordcombination(combine_word);

      // these are placeholders... need to decide what to use for the name, how
      // to handle the tier, etc.
      const std::string tower_name{"PLACEHOLDER"};
      const int tier = 1337;

      std::stringstream tinfo_oss;
      tinfo_oss << tower_name << "\nTier: " << tier << " \nAttributes:\n"
                << cmb_stats << "\n";
      auto tower_info_str = tinfo_oss.str();
      gui_wordcombine_layout->getChild("DTDWordCombinePanel")
          ->getChild("CombinedStatsEdit")
          ->setText(tower_info_str);
    } else {
      std::cout << "@COMBINE -- word " << combine_word << " is NOT valid word"
                << std::endl;
    }
  } else {
    std::cout << "@COMBINE -- word " << combine_word
              << " is NOT a structurally valid word" << std::endl;
  }

  return true;
}


bool TowerUpgradeUI::wordcombine_clearbtn(const CEGUI::EventArgs &e)
{

  // TODO: need to clear the current letter selections, move the icons back to
  // the inventory, and reset any inventory state changes that were enacted

    detail::clear_combinepanel(gui_wordcombine_layout);

    //reset any letter state
    for (int letter_idx = 0; letter_idx < MAX_NUM_LETTERS; letter_idx++) {
        word_letters[letter_idx] = "";
    }
    word_letter_count = 0;
    //reset any local inventory changes
    inventory_snapshot = pristine_inventory_snapshot;

    std::cout << "clearing word combination...." << std::endl;
    detail::draw_inventory(inventory_snapshot, gui_inventory_window);

    return true;
}

bool TowerUpgradeUI::wordcombine_cancelbtn(const CEGUI::EventArgs &e)
{
    gui_wordcombine_window->setVisible(false);

    detail::clear_combinepanel(gui_wordcombine_layout);
    //reset any letter state
    for (int letter_idx = 0; letter_idx < MAX_NUM_LETTERS; letter_idx++) {
        word_letters[letter_idx] = "";
    }
    word_letter_count = 0;

    //CEGUI::WindowManager::getSingleton().destroyWindow(wordslot_layout);
    //wordslot_layout = nullptr;
    /*
    //how to delete GUI widgets?
    for (auto wslot_widgetit : session_word_slots) {
    CEGUI::WindowManager::getSingleton().destroyWindow(wslot_widgetit);
    }
    session_word_slots.clear();
    */  
    //TBH, no idea what the difference between disable and deactivate is here
    gui_wordcombine_window->disable();
    gui_wordcombine_window->deactivate();

    gui_inventory_window->disable();
    gui_inventory_window->deactivate();

    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(gui_window);
    return true;
}
