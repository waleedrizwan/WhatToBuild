#include "data/items.h"

#include <algorithm>

namespace {

constexpr std::array<std::string_view, kNeedCount> NEED_KEYS = {
    "armor",     "mr",     "antiheal", "antishield", "armorpen", "magicpen",
    "antiburst", "antiCC", "antiAuto", "antiCrit",   "antiPoke",
};

}  // namespace

std::string_view needKey(Need n) { return NEED_KEYS[idx(n)]; }

const std::vector<ItemKnowledge>& itemKnowledge() {
  using enum Archetype;
  using enum Need;
  static const std::vector<ItemKnowledge> items = {
      // --- Marksman / crit ---
      {.name = "Infinity Edge", .fit = {{AdcCrit, 10}}},
      {.name = "Yun Tal Wildarrows", .fit = {{AdcCrit, 9}}},
      {.name = "The Collector", .fit = {{AdcCrit, 8}, {AdAssassin, 3}}},
      {.name = "Navori Flickerblade", .fit = {{AdcCrit, 7}}},
      {.name = "Phantom Dancer", .fit = {{AdcCrit, 7}, {AdcOnHit, 4}}},
      {.name = "Rapid Firecannon", .fit = {{AdcCrit, 6}}},
      {.name = "Runaan's Hurricane", .fit = {{AdcCrit, 6}, {AdcOnHit, 6}}},
      {.name = "Fiendhunter Bolts", .fit = {{AdcCrit, 6}}},
      {.name = "Hexoptics C44", .fit = {{AdcCrit, 6}}},
      {.name = "Stormrazor", .fit = {{AdcCrit, 6}}},
      {.name = "Essence Reaver", .fit = {{AdcCrit, 5}, {AdCaster, 7}}, .group = "spellblade"},
      {.name = "Lord Dominik's Regards", .fit = {{AdcCrit, 7}, {AdCaster, 4}}, .counters = {{Armorpen, 1}},
       .group = "lastwhisper"},
      {.name = "Mortal Reminder", .fit = {{AdcCrit, 5}, {AdcOnHit, 3}}, .counters = {{Antiheal, 1}, {Armorpen, 0.6}},
       .group = "lastwhisper"},
      {.name = "Bloodthirster", .fit = {{AdcCrit, 7}, {AdcOnHit, 4}}, .counters = {{Antiburst, 0.3}, {AntiPoke, 0.3}}},
      {.name = "Immortal Shieldbow", .fit = {{AdcCrit, 5}}, .counters = {{Antiburst, 0.9}}, .group = "lifeline"},
      {.name = "Guardian Angel",
       .fit = {{AdcCrit, 4}, {AdcOnHit, 4}, {AdCaster, 4}, {Bruiser, 4}, {AdAssassin, 3}},
       .counters = {{Antiburst, 0.8}, {Armor, 0.4}}},
      {.name = "Mercurial Scimitar", .fit = {{AdcCrit, 4}, {AdcOnHit, 4}}, .counters = {{AntiCC, 1}, {Mr, 0.6}},
       .group = "quicksilver"},
      {.name = "Kraken Slayer", .fit = {{AdcCrit, 7}, {AdcOnHit, 9}}, .counters = {{Armorpen, 0.4}}},
      {.name = "Blade of The Ruined King", .fit = {{AdcCrit, 5}, {AdcOnHit, 10}, {Bruiser, 4}},
       .counters = {{Armorpen, 0.8}}},
      {.name = "Guinsoo's Rageblade", .fit = {{AdcOnHit, 9}, {ApOnHit, 8}}},
      {.name = "Wit's End", .fit = {{AdcOnHit, 7}, {Bruiser, 3}, {ApOnHit, 4}}, .counters = {{Mr, 0.9}, {AntiCC, 0.2}}},
      {.name = "Terminus", .fit = {{AdcOnHit, 8}}, .counters = {{Armorpen, 0.6}, {Magicpen, 0.3}}},
      {.name = "Statikk Shiv", .fit = {{AdcOnHit, 4}, {AdcCrit, 3}}},

      // --- AD casters / lethality ---
      {.name = "Manamune", .fit = {{AdCaster, 10}}, .group = "manaflow"},
      {.name = "Trinity Force", .fit = {{AdCaster, 9}, {Bruiser, 8}}, .group = "spellblade"},
      {.name = "Serylda's Grudge", .fit = {{AdCaster, 8}, {AdAssassin, 7}, {Bruiser, 4}}, .counters = {{Armorpen, 0.7}}},
      {.name = "Black Cleaver", .fit = {{AdCaster, 6}, {Bruiser, 9}}, .counters = {{Armorpen, 0.7}}},
      {.name = "Spear of Shojin", .fit = {{AdCaster, 6}, {Bruiser, 7}}},
      {.name = "Eclipse", .fit = {{AdCaster, 7}, {AdAssassin, 7}, {Bruiser, 7}},
       .counters = {{Armorpen, 0.3}, {Antiburst, 0.3}}},
      {.name = "Axiom Arc", .fit = {{AdCaster, 6}, {AdAssassin, 7}}},
      {.name = "Youmuu's Ghostblade", .fit = {{AdCaster, 5}, {AdAssassin, 8}}},
      {.name = "Hubris", .fit = {{AdAssassin, 8}}},
      {.name = "Profane Hydra", .fit = {{AdAssassin, 9}, {Bruiser, 4}}, .group = "hydra"},
      {.name = "Voltaic Cyclosword", .fit = {{AdAssassin, 8}}},
      {.name = "Umbral Glaive", .fit = {{AdAssassin, 6}}},
      {.name = "Bastionbreaker", .fit = {{AdAssassin, 7}}},
      {.name = "Edge of Night", .fit = {{AdAssassin, 7}, {AdCaster, 5}, {Bruiser, 4}},
       .counters = {{AntiCC, 0.6}, {Antiburst, 0.5}}},
      {.name = "Serpent's Fang", .fit = {{AdAssassin, 6}}, .counters = {{Antishield, 1}}},
      {.name = "Death's Dance", .fit = {{AdCaster, 5}, {AdAssassin, 5}, {Bruiser, 8}},
       .counters = {{Armor, 0.7}, {Antiburst, 0.6}}},
      {.name = "Maw of Malmortius", .fit = {{AdCaster, 5}, {AdAssassin, 5}, {Bruiser, 6}},
       .counters = {{Mr, 0.9}, {Antiburst, 0.6}}, .group = "lifeline"},
      {.name = "Chempunk Chainsword", .fit = {{AdAssassin, 5}, {Bruiser, 6}, {AdCaster, 3}},
       .counters = {{Antiheal, 1}}},

      // --- Bruisers ---
      {.name = "Sterak's Gage", .fit = {{Bruiser, 9}}, .counters = {{Antiburst, 0.7}, {AntiCC, 0.3}},
       .group = "lifeline"},
      {.name = "Sundered Sky", .fit = {{Bruiser, 8}}},
      {.name = "Stridebreaker", .fit = {{Bruiser, 7}}},
      {.name = "Hullbreaker", .fit = {{Bruiser, 6}}},
      {.name = "Ravenous Hydra", .fit = {{Bruiser, 7}}, .group = "hydra"},
      {.name = "Titanic Hydra", .fit = {{Bruiser, 6}, {Tank, 4}}, .group = "hydra"},
      {.name = "Overlord's Bloodmail", .fit = {{Bruiser, 7}}},
      {.name = "Experimental Hexplate", .fit = {{Bruiser, 6}}},
      {.name = "Endless Hunger", .fit = {{Bruiser, 6}, {AdAssassin, 4}}, .counters = {{AntiCC, 0.4}}},
      {.name = "Protoplasm Harness", .fit = {{Bruiser, 5}, {Tank, 6}}, .counters = {{Antiburst, 0.6}, {AntiCC, 0.3}},
       .group = "lifeline"},

      // --- Tank ---
      {.name = "Sunfire Aegis", .fit = {{Tank, 8}, {Bruiser, 3}}, .counters = {{Armor, 0.8}}},
      {.name = "Hollow Radiance", .fit = {{Tank, 7}, {Bruiser, 2}}, .counters = {{Mr, 0.8}}},
      {.name = "Heartsteel", .fit = {{Tank, 8}, {Bruiser, 3}}},
      {.name = "Jak'Sho, The Protean", .fit = {{Tank, 9}, {Bruiser, 5}}, .counters = {{Armor, 0.7}, {Mr, 0.7}}},
      {.name = "Unending Despair", .fit = {{Tank, 8}}, .counters = {{Armor, 0.8}}},
      {.name = "Thornmail", .fit = {{Tank, 6}, {Bruiser, 4}},
       .counters = {{Armor, 1}, {Antiheal, 1}, {AntiAuto, 0.6}}},
      {.name = "Randuin's Omen", .fit = {{Tank, 6}, {Bruiser, 4}},
       .counters = {{Armor, 1}, {AntiCrit, 1}, {AntiAuto, 0.5}}},
      {.name = "Frozen Heart", .fit = {{Tank, 6}, {Bruiser, 3}}, .counters = {{Armor, 1}, {AntiAuto, 1}}},
      {.name = "Dead Man's Plate", .fit = {{Tank, 6}, {Bruiser, 5}}, .counters = {{Armor, 0.8}}},
      {.name = "Iceborn Gauntlet", .fit = {{Tank, 6}, {Bruiser, 4}}, .counters = {{Armor, 0.8}}, .group = "spellblade"},
      {.name = "Warmog's Armor", .fit = {{Tank, 6}}, .counters = {{AntiPoke, 0.6}}},
      {.name = "Force of Nature", .fit = {{Tank, 6}, {Bruiser, 4}}, .counters = {{Mr, 1}}},
      {.name = "Spirit Visage", .fit = {{Tank, 6}, {Bruiser, 5}}, .counters = {{Mr, 0.9}}},
      {.name = "Kaenic Rookern", .fit = {{Tank, 7}, {Bruiser, 5}}, .counters = {{Mr, 1}, {AntiPoke, 1}}},
      {.name = "Abyssal Mask", .fit = {{Tank, 5}}, .counters = {{Mr, 0.8}}},
      {.name = "Locket of the Iron Solari", .fit = {{Tank, 6}, {Enchanter, 5}},
       .counters = {{Antiburst, 0.6}, {Mr, 0.4}, {Armor, 0.3}}, .supportOnly = true},
      {.name = "Knight's Vow", .fit = {{Tank, 5}}, .counters = {{Armor, 0.5}}, .supportOnly = true},
      {.name = "Zeke's Convergence", .fit = {{Tank, 4}}, .counters = {{Armor, 0.4}, {Mr, 0.4}}, .supportOnly = true},
      {.name = "Bandlepipes", .fit = {{Tank, 4}, {Enchanter, 4}}, .counters = {{Armor, 0.3}, {Mr, 0.3}},
       .supportOnly = true},

      // --- Mages ---
      {.name = "Luden's Echo", .fit = {{ApMage, 9}, {ApAssassin, 6}}},
      {.name = "Rabadon's Deathcap", .fit = {{ApMage, 9}, {ApBattlemage, 7}, {ApAssassin, 7}, {ApOnHit, 8}}},
      {.name = "Shadowflame", .fit = {{ApMage, 8}, {ApAssassin, 8}, {ApOnHit, 5}}, .counters = {{Magicpen, 0.4}}},
      {.name = "Void Staff", .fit = {{ApMage, 7}, {ApBattlemage, 6}, {ApAssassin, 6}, {ApOnHit, 6}},
       .counters = {{Magicpen, 1}}, .group = "blight"},
      {.name = "Cryptbloom", .fit = {{ApMage, 6}, {ApAssassin, 5}}, .counters = {{Magicpen, 0.7}}, .group = "blight"},
      {.name = "Bloodletter's Curse", .fit = {{ApMage, 5}, {ApBattlemage, 7}}, .counters = {{Magicpen, 0.7}},
       .group = "blight"},
      {.name = "Zhonya's Hourglass", .fit = {{ApMage, 7}, {ApBattlemage, 7}, {ApAssassin, 7}, {ApOnHit, 6}},
       .counters = {{Antiburst, 1}, {Armor, 0.7}}},
      {.name = "Banshee's Veil", .fit = {{ApMage, 6}, {ApBattlemage, 5}, {ApAssassin, 6}, {ApOnHit, 5}},
       .counters = {{Mr, 0.8}, {Antiburst, 0.6}, {AntiCC, 0.5}}},
      {.name = "Malignance", .fit = {{ApMage, 8}, {ApBattlemage, 5}}},
      {.name = "Horizon Focus", .fit = {{ApMage, 7}}},
      {.name = "Blackfire Torch", .fit = {{ApMage, 8}, {ApBattlemage, 7}}},
      {.name = "Actualizer", .fit = {{ApMage, 7}}},
      {.name = "Archangel's Staff", .fit = {{ApMage, 7}, {ApBattlemage, 5}}, .group = "manaflow"},
      {.name = "Liandry's Torment", .fit = {{ApMage, 6}, {ApBattlemage, 9}, {ApOnHit, 5}},
       .counters = {{Magicpen, 0.6}}},
      {.name = "Rylai's Crystal Scepter", .fit = {{ApMage, 5}, {ApBattlemage, 7}}},
      {.name = "Riftmaker", .fit = {{ApBattlemage, 9}, {ApOnHit, 6}, {ApAssassin, 4}}},
      {.name = "Rod of Ages", .fit = {{ApBattlemage, 7}, {ApMage, 4}}},
      {.name = "Cosmic Drive", .fit = {{ApMage, 6}, {ApBattlemage, 6}, {ApAssassin, 6}}},
      {.name = "Stormsurge", .fit = {{ApMage, 7}, {ApAssassin, 8}}},
      {.name = "Hextech Rocketbelt", .fit = {{ApAssassin, 8}, {ApBattlemage, 5}}},
      {.name = "Lich Bane", .fit = {{ApAssassin, 8}, {ApMage, 4}, {ApOnHit, 6}}, .group = "spellblade"},
      {.name = "Nashor's Tooth", .fit = {{ApOnHit, 10}}},
      {.name = "Dusk and Dawn", .fit = {{ApOnHit, 7}}, .group = "spellblade"},
      {.name = "Morellonomicon", .fit = {{ApMage, 5}, {ApBattlemage, 5}, {ApAssassin, 4}}, .counters = {{Antiheal, 1}}},
      {.name = "Hextech Gunblade", .fit = {{ApAssassin, 4}}},

      // --- Enchanters ---
      {.name = "Moonstone Renewer", .fit = {{Enchanter, 8}}, .supportOnly = true},
      {.name = "Echoes of Helia", .fit = {{Enchanter, 7}}, .supportOnly = true},
      {.name = "Dawncore", .fit = {{Enchanter, 7}}, .supportOnly = true},
      {.name = "Staff of Flowing Water", .fit = {{Enchanter, 7}}, .supportOnly = true},
      {.name = "Ardent Censer", .fit = {{Enchanter, 7}}, .supportOnly = true},
      {.name = "Shurelya's Battlesong", .fit = {{Enchanter, 6}}, .supportOnly = true},
      {.name = "Redemption", .fit = {{Enchanter, 7}}, .counters = {{AntiPoke, 0.4}}, .supportOnly = true},
      {.name = "Mikael's Blessing", .fit = {{Enchanter, 6}}, .counters = {{AntiCC, 0.8}}, .supportOnly = true},
      {.name = "Imperial Mandate", .fit = {{Enchanter, 6}}, .supportOnly = true},
  };
  return items;
}

const ItemKnowledge* findItemKnowledge(std::string_view name) {
  const auto& all = itemKnowledge();
  auto it = std::find_if(all.begin(), all.end(), [&](const ItemKnowledge& k) { return k.name == name; });
  return it == all.end() ? nullptr : &*it;
}

const std::vector<BootsKnowledge>& bootsKnowledge() {
  using enum Archetype;
  using enum Need;
  static const std::vector<BootsKnowledge> boots = {
      {.name = "Plated Steelcaps", .fit = {{Tank, 5}, {Bruiser, 5}}, .counters = {{Armor, 1}, {AntiAuto, 0.8}}},
      {.name = "Mercury's Treads", .fit = {{Tank, 5}, {Bruiser, 5}}, .counters = {{Mr, 1}, {AntiCC, 1}}},
      {.name = "Berserker's Greaves", .fit = {{AdcCrit, 9}, {AdcOnHit, 9}, {ApOnHit, 6}}},
      {.name = "Sorcerer's Shoes", .fit = {{ApMage, 9}, {ApBattlemage, 8}, {ApAssassin, 9}, {ApOnHit, 6}}},
      {.name = "Ionian Boots of Lucidity",
       .fit = {{AdCaster, 8}, {Enchanter, 6}, {AdAssassin, 6}, {Bruiser, 4}, {ApMage, 4}}},
      {.name = "Boots of Swiftness", .fit = {{Enchanter, 5}, {Tank, 4}}},
      {.name = "Gluttonous Greaves", .fit = {{AdAssassin, 5}, {Bruiser, 4}}},
  };
  return boots;
}

bool isAntihealItem(std::string_view name) {
  return name == "Mortal Reminder" || name == "Chempunk Chainsword" || name == "Morellonomicon" || name == "Thornmail";
}
