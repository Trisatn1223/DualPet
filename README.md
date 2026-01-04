# Animal Companion Module (AzerothCore)

## Features
- Hunter always has a second pet
- Second pet is always the pet from stable slot 1
- Works like the later "Animal Companion" talent
- No SQL changes required
- No core modifications

## Installation
1. Copy this module into:
   azerothcore/modules/mod-animal-companion
2. Rebuild the core:
   mkdir build && cd build
   cmake ..
   make -j$(nproc)

## Notes
- Second pet deals 50% damage
- Second pet despawns automatically
- Works with WotLK 3.3.5a
