#include "constant.h"
#include "types.h"
#include "externs.h"

describe_mon_type desc_list[MAX_CREATURES] = {

{"Filthy street urchin", "It looks squalid and thoroughly revolting"},
{"Blubbering idiot", "It tends to blubber a lot"},
{"Pitiful looking beggar", "You just can't help feeling sorry for it"},
{"Mangy looking leper", "You feel it isn't safe to touch it"},
{"Squint eyed rogue", "A hardy, street-wise crook that knows an easy"
  " catch when it sees one"},
{"Singing, happy drunk", "It makes you glad to be sober"},
{"Mean looking mercenary", "No job is too low for it"},
{"Battle scarred veteran", "It doesn't take to strangers kindly"},
{"Grey mushroom patch", "Yum! It looks quite tasty"},
{"Giant yellow centipede", "It is about four-foot long and carnivorous"},
{"Giant white centipede", "It is about four-foot long and carnivorous"},
{"White icky thing", "It is a sort of pile of slime on legs"},
{"Clear icky thing", "It is a smallish, slimy, icky, blobby creature"},
{"Giant white mouse", "It is about three-feet long with large teeth"},
{"Large brown snake", "It is about eight-feet long"},
{"Large white snake", "It is about eight-feet long"},
{"Kobold", "It is a small, ugly humanoid"},
{"White worm mass", "It is a large slimy pile of worms"},
{"Floating eye" , "A disembodied eye floating a few feet above the ground"},
{"Shrieker mushroom patch", "Yum! These look quite tasty"},
{"Blubbering icky thing", "It is a smallish, slimy, icky creature"},
{"Metallic green centipede", "It is about four-foot long and carnivorous"},
{"Novice warrior" , "He looks inexperienced but tough"},
{"Novice rogue" , "A rather shifty individual"},
{"Novice priest" , "He is tripping over his priestly robes"},
{"Novice mage" , "He is leaving behind a trail of dropped spell components"},
{"Yellow mushroom patch", "Yum! It looks quite tasty"},
{"White jelly" , "Its a large pile of white flesh"},
{"Giant green frog" , "It is as big as a wolf"},
{"Giant black ant" , "It is about three-feet long"},
{"White harpy" , "A flying, screeching bird with a woman's face"},
{"Blue yeek" , "A small humanoid like figure"},
{"Green worm mass" , "It is a large slimy pile of worms"},
{"Large black snake" , "It is about ten-feet long"},
{"Cave spider" , "It is a black spider that moves in fits and starts"},
{"Poltergeist" , "It is a ghastly, ghostly form"},
{"Metallic blue centipede", "It is about four-foot long and carnivorous"},
{"Giant white louse" , "It is six inches long"},
{"Black naga" , "A large black serpent's body with a female torso"},
{"Spotted mushroom patch", "Yum! It looks quite tasty"},
{"Yellow jelly" , "Its a large pile of yellow flesh"},
{"Scruffy looking hobbit" , "A short little guy, in bedragled clothes.  He"
  " asks you if you know of a good tavern nearby"},
{"Giant white ant" , "It is about two-feet long and has sharp pincers"},
{"Yellow mold" , "It is a strange growth on the dungeon floor"},
{"Metallic red centipede", "It is about four-foot long and carnivorous"},
{"Yellow worm mass" , "It is a large slimy pile of worms"},
{"Radiation eye" , "A glowing eye that seems to crackle with energy"},
{"Blue jelly" , "It's a large pile of pulsing blue flesh"},
{"Creeping copper coins" , "A strange pile of moving coins"},
{"Giant white rat" , "It is a very vicious rodent"},
{"Blue worm mass" , "It is a large slimy pile of worms"},
{"Large grey snake" , "It slithers and moves towards you"},
{"Wood spider" , "It creeps towards you"},
{"Green naga" , "A large green serpent with a female's torso, the"
  " green skin glistens with acid"},
{"Green glutton ghost" , "It is a very ugly green ghost with a"
  " voracious appetite"},
{"Green jelly" , "It is a large pile of pulsing green flesh"},
{"Skeleton kobold" , "It is a small animated kobold skeleton"},
{"Grey icky thing" , "It is a smallish, slimy, icky creature"},
{"Disenchanter eye" , "A large white floating eye that crackles with magic"},
{"Red worm mass" , "It is a large slimy pile of worms"},
{"Copperhead snake" , "It has a copper head and sharp venomous fangs"},
{"Purple mushroom patch" , "Yum! It looks quite tasty"},
{"Brown mold" , "A strange brown growth on the dungeon floor"},
{"Giant brown bat" , "It screeches as it attacks"},
{"Creeping silver coins" , "A pile of silver coins that move on"
  " thousands of tiny legs"},
{"Snaga" , "It is one of the many weaker 'slave' orcs, often"
  " mistakenly known as a goblin"},
{"Cave orc" , "It is often found in huge numbers in deep caves"},
{"Hill orc" , "It is a hardy well-weathered survivor"},
{"Rattlesnake" , "It is recognized by the hard-scaled end of it's body"
  " that is often 'rattled' to frighten it's prey"},
{"Manes" , "It is minor but aggressive demon"},
{"Bloodshot eye" , "A large floating bloodshot eye"},
{"Red naga" , "A large red snake with a woman's torso"},
{"Red jelly" , "It is a large pulsating mound of red flesh"},
{"Giant red frog" , "It looks poisonous"},
{"Green icky thing" , "It is a smallish, slimy, icky creature"},
{"Zombie kobold" , "It is an animated kobold corpse, flesh falls of"
  " in large chunks as it shambles forward"},
{"Lost soul" , "It is almost insubstantial"},
{"Brown yeek" , "It is a strange small humanoid"},
{"Green mold" , "It is a strange growth on the dungeon floor"},
{"Skeleton orc" , "It is an animated orc skeleton"},
{"Lemure" , "It is the larval form of a major demon"},
{"Seedy looking human" , "He is covered in scars and looks disreputable"},
{"Bandit" , "He is after your cash"},
{"Yeti" , "A large white figure covered in shaggy fur"},
{"Bloodshot icky thing" , "It is a slimy, icky creature"},
{"Giant grey rat" , "It a rodent of unusual size"},
{"Black harpy" , "A woman's face on the body of a vicious black bird"},
{"Orc shaman" , "An orc dressed in skins who gestures wildly"},
{"Giant red ant" , "It is large and has venomous mandibles"},
{"King cobra" , "It is a large snake with a hooded face"},
{"Giant spider" , "It is a vast black spider whose bulbous body"
  " is bloated with poison"},
{"Clear mushroom patch" , "Yum! It looks quite tasty"},
{"Giant white tick" , "It is moving slowly towards you"},
{"Hairy mold" , "It is a strange hairy growth on the dungeon floor"},
{"Disenchanter mold" , "It is a strange glowing growth on the dungeon floor"},
{"Tengu", "It is a fast moving demon, that blinks quickly in and out"
  " of existence; no other demon matches its teleporting mastery"},
{"Creeping gold coins" , "They are shambling forward on thousands of legs"},
{"Giant fruit fly" , "A fast-breeding, annoying pest"},
{"Brigand" , "He is eyeing your backpack"},
{"Orc zombie" , "It is a shambling orcish corpse leaving behind a trail"
  " of flesh"},
{"Creeping mithril coins" , "They are shambling forward on needle sharp legs"},
{"Gnome mage" , "A mage of short stature"},
{"Black mamba" , "It has glistening black skin, a sleek body and"
  " highly venomous fangs"},
{"Grape jelly" , "It is a pulsing mound of glowing flesh"},
{"Master yeek" , "A small humanoid that radiates some power"},
{"Priest" , "A robed humanoid dedicated to his god"},
{"Air spirit" , "A whirlwind of intelligent air"},
{"Skeleton human" , "It is an animated human skeleton"},
{"Zombie human" , "It is a shambling human corpse dropping chunks of"
  " flesh behind it"},
{"Moaning spirit" , "A ghostly apparition that shrieks horribly"},
{"Swordsman" , "A warrior of considerable skill"},
{"Stegocentipede" , "It is a vast armoured centipede with massive"
  " mandibles and a spiked tail"},
{"Killer brown beetle" , "It is a vicious insect with a tough carapace"},
{"Ogre" , "A hideous, smallish giant that is often found near or with orcs"},
{"Illusionist" , "A deceptive spell caster"},
{"Black orc" , "It is a large orc with powerful arms and deep black skin"},
{"Half-orc" , "It is a hideous deformed cross-breed with man and"
  " orc, combining man's strength and cunning with orcish evil"},
{"Orc captain" , "An armoured orc with an air of authority"},
{"Giant flea" , "It makes you itch just to look at it"},
{"Giant white dragon fly" , "It is a large fly that drips frost"},
{"Hill giant" , "A ten foot tall humanoid with powerful muscles"},
{"Flesh golem" , "A shambling humanoid monster with long scars"},
{"Giant black louse" , "It makes you itch just to look at it"},
{"Guardian naga" , "A giant snake like figure with a woman's torso"},
{"Giant clear centipede" , "It is about four-foot long and carnivorous"},
{"Frost giant" , "A twelve foot tall giant covered in furs"},
{"Spotted jelly" , "A jelly thing"},
{"Ochre jelly" , "A fast moving highly acidic jelly thing, that is eating"
  " away the floor it rests on"},
{"Griffon", "It is half lion, half eagle. It flies"
  " menacingly towards you"},
{"Homonculous" , "It is a small demonic spirit, full of malevolence"},
{"Clay golem" , "It is a massive statue-like figure made out of"
  " malleable clay"},
{"Umber hulk" , "It is like a huge beetle with glaring eyes and"
  " large mandibles capable of slicing through rock"},
{"Gelatinous cube" , "It is a strange vast gelatinous structure,"
  " that assumes cubic proportions as it lines all for walls of"
  " the corridors it patrols. Through it's transparent jelly structure"
  " you can see treasures that has been engulfed, and a few corpses as well"},
{"Giant green dragon fly" , "A vast foul smelling dragonfly"},
{"Fire giant" , "A glowing fourteen foot tall giant, flames drip from its"
  " red skin"},
{"Quasit" , "The chaotic evil master's favourite pet"},
{"Imp" , "The lawful evil master's favourite pet"},
{"Forest troll" , "It is green skinned and ugly"},
{"Water spirit" , "A whirlpool of sentient liquid"},
{"Giant brown scorpion" , "It is fast and poisonous"},
{"Earth spirit" , "It is humanoid in shape and made out of solid rock"},
{"Fire spirit" , "It is composed of pure flame"},
{"Uruk" , "It is a cunning orc of power, as tall as a man,"
  " and stronger. It fears little"},
{"Stone giant" , "It is eighteen foot tall and looking at you"},
{"Stone golem" , "It is an animated statue"},
{"Red mold" , "It is a strange red growth on the dungeon floor; it seems"
  " to burn with flame"},
{"Quylthulg" , "It is a strange pulsing mound of flesh"},
{"Nexus quylthulg" , "It is a very unstable, strange pulsing mound of flesh"},
{"Chimera" , "It is a strange concoction of lion, dragon and goat. It"
  " looks very odd but very avoidable"},
{"Cloud giant" , "It is a twenty foot tall giant wreathed in clouds"},
{"Storm giant" , "It is a twenty-five foot tall giant wreathed in lighting"},
{"Blue dragon bat" , "It is a glowing blue bat with a sharp tail"},
{"Mummified orc" , "It is an orcish figure covered in wrappings"},
{"Killer stag beetle" , "It is a giant beetle with vicious claws"},
{"Iron golem" , "It is a giant metal statue that moves slowly towards you"},
{"Giant yellow scorpion" , "It is a giant scorpion with a sharp stinger"},
{"Black ooze" , "It is a strangely moving puddle"},
{"Hardened warrior" , "A scarred warrior who moves with confidence"},
{"Red dragon bat" , "It is a sharp-tailed bat, wreathed in fire"},
{"Killer blue beetle" , "It is looking for prey"},
{"Forest wight" , "It is a ghostly apparition with a humanoid form"},
{"Mummified human" , "It is a human form encased in mouldy wrappings"},
{"Banshee" , "It is a ghostly woman's form that wails mournfully"},
{"Stone troll" , "It is a giant troll with scabrous black skin"},
{"Killer red beetle" , "It is a giant beetle with poisonous mandibles"},
{"Giant grey ant lion" , "It is an ant encased in shaggy grey fur"},
{"Giant fire tick" , "It is smoking and burning with great heat"},
{"White wraith" , "It is a tangible but ghostly form made of white fog"},
{"Killer fire beetle" , "It is a giant beetle wreathed in flames"},
{"Vampire" , "It is a humanoid with an aura of power. You notice a"
  " sharp set of front teeth"},
{"Black knight" , "It is a figure encased in deep black plate armour;"
  " it looks at you menacingly"},
{"Mage" , "A mage of some power; you can tell by the size of his hat"},
{"Ice troll" , "It is a white troll with powerfully clawed hands"},
{"Giant purple worm" , "It is a massive worm form, many feet in length,"
  " its vast maw drips acid and poison"},
{"Young blue dragon" , "It has a form that legends are made of. Its"
  " still tender scales are a deep blue in hue; sparks crackle along its"
  " length"},
{"Young white dragon" , "It has a form that legends are made of. Its"
  " still tender scales are a frosty white in hue; icy blasts of cold"
  " air come from it as it breathes"},
{"Young green dragon" , "It has a form that legends are made of. Its"
  " still tender scales are a deep green in hue; foul gas seeps through"
  " its scales"},
{"Skeleton troll" , "It is a troll skeleton animated by dark dweomers"},
{"Giant static ant" , "It is a giant ant that crackles with energy"},
{"Manticore" , "It is a winged lion's body, with a human torso, and a"
  " tail covered in vicious spikes"},
{"Grave wight" , "It is a ghostly form with eyes that haunt you"},
{"Killer slicer beetle" , "It is a beetle with deadly sharp"
  " cutting mandibles and a rock-hard carapace"},
{"Ghost" , "You don't believe in them"},
{"Death watch beetle" , "It is a giant beetle that produces a chilling sound"},
{"Ogre shaman" , "It is an ogre wrapped in furs and covered in grotesque"
  " body paints"},
{"Cave troll" , "It is a vicious monster, feared for its ferocity"},
{"Invisible stalker" , "It is impossible to define its form but"
  " its violence is legendary"},
{"Ninja" , "A humanoid clothed in black who moves with blinding speed"},
{"Barrow wight" , "It is a ghostly nightmare of a entity"},
{"Giant skeleton troll" , "It is the animated form of a massive troll"},
{"Wolf" , "It howls and snaps at you"},
{"Warg" , "It is a large wolf with eyes full of cunning"},
{"Werewolf" , "It is a huge wolf with eyes that glow with manly intelligence"},
{"Hell hound" , "It is a giant dog that glows with heat, flames pour"
  " from its nostrils"},
{"Hell hound" , "It is a giant dog that glows with heat, flames pour"
  " from its nostrils"},
{"Water troll" , "It is a troll that reeks of brine"},
{"Olog" , "It is a massive intelligent troll with needle sharp fangs"},
{"Water elemental" , "It is a giant tempest of water"},
{"Fire elemental" , "It is a giant inferno of flames"},
{"Air elemental" , "It is a giant tornado of winds"},
{"Lich" , "It is a skeletal form dressed in robes. It radiates vastly"
  " evil power"},
{"Master vampire" , "It is a humanoid form dressed in robes; power"
  " eminates from it's chilling frame"},
{"Giant red scorpion" , "It is a giant red scorpion. It looks poisonous"},
{"Earth elemental" , "It is a giant form composed of rock, with fists"
  " of awesome power"},
{"Ice elemental" , "It is a animated statue of ice, it regards"
  " you disdainfully"},
{"Magma elemental" , "It is a glowing form of molten hate"},
{"Ooze elemental" , "Animated filth, an eyesore of ooze"},
{"Smoke elemental" , "Its blackened form crackles with heat"},
{"Young black dragon" , "It has a form that legends are made of. Its"
  " still tender scales are a darkest black hue; acid drips from its body"},
{"Young red dragon" , "It has a form that legends are made of. Its"
  " still tender scales are a deepest red hue; heat radiates from its"
  " form"},
{"Warlock" , "A gaunt figure, clothed in black robes"},
{"Mummified troll" , "It is a massive figure clothed in wrappings. You"
  " are wary of its massive fists"},
{"Giant red ant lion" , "A giant ant covered in shaggy fur, its"
  " powerful jaws glow with heat"},
{"Mature white dragon" , "A large dragon, scales gleaming bright white"},
{"Xorn" , "A huge creature of the element Earth. Able to merge with its"
  " element, it has four huge arms protruding from its enormous torso"},
{"Grey wraith" , "A tangible but ghostly form, made of grey fog, the"
  " air around it feels deathly cold"},
{"Young multi-hued dragon" , "It has a form that legends are made"
  " of; beautiful scales of shimmering and magical colours cover it"},
{"Mature blue dragon" , "A large dragon, scales tinted deep blue"},
{"Mature green dragon" , "A large dragon, scales tinted deep green"},
{"Iridescent beetle" , "It is a giant beetle, whose carapace shimmers"
  " with vibrant energies"},
{"Vampire lord" , "A foul wind chills your bones as this ghastly"
  " figure approaches"},
{"Master lich" , "A skeletal form wrapped in robes, powerful magic"
  " crackles along its bony fingers"},
{"Mature red dragon" , "A large dragon, scales tinted deep red"},
{"Mature black dragon" , "A large dragon, with scales of deepest black"},
{"Mature multi-hued dragon" , "A large dragon, scales shimmering many colours"},
{"Ancient blue dragon" , "A huge dragonic form, lightning crackles"
  " along its length"},
{"Emperor wight" , "Your life force is torn from your body as this"
  " powerful unearthly being approaches"},
{"Black wraith" , "A figure that seems made of void, a strangely"
  " human shape cloaked in shadow, it reaches out at you"},
{"Nether wraith" , "A form that hurts the eye, death permeates the"
  " air around it. As it nears you, a coldness saps your soul"},
{"Sorceror" , "A human figure in robes, he moves with magically"
  " improved speed, and his hands are ablur with spell casting"},
{"Ancient white dragon" , "A huge dragonic form, frost covers it from"
  " head to tail"},
{"Ancient green dragon" , "A huge dragonic form enveloped in clouds"
  " of poisonous vapour"},
{"Ancient black dragon" , "A huge dragonic form, pools of acid melt"
  " the floor around it"},
{"Disenchanter worm" , "A strange mass of squirming worms, magical"
  " energy crackles around this disgusting form"},
{"Rotting quylthulg" , "It is a pulsing flesh mound that reeks of death"
  " and putrefaction"},
{"Lesser titan" , "It is a humanoid figure thirty feet tall that gives"
  " off an aura of power and hate"},
{"Solar" , "Never a more heavenly being have you seen. The very holiness"
  " of its presence makes you deeply respect it. Few creatures can"
  " match the powers of a Solar, fewer still live to tell the tale"
  " after attacking one"},
{"Ancient red dragon" , "A huge dragonic form, wisps of smoke steam"
  " from its nostrils and the extreme heat surrounding it makes you"
  " gasp for breath"},
{"Death quasit" , "It is a demon of small stature, but its armoured"
  " frame moves with lightning speed and its powers make it a tornado"
  " of death and destruction"},
{"Ancient multi-hued dragon", "A huge dragonic form, many colours"
  " ripple down its massive frame. Few live to see another"},
{"Erinyes" , "It is a lesser demon of female form, however it takes"
  " little time to show its true colours"},
{"Vrock" , "It is a demon with a long neck and raking claws"},
{"Nalfeshnee" , "It is a large demon with the head of a giant boar."
  " Flames run up and down its length"},
{"Marilith" , "It is a demon of female form with many arms, each"
  " bearing deadly weapons"},
{"Hezrou" , "It is a demon of lizard form with cruel looking jaws"},
{"Grishnakh, the Hill Orc" , "He is a cunning and devious orc with"
  " a chaotic nature"},
{"Beholder" ,"It is a ball-like creature, with one main eye and"
  " twelve smaller eyes on stalks. It has thousands of fangs, beware!"},
{"Shagrat, the Orc Captain","He is an Uruk of power and great cunning"},
{"Azog, King of the Uruk-Hai","He is also known as King of Khazad-dum."
  " His ego is renowned to be bigger than his head"},
{"Draebor, the Imp" ,"An intensely irritating git of a monster"},
{"Rogrog the Black Troll" ,"A massive and cruel troll of great power,"
  " drool slides caustically down his muscular frame. Despite his bulk,"
  " he strikes with stunning speed"},
{"Xaren" ,"It is a tougher relative of the Xorn, its hide glitters"
  " with metal ores"},
{"Minotaur" ,"It is a cross between a human and a bull"},
{"Glabrezu" ,"It is demon with arms and pincers, its form a true"
  " mockery of life"},
{"Lesser balrog" ,"It is a massive humanoid demon wreathed in flames"},
{"Vecna, the Emperor Lich" ,"He is a highly cunning, extremely"
  " magical being, spoken of in legends. This ancient shadow of death"
  " wilts any living thing it passes"},
{"Bullroarer the Hobbit", "He is a sturdy hobbit who is renowned for"
  " his unusual strength and vigour. He can prove a troublesome opponent"},
{"Carcharoth, the Jaws of Thirst","The first guard of Angband,"
  " Carcharoth, also known as 'The Red Maw', is the largest wolf to"
  " ever walk the earth. He is highly intelligent and a deadly opponent"
  " in combat"},
{"Draugluin, Sire of all Werewolves","Draugluin provides Sauron with"
  " a fearsome personal guard. He is an enormous wolf inhabited with"
  " a human spirit, he is chief of all his kind"},
{"Shelob, Spider of Darkness","Shelob is an enormous bloated"
  " spider, rumoured to have been one of the brood of Ungoliant"
  " the Unlight. Her poison is legendary, as is her ego, which maybe"
  " her downfall. She used to guard the pass through Cirith Ungol, but"
  " has not been seen there for many eons"},
{"Thuringwethil","Chief messenger between Sauron and Morgoth, she is"
  " surely the most deadly of her vampire race. At first she is charming"
  " to meet, but her wings and eyes give away her true form"},
{"Ungoliant, the Unlight","This enormous, hideous spirit of void is in"
  " the form of a spider of immense proportions. She is surrounded by"
  " a cloud of Unlight as she sucks in all living light into her"
  " bloated body. She is always ravenously hungry and would even"
  " eat herself to avoid starvation. She is rumoured to have a foul"
  " and deadly breath"},
{"Smaug the Golden","Smaug is one of the Uruloki that still survive, a"
  " fire-drake of immense cunning and intelligence. His speed through"
  " air is matched by few other dragons and his dragon's fire is"
  " what legends are made of"},
{"Glaurung, Father of the Dragons","Glaurung is the father of all"
  " Dragons, and was for a long time the most powerful. He, never the"
  " less, still has full command over his brood and can command them"
  " to appear whenever he so wishes. He is the definition of Dragon"
  "'s fire"},
{"Ancalagon the Black","'Rushing Jaws' is his name, and death is his"
  " game. No dragon of the brood of Glaurung can match him"},
{"Tiamat, Celestial Dragon of Evil","Usually found guarding the first"
  " plane of Hell, Tiamat is a formidable opponent, her five"
  " heads breathing death to all who stand against her"},
{"Muar, the Balrog","A huge balrog, surrounded by raging pillars of"
  " fire, Muar is indeed a terrible opponent. Wielding a great whip of"
  " fire and blazing sword, his fury blisters your skin, melts your flesh"},
{"Lungorthin, the Balrog of White Fire","A massive form, cloaked in"
  " flame. Lungorthin stares balefully at you with eyes that smoulder"
  " red. The dungeon floor where he stands is scorched by the heat of"
  " his body"},
{"Gothmog, the High Captain of Balrogs","Gothmog is the Chief Balrog"
  " in Morgoth's personal guard. He is reknowned for slaying Ecthelion"
  " the Warder of the Gates and he has never been defeated in combat."
  " With his whip of flame and awesome fire breath he saved his master"
  " from Ungoliant's rage"},
{"Sauron, the Sorcerer","He is Morgoth's most powerful servant. Mighty"
  " in spells and enchantments, he created the One Ring. His eyes glow"
  " with power and his gaze seeks to destroy your soul. He has"
  " many servants, and rarely fights without them"},
{"Morgoth, Lord of Darkness","The Big Boss Himself. His figure is like"
  " a black mountain crowned with Lightning. He rages with"
  " everlasting anger, his body scarred by Fingolfin's eight mighty"
  " wounds. He can never rest from his pain, but seeks forever to"
  " dominate all that is light and good in the world. He is the origin"
  " of man's fear of darkness and created many foul creatures with his"
  " evil powers. Orcs, Dragons and Trolls are his most foul"
  " corruptions, causing much pain and suffering in the world to"
  " please him. His disgusting visage, twisted with evil, is crowned"
  " with iron, the two remaining Silmarils forever burning him. Grond,"
  " the mighty Hammer of the Underworld, cries defiance as he"
  " strides towards you to crush you to a pulp"},
{"Quaker, Master of Earth","A huge stone elemental stands before you."
  " The walls and ceiling are reduced to rubble as Quaker advances"},
{"Ariel, Queen of Air","A whirlwind of destruction, Ariel, air"
  " elemental sorceress avoids your blows with her extreme speed"},
{"Waldern, King of Water","A huge water elemental, Waldern is master of"
  " all things liquid. Wave after wave drown your frail body"},
{"Vargo, Tyrant of Fire","A raging pillar of fire, Vargo burns every"
  " living thing beyond recognition"},
{"Smeagol","He's been sneaking. He wants his 'precious'"},
{"Mughash the Kobold Lord","Strong and powerful, for a kobold"},
{"Boldor, King of the Yeeks","A great yeek, powerful in magic and"
  " sorcery, but a yeek all the same"},
{"Bolg, Son of Azog","A large and powerful orc! He looks just like"
  " his daddy. He is tall and fast, but fortunately blessed"
  " with orcish brains"},
{"Ulfast, Son of Ulfang","A short and swarthy Easterling"},
{"Ulwarth, Son of Ulfang","A short and swarthy Easterling"},
{"Uldor the Accursed","An evil and cunning man from the East"},
{"Bert the Stone Troll","Big, brawny, powerful and with a taste for"
  " hobbit. He has friends called Bill and Tom"},
{"Bill the Stone Troll","Big, brawny, powerful and with a taste for"
  " hobbit. He has friends called Bert and Tom"},
{"Tom the Stone Troll" ,"Big, brawny, powerful and with a taste for"
  " hobbit. He has friends called Bert and Bill"},
{"Ulfang the Black","A short and swarthy Easterling dressed in Black"},
{"Castamir the Usurper","A Black Numenorean who usurped the throne"
  " of Gondor, he is treacherous and evil"},
{"Uvatha the Horseman","A tall black cloaked Ringwraith, he is a master"
  " at horsemanship. He longs to taste your blood"},
{"Adunaphel the Quiet","A sorceress in life, Adunaphel quickly fell"
  " under Sauron's sway and the power of the rings"},
{"Akhorahil the Blind","A mighty sorcerer King, Akhorahil was blind"
  " in life. With powerful enchantments, he created jewelled eyes"
  " that enabled him to see better than any ordinary man ever could"},
{"Ren the Unclean","Ren was an insane eastern King who believed himself"
  " to be the son of a volcano god. At an early age his sanity"
  " was destroyed by a plague that wiped out his family, and he"
  " never recovered"},
{"Ji Indur Dawndeath","This Ringwraith was a weakminded sorcerer King"
  " who fell easily under Sauron's power"},
{"Scatha the Worm","Scatha is an ancient and wise Dragon. Scatha has"
  " grown clever over the long years. His scales are covered with frost,"
  " and his breath sends a shower of ice into the air"},
{"Dwar, Dog Lord of Waw","Dwar had a special affinity for dogs in life,"
  " and can still command them at will. He howls manically as he"
  " reaches out to destroy you"},
{"Itangast the Fire Drake","A mighty ancient dragon, Itangast's"
  " form scorches your flesh. Wisps of smoke curl up from his nostrils"
  " as he regards you with disdain"},
{"Hoarmurath of Dir","A Ringwraith powerful in fell sorcery, he yearns"
  " for the life he has lost for a life of everlasting torment"},
{"Khamul the Easterling","A warrior King of the East. Khamul is a"
  " powerful opponent, his skill in combat awesome and his form twisted"
  " by evil cunning"},
{"Saruman of Many Colours","Originally known as the white, Saruman"
  " fell prey to Sauron's wiles. He seeks to emulate him and breeds"
  " orcs and trolls to fight for him. He searches forever for the One"
  " Ring, to become a mighty Sorcerer-King of the world"},
{"Murazor, the Witch-King of Angmar","The Chief of the Ringwraiths. A"
  " fell being of devastating power. His spells are lethal and his"
  " combat blows crushingly hard. He moves at speed, and commands"
  " legions of evil to do his bidding. It is said that he is fated to never"
  " die by the hand of mortal man"},
{"Feagwath the Undead Sorceror","A stench of corruption and decay"
  " surrounds this sorceror, who has clearly risen from the grave"
  " to continue his foul plots and schemes "},
{"Master thief","Cool and confident, fast and lithe; protect"
  " your possessions quickly!"},
{"Master rogue","A thief of great power and shifty speed"},
{"Mithril golem","This is a statue of true-silver! Imagine how much"
  " that would be worth if you could take it home! "},
{"Eog golem","A deep grey statue, which is striding towards you with an"
  " all-too-familiar purpose. Your magic surprisingly feels much"
  " less powerful now"},
{"Colbran","A man-shaped form of living lightning, sparks and"
  " shocks crackle all over this madly capering figure. It leaps and"
  " whirls around and about you....."},
{"Pukelman","A stumpy figure carved from stone, with glittering eyes,"
  " this sentinal strides towards you with deadly intent"},
{"Jabberwock","'Beware the Jabberwock, my son! / The jaws that bite, the"
  " claws that catch!' Run and run quickly, death incarnate chases behind you"},
{"Large kobold","It a man sized figure but with the all too"
  " recognizable face of a kobold"},
{"Black ogre","A massive orc like figure, with black skin and powerful arms"},
{"Cave ogre","A giant orc like figure, with an awesomely muscled frame"},
{"Mumak","A massive elephant form with eyes twisted by madness"},
{"Mumak","A massive elephant form with eyes twisted by madness"},
{"Catoblepas","A strange oxen-like form with a huge head but a thin"
  " weak neck, it looks likes the creation of some deranged alchemist"},
{"Bodak","It is a humanoid form composed of flames and hatred"},
{"Boil-covered wretch","Ugly doesn't begin to describe it"},
{"Spirit naga","A wraithly snakelike form with the torso of a"
  " beautiful woman, it is the most powerful of its kind"},
{"Hummerhorn","A giant buzzing wasp, it's stinger drips venom"},
{"Monadic deva","A lesser angel wearing little more than a loin cloth;"
  " its steely skin providing all the protection it needs"},
{"Movanic deva","A lesser angel protected by an aura of holiness."
  " Its muscular form looks extremely powerful next to your own frail body"},
{"Astral deva","It is an angel moving very quickly, wielding a holy"
  " war hammer and casting a volley of powerful spells in your direction"},
{"Planetar","It is an angel, fast and strong. You are stunned by"
  " its extreme holiness and try to resist all desires to obey it"},
{"Rock lizard","It is a small lizard with a hardened hide"},
{"Small kobold","It is a squat and ugly humanoid figure"},
{"Jackal","It is a yapping snarling dog, dangerous when in a pack"},
{"Grey mold","A small strange growth"},
{"Novice ranger","An agile hunter, ready and relaxed"},
{"Archpriest","An evil priest, dressed all in black. Deadly spells hit"
  " you at an alarming rate as his black spiked mace rains down blow"
  " after blow on your pitiful frame"},
{"Cave lizard","It is an armoured lizard with a powerful bite"},
{"Night lizard","It is a black lizard with overlapping scales and"
  " a powerful jaw"},
{"Death drake","It is a dragonlike form wrapped in darkness, you"
  " cannot make out its true form but you sense its evil"},
{"Dracolich","The skeletal form of a once great dragon, enchanted by"
  " magic most perilous, its animated form strikes with speed and"
  " drains life from its prey to satisfy its hunger"},
{"Shadow drake","It is a dragonlike form wrapped in shadow. Glowing"
  " red eyes shine out in the dark"},
{"Death knight","It is a humanoid form dressed in armour of an"
  " ancient form, from beneath it's helmet, eyes glow a baleful red"
  " and seem to pierce you like lances of fire"},
{"Great storm wyrm","A vast dragon of power, storms and lightning"
  " crash around it's titanic form. Deep blue scales reflect the"
  " flashes and highlight the creature's great muscles. It regards you"
  " with contempt"},
{"Great hell wyrm","A vast dragon of immense power. Fire leaps"
  " continuously from its huge form, the air around it scalds you."
  " Its slightest glance burns you, and you truly realize how"
  " insignificant you are"},
{"Great ice wyrm","An immense dragon capable of awesome destruction."
  " You have never felt such extreme cold, or witnessed such an icy"
  " stare. Begone quickly or feel its wrath"},
{"Shade","A shadowy form clutches at you from the darkness, a"
  " powerful undead with a deadly touch"},
{"Spectre","A phantasmal shrieking spirit, its wail drives the intense"
  " cold of pure evil deep within your body"},
{"Dread","It is a form that screams its presence against the eye."
  " Death incarnate, its hideous black body seems to struggle"
  " against reality as the universe itself struggles to banish it"},
{"Dreadmaster","It is an unlife of power almost unequaled. An affront"
  " to existence it's very touch abuses and disrupts the flow of life"
  " and its unearthly limbs, of purest black, crush rock and flesh with ease"},
{"Dread","It is a form that screams its presence against the eye."
  " Death incarnate, its hideous black body seems to struggle"
  " against reality as the universe itself struggles to banish it"},
{"Dreadlord","It is a massive form of animated death, its colour"
  " deeper than black, it drinks in light, and space around it is"
  " twisted and torn by the weight of its evil. It is unlife and it"
  " knows nothing but the stealing of souls and the stench of death."
  " Flee it's hunger"},
{"Demonist","A figure twisted by evil standing in robes of deepest crimson"},
{"Dark elf","An elven figure with jet black skin and white hair, its"
  " eyes are large and twisted with evil"},
{"Dark elven mage","A drow elven figure dressed all in black, hurling spells"
  " at you"},
{"Dark elven priest","A drow elven figure dressed all in black, chanting"
  " curses and waiting to deliver your soul to hell"},
{"Dark elven lord","A drow elven figure in armour and radiating evil power"},
{"Dark elven warrior","An drow elven figure in armour and ready with his sword"},
{"Dark elven sorceror","A drow elven figure dressed in deepest black,"
  " power seems to crackle from its slender frame"},
{"Drider","A dark elf twisted by the goddess Lolth, a dark elven torso"
  " sits upon the bloated form of a giant spider"},
{"Demonic quylthulg", "A pile of pulsing flesh that glows with an"
  " inner hellish fire, the world seems to cry out against it"},
{"Dragonic quylthulg", "It looks like it was once a dragon corpse,"
  " now deeply infected with magical bacteria that make it pulse in a"
  " foul and degrading way"},
{"Player ghost", "You don't believe what you are seeing"},
{"Aimless looking merchant","The typical ponce around town, with purse"
  " jingling, and looking for more amulets of adornment to buy"},
{"Village idiot","Drooling and comical, but then what do you expect"},
{"Green ooze","It is green and it's oozing"},
{"Blue ooze","It is blue and it's oozing"},
{"Soldier ant","A large ant with powerful mandibles"},
{"Dagashi","A human warrior, moving with lightning speed"},
{"Mind flayer","A humanoid form with a gruesome head, tentacular mouth"
  " and piercing eyes. Claws reach out for you and you feel a"
  " presence invade your mind"},
{"Algroth","A powerful troll form, venom drips from its needlelike claws"},
{"Eldrakyn","A massive troll, larger and stronger than many men"
  " together, and usually a solitary creature"},
{"Hand druj","A skeletal hand floating in the air, motionless except for"
  " its flexing fingers"},
{"Eye druj","A bloodshot eyeball floating in the air, you'd be forgiven"
  " for assuming it as harmless"},
{"Skull druj","A glowing skull possessed by sorcerous power, it need"
  " not move, but just blasts you with mighty magic"},
{"Young gold dragon", "It has a form that legends are made of. Its"
  " still tender scales are a tarnished gold hue, and light is"
  " reflected from its form"},
{"Young bronze dragon", "It has a form that legends are made of. Its"
  " still tender scales are a rich bronze hue, and its shape masks"
  " its true form"},
{"Mature gold dragon","A large Dragon, with scales of gleaming"
  " gold"},
{"Mature bronze dragon","A large Dragon, with scales of rich bronze"},
{"Ancient gold dragon","A huge Dragonic form, wreathed in a nimbus of light"},
{"Ancient bronze dragon","A huge Dragonic form, enveloped in a cascade"
  " of colour"},
{"Crystal drake","A dragon of strange crystalline form, light"
  " shines through it, dazzling your eyes with spectrums of colour"},
{"Great crystal drake","A huge crystalline Dragon. Its claws could cut"
  " you to shreds and its teeth are razor sharp. Strange colours"
  " ripple through it as it moves in the light"},
{"Patriarch","A dark priest of the highest order. Powerful and evil,"
  " beware his many spells"},
{"Salamander","A black and yellow lizard.. WATCH OUT"},
{"Giant salamander","A large black and yellow lizard. You'd better run away"},
{"Clear worm mass","A disgusting mass of poisonous worms"},
{"Great wyrm of chaos","A massive Dragon of changing form. As you watch,"
  " it appears first fair and then foul. Its body is twisted by"
  " chaotic forces as it strives to stay real. Its very existence"
  " distorts the universe around it"},
{"Great wyrm of law","A massive Dragon of powerful intellect. It seeks"
  " to dominate the universe and despises all other life. It sees all"
  " who do not obey it as merely insects to be crushed underfoot"},
{"Giant tarantula","A giant spider with hairy black and red legs."},
{"Giant gold dragon fly","Large beating wings support this dazzling"
  " insect, as they beat a loud buzzing noise pervades the air"},
{"Giant bronze dragon fly","This vast gleaming bronze fly has wings"
  " which beat mesmerically fast"},
{"Giant black dragon fly","The size of a large bird, this fly drips"
  " caustic acid"},
{"Fruit bat","A fast-moving pest"},
{"Giant army ant","An armoured form moving with purpose, powerful on"
  " its own; flee when hordes of them march"},
{"Gorbag, the Orc Captain","A gruesomely ugly but cunning orc, his"
  " eyes regard you with hatred and powerful arms flex menacingly as"
  " he advances"},
{"Light hound","A brilliant canine form whose light hurts your eyes,"
  " even at this distance"},
{"Dark hound","A hole in the air in the shape of a huge hound. No"
  " light falls upon this form"},
{"Fire hound","Flames lick at its feet and its tongue is a blade of"
  " fire. You can feel a furnace heat radiating from the creature"},
{"Cold hound","A hound as tall as a man, this creature appears to"
  " be composed of angular planes of ice. Cold radiates from it and"
  " freezes your breath in the air"},
{"Energy hound","Saint Elmo's Fire forms a ghostly halo around this"
  " hound, and sparks sting your fingers as energy builds up in the"
  " air around you"},
{"Vibration hound","A blurry canine form which seems to be moving as"
  " fast as the eye can follow. You can feel the earth resonating"
  " beneath your feet"},
{"Water hound","Liquid footprints follow this hound as it pads around"
  " the dungeon. An acrid smell of acid rises from the dog's pelt"},
{"Air hound","Swirling vapours surround this beast as it floats"
  " towards you, seemingly walking on air. Noxious gases sting your throat"},
{"Earth hound","A beautiful crystalline shape does not disguise the"
  " danger this hound clearly presents. Your flesh tingles as"
  " it approaches..."},
{"Impact hound","A deep blue shape is visible before you, its canine"
  " form strikes you with an almost physical force. The dungeon"
  " floor buckles as if struck by a powerful blow as it stalks towards you"},
{"Inertia hound","Bizarrely, this hound seems to be hardly moving at"
  " all, yet it approaches you with deadly menace. It makes you tired"
  " just to look at it"},
{"Time hound","You get a terrible sense of deja vu, or is it a"
  " premonition? All at once you see a little puppy and a toothless"
  " old dog. Perhaps you should give up and go to bed now?"},
{"Nether hound","You feel a soul-tearing chill upon viewing this beast,"
  " a ghostly shape of darkness. You think it may be a dog, but it"
  " makes you feel weaker just to look at it..."},
{"Nexus hound","A locus of conflicting points coalesce to form the"
  " vague shape of a huge hound. Or is it really there? Anyway, it seems"
  " to be coming towards you..."},
{"Plasma hound","The very air warps as pure elemental energy stalks"
  " towards you in the shape of a giant hound. Your hair stands on end"
  " and your palms itch as you sense trouble..."},
{"Aether hound","A shifting, swirling form. It seems to be all colours"
  " and sizes and shapes, though the dominant form is that of a huge"
  " dog. You feel very uncertain all of a sudden"},
{"Chaos hound","A constantly changing canine form, these hounds"
  " rush towards you as if expecting mayhem and chaos ahead. They appear"
  " to have an almost kamikaze relish for combat. You suspect all may"
  " not be as it seems..."},
{"Gravity hound","Unfettered by the usual constraints of gravity,"
  " these unnatural creatures are walking on the walls and even"
  " the ceiling! The earth suddenly feels rather less solid as you"
  " see gravity warp all round the monsters"},
{"Fire vortex","A whirling maelstrom of fire"},
{"Cold vortex","A twisting whirlpool of frost"},
{"Energy vortex","A shimmering tornado of air, sparks crackle along"
  " its length"},
{"Water vortex","A caustic spinning tower of water"},
{"Nexus vortex","A maelstrom of potent magical energy"},
{"Chaos vortex","Void, nothingness, spinning destructively"},
{"Aether vortex","An awesome vortex of pure magic, power radiates from its"
  " frame"},
{"Plasma vortex","A whirlpool of intense flame, charring the stones at"
  " your feet"},
{"Time vortex","You haven't seen it yet"},
{"Acidic cytoplasm","A disgusting animated blob of destruction, flee its"
  " gruesome hunger!"},
{"Black reaver","A humanoid form. Black as night. Advancing steadily"
  " and unstoppably. Flee"},
{"Great wyrm of balance","This massive Dragon is the mightiest"
  " of Dragonkind. It is thousands of years old and seeks to maintain"
  " the Balance. It sees you as an upstart troublemaker without the"
  " wisdom to control your actions. It will destroy you"},
{"Ugluk, the Uruk","Another of Morgoth's servants, this Orc is"
  " strong and cunning. He is ugly and scarred from many power struggles"},
{"Omarax the Eye Tyrant","A vast baleful eye floating in the air. His"
  " gaze seems to shred your soul and his spells crush your will. He"
  " is ancient, his history steeped in forgotten evils, his"
  " atrocities numerous and sickening"},
{"Wormtongue, Agent of Saruman","He's been spying for Saruman. He is"
  " a snivelling wretch with no morals and disgusting habits"},
{"Cantoras, the Skeletal Lord","A legion of evil undead druj animating"
  " the skeleton of a once mighty sorcerer. His power is devastating"
  " and his speed unmatched in the underworld. Flee his wrath"},
{"2-headed hydra","A strange reptilian hybrid with 2 heads, guarding"
  " its hoard"},
{"3-headed hydra","A strange reptilian hybrid with 3 heads, guarding"
  " its hoard"},
{"4-headed hydra","A strange reptilian hybrid with 4 heads, guarding"
  " its hoard"},
{"5-headed hydra","A strange reptilian hybrid with 5 heads"
  " dripping venom"},
{"7-headed hydra","A strange reptilian hybrid with 7 heads dripping venom"},
{"9-headed hydra","A strange reptilian hybrid with 9 smouldering heads"},
{"11-headed hydra","A strange reptilian hybrid with 11 smouldering heads"},
{"The Lernean Hydra","A massive legendary hydra. It has 12 powerful"
  " heads. Its many eyes stare at you as clouds of smoke and"
  " poisonous vapour rise from its seething form"},
{"Vampire bat","An undead bat that flies at your neck hungrily"},
{"Shimmering vortex","A strange pillar of shining light that hurts"
  " your eyes. Its shape changes constantly as it cuts through the"
  " air towards you. It is like a beacon, waking monsters from their slumber"},
{"White wolf","A large and muscled wolf from the northern wastes."
  " Its breath is cold and icy and its fur coated in frost"},
{"Wereworm","A huge wormlike shape dripping acid, twisted by evil"
  " sorcery into a foul monster that breeds on death"},
{"Mirkwood spider","A strong and powerful spider from Mirkwood"
  " forest. Cunning and evil, it seeks to taste your juicy"
  " insides"},
{"Shadow","A mighty spirit of darkness of vaguely humanoid form."
  " Razor-edged claws reach out to end your life as it glides towards"
  " you, seeking to suck the energy from your soul to feed its power"},
{"Ufthak of Cirith Ungol","A strong orc guarding the Pass of Cirith"
  " Ungol. He is mortally afraid of spiders"},
{"Golfimbul, the Hill Orc Chief","A leader of a band of raiding orcs,"
  " he picks on hobbits"},
{"Mim, Betrayer of Turin","The last of his race, Mim is a petty"
  " dwarf. Petty dwarves are strange creatures, powerful in sorcery"
  " and originating in the East. They were hunted to extinction by high elves"},
{"Novice mage","He is leaving behind a trail of dropped spell components"},
{"Gnome mage","A mage of short stature"},
{"Phantom","An unholy creature of darkness, the aura emanating from"
  " this evil being saps your very soul"},
{"Filthy street urchin","It looks squalid and thoroughly revolting"},
{"Carrion crawler","A hideous centipede covered in slime and with"
  " glowing tentacles around its head"},
{"Carrion crawler","A hideous centipede covered in slime and with"
  " glowing tentacles around its head"},
{"Blink dog","A strange magical member of the canine race, its form"
  " seems to shimmer and fade in front of your very eyes"},
{"Phase spider","A spider that never seems quite there. Everywhere you"
  " look it is just half-seen, in the corner of one eye"},
{"Blue icky thing","A strange icky creature with rudimentary"
  " intelligence but evil cunning. It hungers for food, and you look tasty"},
{"Basilisk","An evil reptile that preys on unsuspecting travellers."
  " Its eyes stare deeply at you...Your soul starts to wilt.. look"
  " away.."}, 
{"Ethereal hound","A pale white hound. You can see straight through"
  " it. Pulsing red lines and strange fluorescent light hints at"
  " internal organs best left to the imagination"},
{"Novice archer","A nasty little fellow with a bow and arrow"},
{"Undead beholder","A huge eyeball that floats in the air. Black"
  " nether storms rage around its bloodshot pupil and light seems to"
  " bend as it sucks its power from the very air around it. Your"
  " soul chills as it drains your vitality for its evil enchantments"},
{"Will o' the wisp","A strange ball of glowing light. It disappears"
  " and reappears and seems to draw you to it. You seem somehow"
  " compelled to stand still and watch its strange dancing motion"},
{"Nightwing","Everywhere colours seem paler and the air chiller. At"
  " the centre of the cold stands a mighty figure. Its wings envelop you"
  " in the chill of death as the Nightwing reaches out to draw you"
  " into oblivion. Your muscles sag and your mind loses all will to"
  " fight as you stand in awe of this mighty being"},
{"Nightcrawler","This intensely evil creature bears the form of"
  " a gargantuan black worm. Its gaping maw is a void of blackness,"
  " acid drips from its steely hide. It is like nothing you have ever"
  " seen before, and a terrible chill runs down your spine as you face it..."},
{"Nightwalker","A huge giant garbed in black, more massive than a titan"
  " and as strong as a dragon. With terrible blows, it breaks your"
  " armour from your back, leaving you defenseless against its evil"
  " wrath. It can smell your fear, and you in turn smell the awful"
  " stench of death as this ghastly figure strides towards you menacingly"},
{"Death mold","It is the epitome of all that is evil, in a mold."
  " Its lifeless form draws power from sucking the souls of those"
  " that approach it, a nimbus of pure evil surrounds it. However... it"
  " can't move..."},
{"Harowen the Black Hand","He is a master of disguise, an expert"
  " of stealth, a genius at traps and moves with blinding speed."
  " Better check your pockets just in case..."},
{"Fundin Bluecloak","He is one of the greatest dwarven priests to walk"
  " the earth. Fundin has earned a high position in the church, and"
  " his skill with both weapon and spell only justify his position"
  " further. His combination of both dwarven strength and priestly"
  " wisdom are a true match for any adventurer"},
{"Master mystic","A lord of all that is natural, skilled in the"
  " mystic ways. It is a master of martial arts and is at one with"
  " nature, able to summon help from the wild if need be"},
{"Grand master mystic","It is one of the few true masters of the art,"
  " being extremely skillful in all forms of unarmed combat and"
  " controlling the world's natural creatures with disdainful ease"},
{"Mystic","An adept at unarmed combat, the Mystic strikes with"
  " stunning power. He can summon help from nature and is able to focus"
  " his power to ease any pain"},
{"Novice priest","He is tripping over his priestly robes"},
{"Novice warrior","He looks inexperienced but tough"},
{"Novice rogue","A rather shifty individual"},
{"Novice ranger","An agile hunter, ready and relaxed"},
{"Medusa, the Gorgon","One of the original three ugly sisters. Her"
  " face could sink a thousand ships. Her scales rattle as she"
  " slithers towards you, venom dripping from her ghastly mouth"},
{"Grip, Farmer Maggot's dog","A rather vicious dog belonging to"
  " Farmer Maggot. It thinks you are stealing mushrooms"},
{"Fang, Farmer Maggot's dog","A rather vicious dog belonging to"
  " Farmer Maggot. It thinks you are stealing mushrooms"},
{"Ibun, Son of Mim","One of the last of the petty dwarves. Ibun is a"
  " tricky sorcerous little being, full of mischief"},
{"Khim, Son of Mim","One of the last of the petty dwarves. Khim is a"
  " tricky sorcerous little being, full of mischief"},
{"Nether worm mass","A disgusting pile of dark worms, eating each"
  " other, the floor, the air, you..."},
{"Orfax, Son of Boldor","He's just like daddy! He knows mighty spells,"
  " but fortunately he is a yeek"},
{"Lagduf, the Snaga","A captain of a regiment of weaker orcs, Lagduf"
  " keeps his troop in order with displays of excessive violence"},
{"Lorgan, Chief of the Easterlings","A mighty warrior from the east,"
  " Lorgan hates everything that he cannot control"},
{"Lugdush, the Uruk","A strong and cunning orc warrior, Lugdush"
  " sneers as he insults your mother"},
{"Astral hound","A pale white hound. You can see straight through"
  " it. Pulsing red lines and strange fluorescent light hints at"
  " internal organs best left to the imagination"},
{"Enchantress","This elusive female spellcaster has a special affinity"
  " for dragons, whom she rarely fights without"},
{"Dark elven druid","A powerful drow, with mighty nature"
  " controlling enchantments"},
{"Sasquatch","A tall shaggy, furry humanoid, he could call the"
  " Yeti brother"},
{"Colossus","An enormous construct resembling a Titan made from stone."
  " It strides purposefully towards you, swinging its slow fists with"
  " earth shattering power"},
{"Drolem","A constructed dragon, the Drolem has massive strength."
  " Powerful spells weaved during its creation make it a"
  " fearsome adversary. Its eyes show little intelligence, but it has"
  " been instructed to destroy all it meets"},
{"The Mouth of Sauron","The Mouth of Sauron is a mighty spell caster."
  " So old that even he cannot remember his own name, his power and"
  " evil are undeniable. He believes unshakeably that he is unbeatable"
  " and laughs as he weaves his awesome spells"},
{"Wererat","A large rat with glowing red eyes. The wererat is a"
  " disgusting creature, relishing in filth and disease"},
{"Magic mushroom patch","Yum! It looks quite tasty.  But, wait... It seems"
  " to glow with an unusual light"},
{"Ettin","A massive troll of huge strength. Ettins are stupid but violent"},
{"Half-troll","A huge ugly half-human in search of plunder"},
{"The Queen Ant","She's upset because you hurt her children"},
{"Troll priest","A troll who is so bright he knows how to read"},
{"Spirit troll","A weird troll from the elemental planes"},
{"Hippogriff","A strange hybrid of eagle, lion and horse. It looks weird"},
{"Gorgimera","The result of evil"
  " experiments, this travesty of nature should never be alive. It has"
  " 3 heads - gorgon, goat and dragon - all attached to a lion's body"},
{"Kavlax the Many-Headed","A Large dragon with a selection of heads,"
  " all shouting and arguing as they look for prey, but each with its"
  " own deadly breath weapon"},
{"Baphomet the Minotaur Lord","A fearsome bull-headed demon,"
  " Baphomet swings a mighty axe as he curses all that defy him"},
{"Dracolisk","A mixture of dragon and basilisk, the Dracolisk stares at"
  " you with deep piercing eyes, its evil breath burning the ground"
  " where it stands"},
{"Shambling mound","A pile of rotting vegetation that slides towards"
  " you with a disgusting stench, waking all it nears"},
{"Nar, the Dwarf","This dwarf became so obsessed by gold that"
  " Morgoth tricked him into betraying his friends"},
{"Cerberus, Guardian of Hades","A 2 Headed hell hound of fearsome"
  " aspect. Flame burns merrily from its hide as it snarls and roars"
  " its defiance"},
{"Sangahyando of Umbar","A Black Numenorean with a blacker heart"},
{"Angamaite of Umbar","A Black Numenorean who hates the men of the west"},
{"Druid","A mystic being at one with nature."},
{"Pseudo dragon","A small relative of the dragon that inhabits dark caves"},
{"Chaos drake","A dragon twisted by the forces of chaos. It seems"
  " first ugly, then fair, as its form shimmers and changes in front"
  " of your eyes"},
{"Law drake","This dragon is clever and cunning. It laughs at your"
  " puny efforts to disturb it"},
{"Balance drake","A mighty dragon, the Balance drake seeks to maintain"
  " the Balance, and despises your feeble efforts to destroy evil"},
{"Baby white dragon","This hatchling dragon is still soft, its"
  " eyes unaccustomed to light and its scales a pale white"},
{"Baby blue dragon","This hatchling dragon is still soft, its"
  " eyes unaccustomed to light and its scales a pale blue"},
{"Baby black dragon","This hatchling dragon is still soft, its"
  " eyes unaccustomed to light and its scales a dull black"},
{"Baby green dragon","This hatchling dragon is still soft, its"
  " eyes unaccustomed to light and its scales a sickly green"},
{"Baby red dragon","This hatchling dragon is still soft, its"
  " eyes unaccustomed to light and its scales a pale red"},
{"Baby multi-hued dragon","This hatchling dragon is still soft, its"
  " eyes unaccustomed to light and its scales shimmering with a hint"
  " of colour"},
{"Gorlim, Betrayer of Barahir","This once mighty warrior was so"
  " dominated by Morgoth's power that he became little more than a"
  " mindless creature of evil"},
{"Creeping adamantite coins","A mass of shining coins slithering"
  " towards you.. Quick! Pick it up and put it in your pocket"},
{"Black pudding","A lump of rotting black flesh that slurrrrrrrps"
  " across the dungeon floor"},
{"Silver jelly","It is a large pile of silver flesh that sucks all light"
  " from its surroundings "},
{"Uriel, Angel of Fire","A creature of godly appearance, you dare"
  " not challenge Uriel's supremacy. Those who stood against him before"
  " are but a memory, cremated by his mastery of elemental"
  " fire"},
{"Azriel, Angel of Death","Azriel commands awesome power, his visage"
  " holy enough to shrivel your soul. You shriek with disbelief as"
  " his mastery of death draws you to your grave. It is truly beyond"
  " all but the mightiest of warriors to stand against him and live"},
{"Gabriel, the Messenger","Commanding a legion of Solars, Gabriel"
  " will destroy you for your sins. He will crush you like the"
  " pitiful insignificant being he sees you to be. Your very soul"
  " will be taken into judgement by his supreme authority, as he cleanses the"
  " world of evil"},
{"Ethereal dragon","A huge Dragon emanating from the elemental plains,"
  " the Ethereal dragon is a master of light and dark. Its form"
  " disappears from sight as it cloaks itself in unearthly shadows"},
{"Ethereal drake","A dragon of elemental power, with control over light"
  " and dark, the Ethereal drake's eyes glare with white hatred from"
  " the shadows"},
{"Tselakus, the Dreadlord","This huge affront to existence twists and"
  " tears at the fabric of space. A master of mighty magic,"
  " Tselakus hungers for your tender flesh. Darkness itself recoils"
  " from the touch of Tselakus as he leaves a trail of death"
  " and destruction. Tselakus is a being of sneering"
  " contempt, laughing at your pitiful efforts to defy him. Mighty"
  " claws rend reality as he annihilates all in his path to your soul"},
{"Ogre mage","A hideous ogre wrapped in black sorcerous robes"},
{"Greater titan","A forty foot tall humanoid that shakes the ground as"
  " it walks. The power radiating from its frame shakes your courage,"
  " its hatred inspired by your defiance"},
{"Night mare","A fearsome skeletal horse with glowing eyes, that watch"
  " you with little more than a hatred of all that lives"},
{"Lokkak, the Ogre Chieftan","An Ogre reknowned for acts of"
  " surpassing cruelty, Lokkak quickly became the leader of a large band"
  " of violent ogres"},
{"The Tarrasque","The Tarrasque is a massive reptile of legend, rumoured"
  " to be unkillable and immune to magic. Fear its anger, for"
  " its devastation is unmatched"},
{"Brodda, the Easterling","A nasty piece of work, Brodda picks"
  " on defenseless women and children"},
{"Master quylthulg","A pulsating mound of flesh, shining with silver"
  " pulses of throbbing light"},
{"Greater dragonic quylthulg","A massive mound of scaled flesh,"
  " throbbing and pulsating with multi-hued light"},
{"Greater rotting quylthulg","A massive pile of rotting flesh. A"
  " disgusting stench fills the air as it throbs and writhes"},
{"The Emperor Quylthulg","A huge seething mass of flesh with a"
  " rudimentary intelligence, the Emperor Quylthulg changes colours"
  " in front of your eyes. Pulsating first one colour then the next,"
  " it knows only it must bring help to protect itself"},
{"Qlzqqlzuup, the Lord of Flesh","This disgusting creature squeals"
  " and snorts as it writhes on the floor. It pulsates with evil,"
  " its intent to overwhelm you with monster after monster, until it"
  " can greedily dine on the remains"},
{"Mimic","A strange creature that disguises itself as discarded objects"
  " to lure unsuspecting adventurers within reach of its venomous claws"},
{"Mimic","A strange creature that disguises itself as discarded objects"
  " to lure unsuspecting adventurers within reach of its venomous claws"},
{"Mimic","A strange creature that disguises itself as discarded objects"
  " to lure unsuspecting adventurers within reach of its venomous claws"},
{"Scrawny cat","A skinny little furball with sharp claws and a menacing look"},
{"Scruffy little dog","A thin flea-ridden mutt, growling as you get close"},
{"Wild cat","A larger than normal feline, hissing loudly. Its velvet"
  " claws conceal a fistful of needles"},
{"Panther","A large black cat, stalking you with intent. It thinks you"
  "'re its next meal"},
{"Tiger","One of the largest of its species, a sleek orange and black"
  " shape creeps towards you, ready to pounce"},
{"Sabre-tooth tiger","A fierce and dangerous cat, its huge tusks and"
  " sharp claws would lacerate even the strongest armour"},
{"Farmer Maggot","He's lost his dogs. He's had his mushrooms stolen. He"
  "'s not a happy hobbit!"},
{"Displacer beast","It is a huge black panther, clubbed tentacles"
  " sprouting from its shoulders"},
{"Memory moss","A mass of green vegetation, you don't remember"
  " seeing anything like it before..."},
{"Lurker","A strange creature that merges with the dungeon floor,"
  " trapping its victims by enveloping them within its perfectly"
  " disguised form"},
{"Trapper","A larger cousin of the lurker, this creature traps"
  " unsuspecting victims and paralyzes them, to be slowly digested later"},
{"The Cat Lord","Master of all things feline, the Cat Lord moves"
  " with catlike stealth... Miaow!"},
{"Giant roc","A vast legendary bird, its iron talons rake the"
  " most impenetrable of surfaces and its screech echoes through the"
  " many winding dungeon corridors"},
{"The Phoenix","A massive glowing eagle bathed in flames, the searing"
  " heat chars your skin and melts your armour"},
{"Pazuzu, Lord of Air","A winged humanoid from the Planes of Hell,"
  " Pazuzu grins inhumanely at you as he decides your fate"},
{"Novice paladin","An adventurer both devoutly religous and skillful"
  " in combat"},
};
