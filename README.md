# Angband 4.2.5

<p align="center">
  <img src="screenshots/title.png" width="425"/>
  <img src="screenshots/game.png" width="425"/>
</p>

Angband is a graphical dungeon adventure game that uses textual characters to
represent the walls and floors of a dungeon and the inhabitants therein, in the
vein of games like NetHack and Rogue. If you need help in-game, press `?`.

- **Installing Angband:** See the [Official Website](https://angband.github.io/angband/) or [compile it yourself](https://angband.readthedocs.io/en/latest/hacking/compiling.html).
- **How to Play:** [The Angband Manual](https://angband.readthedocs.io/en/latest/)
- **Getting Help:** [Angband Forums](https://angband.live/forums/)

Enjoy!

-- The Angband Dev Team
# AI-Augmented Roguelike Game: Enhanced Design and Mechanics

To address your request for a higher level of order and a better outcome, this iteration refines the previous design by improving clarity, structure, and depth. The content is organized with precise headings, detailed explanations, and practical examples tailored to a kinesthetic learning style—emphasizing tangible actions and interactions. Below is a comprehensive, standalone design document for your modding project.

---

## Game Overview

This is a turn-based roguelike set in the **Tower of Babel**, a 100-floor megastructure where players control a **climber** seeking a lost companion. The game blends classic roguelike elements—procedural generation, permadeath, and inventory management—with a unique AI companion, **Lute the Bard**, who narrates the journey and adapts to player actions via a local AI model. Built on a fork of *Angband*, this design enhances gameplay with dynamic storytelling and faction interactions.

### Key Features
- **Premise**: Ascend the tower, floor by floor, to reunite with a lost companion.
- **Objective**: Reach floor 100 and defeat a final boss (e.g., a corrupt ruler).
- **Unique Element**: Lute, an AI-driven Bard, narrates actions, manages gear, and evolves with the player.

### Base Game Choice
- **Game**: *Angband*
- **Reason**:
  - **Modular Code**: Written in C, with files like `generate.c` (level generation) and `player.c` (player mechanics) easy to modify.
  - **Mechanics Fit**: Offers turn-based combat, permadeath, and procedural levels—perfect for this project.
  - **Support**: Active community with resources (e.g., GitHub repository).
- **Modding Plan**: Fork *Angband* to integrate Lute, the Model Context Protocol (MCP), and new systems.

---

## Setting

The **Tower of Babel** is a sprawling, vertical world with 100 procedurally generated floors, called **ringdoms**, each with unique themes and static landmarks.

### Floor Design
- **Size**: 100x100 tile grid—larger than *Angband*'s default for richer exploration.
- **Generation**:
  - **Algorithm**: Modified room-and-corridor system from `generate.c`.
  - **Process**: Rooms (size varies by theme) connect via corridors, with hazards and NPCs placed dynamically.
- **Static Elements** (consistent across runs):
  - **Airship Dock**: 5x5 tiles at (50,50) for NPC interactions.
  - **Outer Wall Door**: 3x3 tiles at (0,50), locked until a key is found.
  - **Docked Airship**: 7x3 tiles at (75,25), searchable for loot.

### Example Ringdoms
| Floor | Name          | Theme                | Features                          | Hazards                  |
|-------|---------------|----------------------|-----------------------------------|--------------------------|
| 1     | The Ringdom   | Marketplace          | Gondolas (10x10), Traders (5-10)  | Pickpockets (10% chance) |
| 3     | The Parlor    | Decadent Halls       | Ballroom (20x20), Masked NPCs     | Secret Rooms (10% spawn) |
| 5     | The Baths     | Steamy Caves         | Hot Springs (3x3, +5 HP)          | Steam Vents (10 damage)  |
| 7     | The Windlass  | Industrial Zone      | Gears (40x40), Automatons (5-7)   | Steam Vents (10 damage)  |
| 10    | The Heart     | Political Hub        | Throne Room (50x50), Leaders      | Guarded Doors (5x5)      |

### Narrative Goal
- **Mission**: Find a lost companion (e.g., a friend or spouse).
- **Progression**: Clues unfold via Lute's narration and ringdom interactions.

---

## Core Mechanics

These mechanics form the game's foundation, expanded from *Angband*'s systems.

### Turn-Based System
- **Flow**: Player acts (move, attack, etc.), then enemies respond.
- **Clock**: Each action adds 1 unit to the game clock, triggering events (e.g., NPC departure after 50 turns).
- **Code**: Managed in `game-world.c`.

### Combat
- **Hit Check**: Roll $d20 + \text{Attribute}$ vs. enemy defense.
  - **Example**: Strength 10 + roll 12 = 22 vs. Defense 15 → Hit.
- **Damage**: $ \text{Attribute} + d6 - \text{Enemy Armor} $.
  - **Example**: Strength 10 + roll 4 - Armor 2 = 12 damage.
- **Attributes**:
  - **Strength**: Melee damage.
  - **Agility**: Dodge and ranged attacks.
  - **Intellect**: Social or magic (expandable).
- **Enemy Example**: Gear Golem (HP 50, Defense 18, Attack 12).
- **Code**: Extended in `player-attack.c` and `mon-attack.c`.

### Inventory
- **Player**: 10 slots (e.g., sword, potion = 1 slot each).
- **Lute's Bag**: 5-10 slots (randomized at start).
  - **Access**: 1 turn; 5% failure chance in combat (roll $d100 < 5$) → item drops.
- **Actions**: Drop, swap, or transfer items.
- **Code**: Updated in `item.c`.

### Permadeath
- **Rule**: HP = 0 → run ends, save file deleted.
- **Save Points**: Auto-save on floor exit (in `save.c`).

### Procedural Generation
- **Method**: Rooms and corridors themed by ringdom.
- **Static Integration**: Hardcoded features (e.g., docks) ensure consistency.
- **Code**: Enhanced in `generate.c`.

---

## The Bard: Lute

Lute is an AI companion who narrates, assists, and grows with the player.

### Structure
```c
struct bard {
    int courage;          // 0-100
    int perception;       // 1-20
    int lore;             // 1-20
    struct item bag[10];  // 5-10 slots
    struct event memory[10]; // Event log
    char narration[256];  // Narration text
};
```

### Spawn
- Random spot on floor 1 (e.g., (10,10)).
- Starting Stats: Courage 50, Perception 1, Lore 1.

### Features

#### Narration
- **Trigger**: Every action (e.g., "You strike the golem—sparks fly!").
- **Process**: Game state → MCP → AI → narration text.
- **Code**: `bard_narrate()` in `bard.c`.

#### Memory
- **Capacity**: 10 events (FIFO queue).
- **Event Format**: {type: "combat", floor: 2, detail: "killed golem"}.
- **Use**: 50% chance per turn to reference memory (e.g., "Another golem—like old times!").
- **Code**: `bard_memory_update()`, `bard_memory_check()`.

#### Courage
- **Range**: 0-100 (starts at 50).
- **Updates**:
  - Kill enemy: +10
  - Quest done: +5
  - Hit taken: -15
  - Flee: -10
- **Effect**: Courage ≥ 75 → Lute distracts enemy (10% miss chance, once per floor).
- **Code**: `bard_courage_update()`, `bard_courage_action()`.

#### Inventory
- **Slots**: 5-10 (randomized).
- **Access**: 1 turn; 5% fumble risk in combat.
- **Code**: `bard_inventory_access()`.

#### Knowledge
- **Stats**: Perception (relics), Lore (texts).
- **Growth**: +1 per floor, max 20.
- **Check**: $d20 + (\text{Perception} + \text{Lore})$ vs. DC (e.g., 15).
- **Success**: Reveals bonuses (e.g., +5 attack) or clues.
- **Code**: `bard_knowledge_roll()`.

#### AI Integration
- **Model**: Llama 3 via Ollama (local, 4GB RAM).
- **Input**: JSON (e.g., {floor: 5, action: "attack", health: 50}).
- **Output**: 256-character narration.
- **Prompt**: "Narrate {action} on {floor}, health {health}, grim tone."
- **Code**: `bard_ai_call()` in `mcp.c`.

---

## Model Context Protocol (MCP)

The MCP connects the game to Lute's AI for real-time narration.

### Setup
- **Server**: Local Flask app at 127.0.0.1:5000.
- **Code Example**:
```python
from flask import Flask, request
import ollama

app = Flask(__name__)

@app.route('/narrate', methods=['POST'])
def narrate():
    data = request.json
    prompt = f"Narrate {data['action']} on floor {data['floor']}"
    response = ollama.generate(model='llama3', prompt=prompt)
    return {'text': response['text']}

app.run(port=5000)
```

### Data Flow
1. Game sends JSON (e.g., {action: "kill", enemy: "golem"}).
2. MCP forwards to Llama 3.
3. AI returns narration (e.g., "The golem crumbles—iron bends!").
4. Game displays it.

### Caching
- **Preload**: 5 narrations per floor.
- **Refresh**: Every 10 turns or major event.
- **Goal**: <0.1s latency.

---

## Factions

Factions add replayability and social dynamics.

### Structure
- **Persistent**:
  - **Mechanimists**: Tech-focused (The Windlass).
  - **Putus Templar**: Zealots (The Parlor).
  - **Barathrumites**: Rebels (scattered).
- **Procedural**: 1-2 per floor (e.g., Steam Cult in The Baths).

### Reputation
- **Range**: -100 to 100 (starts at 0).
- **Changes**:
  - Aid: +20
  - Harm: -30
  - Quest: +15
- **Effects**:
  - ≥ +50: Ally (e.g., NPC aid).
  - ≤ -50: Hostile (attacks on sight).
- **Code**: `faction.c` with `struct faction {name, rep, behavior}`.

### Quests
- **Example**: "Sabotage automaton for 50 gold."
- **Generation**: Via MCP in `faction_quest_generate()`.

---

## Gameplay Loop

Here's how it feels to play:

1. **Start**: Pick Lute's trait (e.g., +5 Lore), spawn at (10,10) on floor 1.
2. **Explore**: Move across the 100x100 grid, finding items and NPCs. Lute narrates: "A gondola sways—trade awaits."
3. **Fight**: Attack a golem (roll $d20 + 10$ vs. 18). Lute cheers: "Steel bends to your will!"
4. **Interact**: Persuade an NPC (roll $d20 + \text{charisma}$ vs. DC 12). Lute advises: "He's greedy—haggle hard."
5. **Ascend**: Reach stairs at (90,90), climb to the next floor.
6. **End**: Floor 100 → fight boss (HP 200, Attack 25).

---

## Systems Breakdown

### Map
- **Grid**: 100x100 tiles.
- **Tiles**: 0 (floor), 1 (wall), 2-5 (features like vents).

### Combat
- **Hit**: $d20 + \text{Attribute} \geq \text{Defense}$.
- **Damage**: $\text{Attribute} + d6 - \text{Armor}$.

### Memory
- **Events**: 10 slots (e.g., {type: "loot", floor: 3}).
- **Recall**: 50% chance per turn.

### Courage
- **Formula**: $C_{\text{new}} = C_{\text{old}} + \Delta$ (e.g., +10 for kills).
- **Effect**: ≥ 75 → 10% enemy miss chance.

### Recognition
- **Check**: $d20 + (\text{Perception} + \text{Lore}) \geq \text{DC}$.

---

## Architecture

### Development Tools & Environment

- **Version Control**:
  - **Git** with GitHub repository
  - Branch strategy: `main`, `develop`, `feature/`, `bugfix/`
  - Automated build testing with GitHub Actions

- **Build System**:
  - **CMake** for cross-platform build configuration
  - **Make** for Unix-based platforms
  - **MinGW** support for Windows development

- **IDE Support**:
  - **VSCode** with C/C++ extension
  - **CLion** project files
  - **Code::Blocks** project files for accessibility

- **Testing Framework**:
  - **Check** for C unit testing
  - **Python pytest** for MCP testing
  - Automated test runs on build

- **Documentation**:
  - **Doxygen** for code documentation
  - **Markdown** for design documents and user guides
  - Wiki integration with GitHub

### Core Game Components

- **Base Engine** (Angband Fork):
  - `main-*.c` - Platform-specific main functions
  - `z-*.c` - Low-level utility functions
  - `ui-*.c` - User interface components
  - `obj-*.c` - Object handling
  - `player-*.c` - Player mechanics

- **Modified & New Components**:
  - `generate-ringdom.c` - Enhanced level generation
  - `bard.c/.h` - Lute mechanics
  - `mcp.c/.h` - Model Context Protocol
  - `faction.c/.h` - Faction system
  - `memory.c/.h` - Event memory system

- **External Systems**:
  - MCP Python server (`server.py`)
  - LLM integration layer (`llm_wrapper.py`)
  - Tools for content generation (`content_tools/`)

### Bard System Architecture

#### Core Structure

```c
typedef struct event_memory {
    char type[32];         // "combat", "discovery", "dialogue", etc.
    int floor;             // Floor where event occurred
    char detail[128];      // Specific details
    int importance;        // 1-10, affects recall priority
    int timestamp;         // Game turns when recorded
} EventMemory;

typedef struct bard {
    // Core attributes
    int courage;               // 0-100, affects combat support
    int perception;            // 1-20, affects item discovery
    int lore;                  // 1-20, affects lore knowledge
    
    // Dynamic state
    int mood;                  // -100 to 100, affects narration tone
    int relationship;          // 0-100, player relationship
    int energy;                // 0-100, special ability resource
    
    // Inventory system
    int bag_capacity;          // 5-10 slots
    struct object *bag[MAX_BAG_SIZE];
    
    // Memory system
    EventMemory memories[MAX_MEMORIES];
    int memory_count;
    int recent_memory_idx;
    
    // Narration system
    char current_narration[MAX_NARRATION_LENGTH];
    char cached_narrations[CACHE_SIZE][MAX_NARRATION_LENGTH];
    int cache_indices[CACHE_SIZE];  // Game states for cache
    
    // Special abilities
    bool abilities[MAX_ABILITIES];
    int ability_cooldowns[MAX_ABILITIES];
    
    // AI integration
    int prompt_template_idx;   // Current prompt template
    char context_buffer[CONTEXT_BUFFER_SIZE];
    int context_markers[MAX_CONTEXT_MARKERS];
    
    // Statistics for debugging and balancing
    int narrations_generated;
    int cache_hits;
    int abilities_used[MAX_ABILITIES];
    int items_stored;
    int items_retrieved;
} Bard;
```

#### Key Systems & Algorithms

- **Memory Management System**:
  ```c
  void bard_remember_event(Bard *lute, const char *type, int floor, const char *detail, int importance) {
      // Find slot (replace least important or oldest)
      int slot = find_memory_slot(lute);
      
      // Store the new memory
      strncpy(lute->memories[slot].type, type, sizeof(lute->memories[slot].type) - 1);
      lute->memories[slot].floor = floor;
      strncpy(lute->memories[slot].detail, detail, sizeof(lute->memories[slot].detail) - 1);
      lute->memories[slot].importance = importance;
      lute->memories[slot].timestamp = current_game_turn;
      
      // Update memory count and pointer
      if (lute->memory_count < MAX_MEMORIES)
          lute->memory_count++;
      lute->recent_memory_idx = slot;
  }

  const EventMemory *bard_recall_relevant_memory(Bard *lute, const char *type, int floor) {
      // Probability increases with more similar contexts
      int best_match = -1;
      int best_score = 0;
      
      for (int i = 0; i < lute->memory_count; i++) {
          // Calculate relevance score based on type, recency, location, importance
          int score = calculate_memory_relevance(lute->memories[i], type, floor);
          
          if (score > best_score) {
              best_score = score;
              best_match = i;
          }
      }
      
      // Only recall if score passes threshold (affected by lore attribute)
      if (best_match >= 0 && best_score > (20 - lute->lore) * 5)
          return &lute->memories[best_match];
          
      return NULL;  // No relevant memory found
  }
  ```

- **Narration Caching System**:
  ```c
  void bard_cache_narration(Bard *lute, int state_hash, const char *narration) {
      // Find LRU cache slot or matching state
      int slot = find_cache_slot(lute, state_hash);
      
      // Store narration and state hash
      strncpy(lute->cached_narrations[slot], narration, MAX_NARRATION_LENGTH - 1);
      lute->cache_indices[slot] = state_hash;
      
      // Update cache statistics
      lute->narrations_generated++;
  }

  bool bard_get_cached_narration(Bard *lute, int state_hash, char *output, size_t output_size) {
      // Look for matching state in cache
      for (int i = 0; i < CACHE_SIZE; i++) {
          if (lute->cache_indices[i] == state_hash) {
              // Copy cached narration to output
              strncpy(output, lute->cached_narrations[i], output_size - 1);
              output[output_size - 1] = '\0';
              
              // Update cache statistics
              lute->cache_hits++;
              return true;
          }
      }
      
      return false;  // Cache miss
  }

  int calculate_state_hash(const char *action, const char *target, int floor, int health) {
      // A simple hash function for game state
      int hash = floor * 1000 + health;
      
      for (const char *c = action; *c; c++)
          hash = ((hash << 5) + hash) + *c;
          
      for (const char *c = target; *c; c++)
          hash = ((hash << 5) + hash) + *c;
          
      return hash;
  }
  ```

- **Courage & Mood System**:
  ```c
  void bard_update_courage(Bard *lute, int event_type, void *event_data) {
      int change = 0;
      
      switch(event_type) {
          case EVENT_PLAYER_KILL_ENEMY:
              change = 10 * (1 + ((struct monster *)event_data)->level / 10);
              break;
          case EVENT_PLAYER_DAMAGED:
              change = -5 * (*((int *)event_data) / player->max_hp * 10);
              break;
          case EVENT_PLAYER_FLEE:
              change = -10;
              break;
          case EVENT_QUEST_COMPLETE:
              change = 15;
              break;
      }
      
      // Apply change with dampening based on current value
      if (change > 0) {
          lute->courage += change * (1 - (lute->courage / 120.0));
      } else {
          lute->courage += change * (1 - ((100 - lute->courage) / 120.0));
      }
      
      // Ensure within bounds
      lute->courage = MIN(MAX(lute->courage, 0), 100);
      
      // Trigger personality shifts at thresholds
      if (lute->courage >= 75 && !lute->abilities[ABILITY_DISTRACTION])
          bard_unlock_ability(lute, ABILITY_DISTRACTION);
      else if (lute->courage <= 25 && !lute->abilities[ABILITY_CAUTION])
          bard_unlock_ability(lute, ABILITY_CAUTION);
  }
  
  bool bard_attempt_courage_action(Bard *lute, int action_type, void *target) {
      // Check if courage is high enough for the action
      int threshold = get_courage_threshold(action_type);
      
      if (lute->courage >= threshold) {
          // Roll with bonus from courage
          int roll = randint0(100) + (lute->courage - threshold) / 2;
          
          if (roll >= 50) {
              // Action succeeds
              perform_courage_action(lute, action_type, target);
              
              // Cooldown effect
              lute->courage -= threshold / 5;
              return true;
          }
      }
      
      return false;
  }
  ```

- **MCP Request Formation**:
  ```c
  bool bard_generate_narration(Bard *lute, const char *action, const char *target, 
                              int floor, int health, char *output, size_t output_size) {
      // Calculate state hash for caching
      int state_hash = calculate_state_hash(action, target, floor, health);
      
      // Check cache first
      if (bard_get_cached_narration(lute, state_hash, output, output_size))
          return true;
      
      // Prepare request JSON
      json_t *request = json_object();
      json_object_set_new(request, "action", json_string(action));
      json_object_set_new(request, "target", json_string(target));
      json_object_set_new(request, "floor", json_integer(floor));
      json_object_set_new(request, "health", json_integer(health));
      json_object_set_new(request, "courage", json_integer(lute->courage));
      json_object_set_new(request, "mood", json_integer(lute->mood));
      
      // Add relevant memory if available
      const EventMemory *memory = bard_recall_relevant_memory(lute, action, floor);
      if (memory) {
          json_t *memory_obj = json_object();
          json_object_set_new(memory_obj, "type", json_string(memory->type));
          json_object_set_new(memory_obj, "floor", json_integer(memory->floor));
          json_object_set_new(memory_obj, "detail", json_string(memory->detail));
          json_object_set_new(request, "memory", memory_obj);
      }
      
      // Convert to JSON string
      char *json_str = json_dumps(request, JSON_COMPACT);
      
      // Make MCP request
      bool success = mcp_request("narrate", json_str, output, output_size);
      
      // Cache result if successful
      if (success)
          bard_cache_narration(lute, state_hash, output);
      
      json_decref(request);
      free(json_str);
      
      return success;
  }
  ```

### LLM Integration

#### Recommended Open Source Models

- **Primary Model**: **Llama 3 8B** 
  - Balanced performance and resource usage
  - Small enough to run on modest hardware
  - Strong narrative capabilities

- **Lightweight Alternative**: **Phi-3 Mini (3.8B)**
  - Extremely efficient performance/size ratio
  - Can run on systems with limited RAM (4GB)
  - Faster inference than larger models

- **High-Quality Alternative**: **Mistral 7B Instruct v0.2**
  - Superior narrative quality
  - Good instruction following
  - Slightly larger resource requirements

- **Specialized Option**: **TinyLlama (1.1B)**
  - Ultra-lightweight option
  - Can run on very modest hardware
  - Sacrifices some narrative quality

#### Model Optimization Techniques

- **Quantization**:
  - Use 4-bit quantization (GGUF format)
  - Reduces memory footprint by 75% with minimal quality loss
  - Implementation: Ollama supports this natively

- **Context Length Management**:
  - Limit context to 512-1024 tokens
  - Summarize previous context periodically
  - Retain only critical game state information

- **Batched Generation**:
  - Pre-generate common narrations during loading screens
  - Queue non-urgent narrations during high-activity periods

- **Model Splitting**:
  - Use different models for different narrative types
  - Main narration: Llama 3 
  - Lore generation: Mistral
  - Quick responses: TinyLlama

#### Prompt Engineering

- **Base Template**:
```
You are Lute, a bard in the Tower of Babel. Narrate this moment in the adventure:
- Action: {action}
- Target: {target}
- Current floor: {floor_name} (Floor {floor_number})
- Player health: {health}/{max_health}
- Your mood: {mood_description}

Write 1-2 sentences (maximum 50 words) in a {tone} style that captures this moment.
If referencing a memory, smoothly incorporate: {memory_detail}
```

- **Dynamic Prompt Adaptation**:
  - Adjust tone based on Lute's courage (bold vs. cautious)
  - Include floor-specific keywords
  - Reference relevant past events based on memory system

- **Specialized Templates**:
  - Combat narration
  - Discovery narration
  - NPC interaction narration
  - Idle/exploration narration

### Programming Tips & Pitfalls

#### Memory Management

- **Use Static Buffers**:
  - Pre-allocate fixed-size memory for narrations
  - Avoid dynamic allocation during gameplay
  ```c
  // Good: Static allocation
  char narration_buffer[MAX_NARRATION_LENGTH];
  
  // Avoid: Dynamic allocation during gameplay
  char *narration = malloc(length * sizeof(char));
  ```

- **String Handling Safety**:
  - Always use bounds-checked string functions
  - Apply strlcpy/strlcat instead of strcpy/strcat
  ```c
  // Good: Safe string operations
  strlcpy(dest, src, dest_size);
  
  // Avoid: Unsafe operations
  strcpy(dest, src);  // Can cause buffer overflows
  ```

- **Resource Cleanup**:
  - Implement proper cleanup for MCP connections
  - Use systematic error handling with cleanup blocks
  ```c
  // Pattern for resource management
  bool function_with_resources() {
      Resource *r = acquire_resource();
      if (!r) return false;
      
      if (!use_resource(r)) {
          release_resource(r);
          return false;
      }
      
      bool result = finish_with_resource(r);
      release_resource(r);
      return result;
  }
  ```

#### Threading & Performance

- **Asynchronous Processing**:
  - Use a worker thread for AI requests to prevent gameplay stutter
  ```c
  pthread_t mcp_thread;
  
  void *mcp_worker(void *arg) {
      MCP_Request *req = (MCP_Request *)arg;
      process_mcp_request(req);
      req->complete = true;
      return NULL;
  }
  
  void queue_mcp_request(const char *action, const char *target) {
      MCP_Request *req = create_mcp_request(action, target);
      pthread_create(&mcp_thread, NULL, mcp_worker, req);
      // Can detach or join later depending on needs
  }
  ```

- **Caching Strategy**:
  - Implement tiered caching (memory and disk)
  - Pre-generate common narrations during load times
  ```c
  void pregenerate_common_narrations(Bard *lute, int floor) {
      const char *common_actions[] = {"attack", "dodge", "discover", "enter"};
      const char *common_targets[] = {"enemy", "trap", "item", "doorway"};
      
      for (int i = 0; i < 4; i++) {
          for (int j = 0; j < 4; j++) {
              char narration[MAX_NARRATION_LENGTH];
              bard_generate_narration(lute, common_actions[i], common_targets[j], 
                                      floor, player->hp, narration, sizeof(narration));
              // Narration automatically cached by the function
          }
      }
  }
  ```

- **Fallback Mechanisms**:
  - Create template-based narrations as backup
  - Apply degradation strategies when performance issues occur
  ```c
  char *generate_fallback_narration(const char *action, const char *target) {
      static char buffer[256];
      static const char *templates[] = {
          "You %s the %s with determination.",
          "With a swift motion, you %s the %s.",
          "The %s is affected by your %s.",
          "You carefully %s the %s and continue forward."
      };
      
      snprintf(buffer, sizeof(buffer), templates[randint0(4)], action, target);
      return buffer;
  }
  ```

#### AI Integration Robustness

- **Error Recovery**:
  - Implement timeouts for AI requests
  - Provide graceful degradation when AI is unavailable
  ```c
  #define MCP_TIMEOUT_MS 500
  
  bool mcp_request_with_timeout(const char *endpoint, const char *data, 
                               char *response, size_t response_size) {
      // Start timer
      struct timespec start, current;
      clock_gettime(CLOCK_MONOTONIC, &start);
      
      // Start request
      MCP_Request *req = begin_mcp_request(endpoint, data);
      
      // Poll with timeout
      while (!req->complete) {
          clock_gettime(CLOCK_MONOTONIC, &current);
          long elapsed_ms = (current.tv_sec - start.tv_sec) * 1000 + 
                           (current.tv_nsec - start.tv_nsec) / 1000000;
          
          if (elapsed_ms > MCP_TIMEOUT_MS) {
              cancel_mcp_request(req);
              return false;
          }
          
          // Short sleep to prevent CPU spinning
          usleep(10000);  // 10ms
      }
      
      // Copy response
      strncpy(response, req->response, response_size - 1);
      response[response_size - 1] = '\0';
      
      free_mcp_request(req);
      return true;
  }
  ```

- **State Serialization**:
  - Save and restore MCP state during game save/load
  - Persist memory and narration caches
  ```c
  bool bard_save_state(Bard *lute, const char *filename) {
      FILE *f = fopen(filename, "wb");
      if (!f) return false;
      
      // Write basic attributes
      fwrite(&lute->courage, sizeof(int), 1, f);
      fwrite(&lute->perception, sizeof(int), 1, f);
      fwrite(&lute->lore, sizeof(int), 1, f);
      
      // Write memory system
      fwrite(&lute->memory_count, sizeof(int), 1, f);
      fwrite(lute->memories, sizeof(EventMemory), lute->memory_count, f);
      
      // Write narration cache (indices and content)
      fwrite(lute->cache_indices, sizeof(int), CACHE_SIZE, f);
      for (int i = 0; i < CACHE_SIZE; i++) {
          fwrite(lute->cached_narrations[i], sizeof(char), MAX_NARRATION_LENGTH, f);
      }
      
      fclose(f);
      return true;
  }
  
  bool bard_load_state(Bard *lute, const char *filename) {
      FILE *f = fopen(filename, "rb");
      if (!f) return false;
      
      // Read basic attributes
      fread(&lute->courage, sizeof(int), 1, f);
      fread(&lute->perception, sizeof(int), 1, f);
      fread(&lute->lore, sizeof(int), 1, f);
      
      // Read memory system
      fread(&lute->memory_count, sizeof(int), 1, f);
      fread(lute->memories, sizeof(EventMemory), lute->memory_count, f);
      
      // Read narration cache
      fread(lute->cache_indices, sizeof(int), CACHE_SIZE, f);
      for (int i = 0; i < CACHE_SIZE; i++) {
          fread(lute->cached_narrations[i], sizeof(char), MAX_NARRATION_LENGTH, f);
      }
      
      fclose(f);
      return true;
  }
  ```

### Advanced Bard Features

#### Adaptive Personality

- **Mood Tracking**:
  ```c
  void bard_update_mood(Bard *lute, int event_type, void *event_data) {
      int change = 0;
      
      // Calculate mood change based on event
      switch(event_type) {
          case EVENT_PLAYER_DISCOVER_TREASURE:
              change = 10 + randint0(5);
              break;
          case EVENT_PLAYER_LOSE_ITEM:
              change = -10 - randint0(5);
              break;
          case EVENT_FLOOR_CHANGE_UP:
              change = 5;
              break;
          case EVENT_FLOOR_CHANGE_DOWN:
              change = -5;
              break;
      }
      
      // Apply change with personality-based modifiers
      lute->mood += change * (lute->courage > 50 ? 1.2 : 0.8);
      
      // Bound mood
      lute->mood = MIN(MAX(lute->mood, -100), 100);
      
      // Update narration style based on mood
      if (lute->mood > 50)
          lute->prompt_template_idx = TEMPLATE_EXCITED;
      else if (lute->mood < -50)
          lute->prompt_template_idx = TEMPLATE_MELANCHOLY;
      else
          lute->prompt_template_idx = TEMPLATE_NEUTRAL;
  }
  ```

- **Relationship Development**:
  ```c
  void bard_update_relationship(Bard *lute, int event_type) {
      int change = 0;
      
      switch(event_type) {
          case EVENT_PLAYER_USE_BARD_ADVICE:
              change = 5;
              break;
          case EVENT_PLAYER_GIVE_ITEM:
              change = 10;
              break;
          case EVENT_PLAYER_TAKE_ITEM:
              change = -3;
              break;
          case EVENT_PLAYER_IGNORE_BARD:
              change = -5;
              break;
      }
      
      // Apply and bound
      lute->relationship += change;
      lute->relationship = MIN(MAX(lute->relationship, 0), 100);
      
      // Unlock special abilities at thresholds
      if (lute->relationship >= 75 && !lute->abilities[ABILITY_INSPIRE])
          bard_unlock_ability(lute, ABILITY_INSPIRE);
  }
  ```

#### Ability System

- **Ability Types**:
  ```c
  enum BardAbilities {
      ABILITY_DISTRACTION,    // Combat: Distract enemies
      ABILITY_INSPIRE,        // Buff: Temporarily increase player stats
      ABILITY_RECALL,         // Knowledge: Reveal map section from memory
      ABILITY_COMPOSE,        // Special: Create consumable item from components
      ABILITY_CHARM,          // Social: Influence NPC attitudes
      ABILITY_CAUTION,        // Defense: Warn of nearby dangers
      MAX_ABILITIES
  };
  
  bool bard_use_ability(Bard *lute, int ability_idx, void *target) {
      // Check if ability is unlocked and not on cooldown
      if (!lute->abilities[ability_idx] || lute->ability_cooldowns[ability_idx] > 0)
          return false;
      
      // Check energy cost
      int cost = get_ability_cost(ability_idx);
      if (lute->energy < cost)
          return false;
      
      // Apply ability effect
      bool success = execute_ability(lute, ability_idx, target);
      
      if (success) {
          // Consume energy and set cooldown
          lute->energy -= cost;
          lute->ability_cooldowns[ability_idx] = get_ability_cooldown(ability_idx);
          lute->abilities_used[ability_idx]++;
          
          // Generate special narration for ability use
          char ability_narration[MAX_NARRATION_LENGTH];
          generate_ability_narration(lute, ability_idx, target, ability_narration, sizeof(ability_narration));
          msg("%s", ability_narration);
      }
      
      return success;
  }
  ```

#### Knowledge Evolution

- **Progressive Learning**:
  ```c
  void bard_learn_from_event(Bard *lute, int event_type, void *event_data) {
      switch(event_type) {
          case EVENT_DISCOVER_ITEM:
              // Increase perception when finding hidden items
              if (((struct object *)event_data)->hidden)
                  bard_gain_perception(lute, 1);
              break;
              
          case EVENT_READ_INSCRIPTION:
              // Gain lore from reading inscriptions and books
              bard_gain_lore(lute, 1 + ((struct text *)event_data)->complexity / 10);
              break;
              
          case EVENT_IDENTIFY_ITEM:
              // Learn from item identification
              bard_gain_lore(lute, 1);
              bard_gain_perception(lute, 1);
              break;
      }
  }
  
  void bard_gain_perception(Bard *lute, int amount) {
      // Apply diminishing returns based on current level
      float factor = 1.0f - (lute->perception / 30.0f);
      int actual_gain = MAX(1, (int)(amount * factor));
      
      lute->perception = MIN(lute->perception + actual_gain, MAX_ATTRIBUTE);
      
      // Notify player of significant gains
      if (actual_gain > 0)
          msg("Lute's perception has improved to %d.", lute->perception);
  }
  ```

- **Knowledge Checks**:
  ```c
  bool bard_knowledge_check(Bard *lute, int check_type, int difficulty, void *object, char *result, size_t result_size) {
      int attribute = 0;
      int bonus = 0;
      
      // Determine primary attribute for check
      switch(check_type) {
          case CHECK_IDENTIFY_ITEM:
              attribute = lute->perception;
              // Bonus for previously seen similar items
              bonus = count_similar_items_in_memory(lute, (struct object *)object) * 2;
              break;
              
          case CHECK_RECALL_LORE:
              attribute = lute->lore;
              // Bonus for related memories
              bonus = count_related_memories(lute, ((struct text *)object)->category) * 3;
              break;
              
          case CHECK_DETECT_TRAP:
              attribute = lute->perception;
              // Penalty for higher floors
              bonus = -current_floor / 5;
              break;
      }
      
      // Make the roll
      int roll = randint0(20) + 1;  // d20
      int total = roll + attribute + bonus;
      
      // Check against difficulty
      bool success = (total >= difficulty);
      
      // Generate appropriate result text using MCP
      generate_knowledge_check_result(lute, check_type, roll, total, difficulty, 
                                     success, object, result, result_size);
      
      return success;
  }
  ```

This design offers a comprehensive, standalone architecture for your AI-augmented roguelike, with detailed mechanics, code snippets, and a structured layout to bring the Tower of Babel to life.
