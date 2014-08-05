#include "runes.hpp"
#include "runewords.hpp"
#include "TowerDispatcher.hpp"

#include "Tower.hpp"

#include <memory>
#include <iostream>

int main()
{
    tower_properties props;
    std::unique_ptr<aphrodite> aph = std::unique_ptr<aphrodite> (new aphrodite);
    std::unique_ptr<apollo>    apo = std::unique_ptr<apollo> (new apollo);
    props = combine_static_runes(aph.get(), apo.get());
    std::cout << "runeword: " << props << std::endl;

    std::unique_ptr<ares>      are = std::unique_ptr<ares> (new ares);
    std::unique_ptr<artemis>   art = std::unique_ptr<artemis> (new artemis);
    props = combine_static_runes(aph.get(), apo.get(), are.get(), art.get());
    std::cout << "non-runeword: " << props << std::endl;

/*
 *  test the dynamic multiple dispatch
 */
    //generate the type-erased objects
    std::unique_ptr<rune> r_aph = std::unique_ptr<aphrodite> (new aphrodite);
    std::unique_ptr<rune> r_apo = std::unique_ptr<apollo> (new apollo);
    std::unique_ptr<rune> r_are = std::unique_ptr<ares> (new ares);
    std::unique_ptr<rune> r_ath = std::unique_ptr<athena> (new athena);
    std::unique_ptr<rune> r_art = std::unique_ptr<artemis> (new artemis);
    std::unique_ptr<rune> r_dem = std::unique_ptr<demeter> (new demeter);

    tower_generator tower_gen;
    props = tower_gen.combine(r_aph.get(), r_apo.get());
    
	props = tower_gen.combine(r_ath.get(), r_art.get());
    Tower ta(std::move(props), 6);
	std::cout << ta << std::endl;
    
	props = tower_gen.combine(r_aph.get(), r_apo.get(), r_are.get());
    props = tower_gen.combine(r_ath.get(), r_art.get(), r_dem.get());
    props = tower_gen.combine(r_ath.get(), r_ath.get(), r_dem.get());
    Tower tz(std::move(props), 3);
	std::cout << tz << std::endl;

    return 0;
}
