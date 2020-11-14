#include "PlayerInventory.hpp"
#include <cassert>

bool PlayerInventory::add_item(InventoryMetadata data) {
    bool item_added = false;
    // update the backing data first...
    for (int idx = 0; idx < NUM_INVENTORY_SLOTS; idx++) {
      if (!inventory_occupied[idx]) {
        inventory_data[idx] = data;
        inventory_occupied[idx] = true;
        item_added = true;
        break;
      }
    }

    //... then update the GUI (if the item was added)
    // TODO: update the GUI --> NOTE: this might end up being done elsewhere in
    // the frontend actually...
    return item_added;
}

void PlayerInventory::swap_item(const int item_aidx, const int item_bidx) {
    using std::swap;
    assert(item_aidx >= 0 && item_aidx < NUM_INVENTORY_SLOTS);
    assert(item_bidx >= 0 && item_bidx < NUM_INVENTORY_SLOTS);

    swap(inventory_data[item_aidx], inventory_data[item_bidx]);
}

std::pair<bool, InventoryMetadata> PlayerInventory::get_item(const int index) const {
    assert(index >= 0 && index <= NUM_INVENTORY_SLOTS);
    if (inventory_occupied[index]) {
      return std::make_pair(true, inventory_data[index]);
    } else {
      return std::make_pair(false, InventoryMetadata());
    }
}

void PlayerInventory::remove_item(const int index) {
    assert(index >= 0 && index <= NUM_INVENTORY_SLOTS);

    // sanity check that we are removing something that does exist
    assert(inventory_occupied[index]);

    // just doing lazy removal, I guess (I think we can get away with this?)
    inventory_occupied[index] = false;
}


std::ostream& operator<<(std::ostream& stream, 
                     const InventoryMetadata& inventory) {
    stream << "{" << inventory.letter << "}";
	return stream;
}


std::ostream& operator<<(std::ostream& stream, 
                     const PlayerInventory& inventory) {
	for (int row = 0; row < PlayerInventory::NUM_INVENTORY_ROWS; row++) {
        for (int col = 0; col < PlayerInventory::NUM_INVENTORY_COLS; col++) {
			const int flat_idx = row * PlayerInventory::NUM_INVENTORY_COLS + col;
            stream << "[" << row << "][" << col << "]: ";
			if (inventory.inventory_occupied[flat_idx]) {
				stream << inventory.inventory_data[flat_idx];
			} else {
				stream << "N/A";
			}
			stream << "\n";
		}
	}
    return stream;
}

