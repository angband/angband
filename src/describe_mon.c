#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"

describe_mon_type desc_list[MAX_CREATURES] = {

{"Filthy street urchin", "He looks squalid and thoroughly revolting.", 'm'},
{"Filthy street urchin", "He looks squalid and thoroughly revolting.", 'm'},
{"Scrawny cat","A skinny little furball with sharp claws and a menacing look.",
 'n'},
{"Scruffy little dog","A thin flea-ridden mutt, growling as you get close.",
 'n'},
{"Farmer Maggot","He's lost his dogs.  He's had his mushrooms stolen.  He"
  "'s not a happy hobbit!", 'm'},
{"Blubbering idiot", "He tends to blubber a lot.", 'm'},
{"Boil-covered wretch", "Ugly doesn't begin to describe him.", 'm'},
{"Village idiot","Drooling and comical, but then, what do you expect?", 'm'},
{"Pitiful looking beggar", "You just can't help feeling sorry for him.", 'm'},
{"Mangy looking leper", "You feel it isn't safe to touch him.", 'm'},
{"Squint eyed rogue", "A hardy, street-wise crook that knows an easy"
  " catch when it sees one.", 'm'},
{"Singing, happy drunk", "He makes you glad to be sober.", 'm'},
{"Aimless looking merchant","The typical ponce around town, with purse"
  " jingling, and looking for more amulets of adornment to buy.", 'm'},
{"Mean looking mercenary", "No job is too low for him.", 'm'},
{"Battle scarred veteran", "He doesn't take to strangers kindly.", 'm'},
{"Grey mold","A small strange growth.", 'n'},
{"Grey mushroom patch", "Yum!  It looks quite tasty.", 'n'},
{"Giant yellow centipede", "It is about four feet long and carnivorous.", 'n'},
{"Giant white centipede", "It is about four feet long and carnivorous.", 'n'},
{"White icky thing", "It is a sort of pile of slime on legs.", 'n'},
{"Clear icky thing", "It is a smallish, slimy, icky, blobby creature.", 'n'},
{"Giant white mouse", "It is about three feet long with large teeth.", 'n'},
{"Large brown snake", "It is about eight feet long.", 'n'},
{"Large white snake", "It is about eight feet long.", 'n'},
{"Small kobold","It is a squat and ugly humanoid figure.", 'n'},
{"Kobold", "It is a small, dog-headed humanoid.", 'n'},
{"White worm mass", "They are a large slimy pile of worms.", 'p'},
{"Floating eye" , "A disembodied eye floating a few feet above the ground.",
 'n'},
{"Rock lizard","It is a small lizard with a hardened hide.", 'n'},
{"Jackal","It is a yapping snarling dog, dangerous when in a pack.", 'n'},
{"Soldier ant","A large ant with powerful mandibles.", 'n'},
{"Fruit bat","A fast-moving pest.", 'n'},
{"Shrieker mushroom patch", "Yum!  These look quite tasty.", 'n'},
{"Blubbering icky thing", "It is a smallish, slimy, icky creature.", 'n'},
{"Metallic green centipede", "It is about four feet long and carnivorous.",
 'n'},
{"Novice warrior" , "He looks inexperienced but tough.", 'm'},
{"Novice rogue" , "A rather shifty individual.", 'm'},
{"Novice priest" , "He is tripping over his priestly robes.", 'm'},
{"Novice mage" , "He is leaving behind a trail of dropped spell components.",
 'm'},
{"Yellow mushroom patch", "Yum!  It looks quite tasty.", 'n'},
{"White jelly" , "Its a large pile of white flesh.", 'n'},
{"Giant green frog" , "It is as big as a wolf.", 'n'},
{"Giant black ant" , "It is about three feet long.", 'n'},
{"Salamander","A black and yellow lizard....WATCH OUT!", 'n'},
{"White harpy" , "A flying, screeching bird with a woman's face.", 'f'},
{"Blue yeek" , "A small humanoid figure.", 'n'},
{"Grip, Farmer Maggot's dog","A rather vicious dog belonging to"
  " Farmer Maggot.  It thinks you are stealing mushrooms.", 'n'},
{"Fang, Farmer Maggot's dog","A rather vicious dog belonging to"
  " Farmer Maggot.  It thinks you are stealing mushrooms.", 'n'},
{"Green worm mass" , "They are a large slimy pile of worms.", 'p'},
{"Large black snake" , "It is about ten feet long.", 'n'},
{"Cave spider" , "It is a black spider that moves in fits and starts.", 'n'},
{"Wild cat","A larger than normal feline, hissing loudly.  Its velvet"
  " claws conceal a fistful of needles.", 'n'},
{"Smeagol","He's been sneaking, and he wants his 'precious.'", 'm'},
{"Green ooze","It's green and it's oozing.", 'n'},
{"Poltergeist" , "It is a ghastly, ghostly form.", 'n'},
{"Metallic blue centipede", "It is about four feet long and carnivorous.", 'n'},
{"Giant white louse" , "It is six inches long.", 'n'},
{"Black naga" , "A large black serpent's body with a female torso.", 'f'},
{"Spotted mushroom patch", "Yum!  It looks quite tasty.", 'n'},
{"Silver jelly","It is a large pile of silver flesh that sucks all light"
  " from its surroundings.", 'n'},
{"Yellow jelly" , "It's a large pile of yellow flesh.", 'n'},
{"Scruffy looking hobbit" , "A short little guy, in bedraggled clothes.  He"
  " asks you if you know of a good tavern nearby.", 'm'},
{"Giant white ant" , "It is about two feet long and has sharp pincers.", 'n'},
{"Yellow mold" , "It is a strange growth on the dungeon floor.", 'n'},
{"Metallic red centipede", "It is about four feet long and carnivorous.", 'n'},
{"Yellow worm mass" , "They are a large slimy pile of worms.", 'p'},
{"Clear worm mass","They are a disgusting mass of poisonous worms.", 'p'},
{"Radiation eye" , "A glowing eye that seems to crackle with energy.", 'n'},
{"Cave lizard","It is an armoured lizard with a powerful bite.", 'n'},
{"Novice ranger","An agile hunter, ready and relaxed.", 'm'},
{"Novice paladin","An adventurer both devoutly religious and skillful"
  " in combat.", 'm'},
{"Blue jelly" , "It's a large pile of pulsing blue flesh.", 'n'},
{"Creeping copper coins" , "A strange pile of moving coins.", 'p'},
{"Giant white rat" , "It is a very vicious rodent.", 'n'},
{"Blue worm mass" , "They are a large slimy pile of worms.", 'p'},
{"Large grey snake" , "It slithers and moves towards you.", 'n'},
{"Bullroarer the Hobbit", "He is a sturdy hobbit who is renowned for"
  " his unusual strength and vigour.  He can prove a troublesome opponent.",
 'm'},
{"Novice mage","He is leaving behind a trail of dropped spell components.",
 'm'},
{"Green naga" , "A large green serpent with a female's torso.  Her"
  " green skin glistens with acid.", 'f'},
{"Blue ooze","It's blue and it's oozing.", 'n'},
{"Green glutton ghost" , "It is a very ugly green ghost with a"
  " voracious appetite.", 'n'},
{"Green jelly" , "It is a large pile of pulsing green flesh.", 'n'},
{"Large kobold","It a man-sized figure with the all too"
  " recognizable face of a kobold.", 'n'},
{"Skeleton kobold" , "It is a small animated kobold skeleton.", 'n'},
{"Grey icky thing" , "It is a smallish, slimy, icky creature.", 'n'},
{"Disenchanter eye" , "A large white floating eye that crackles with magic.",
 'n'},
{"Red worm mass" , "They are a large slimy pile of worms.", 'p'},
{"Copperhead snake" , "It has a copper head and sharp venomous fangs.", 'n'},
{"Purple mushroom patch" , "Yum!  It looks quite tasty.", 'n'},
{"Novice priest","He is tripping over his priestly robes.", 'm'},
{"Novice warrior","He looks inexperienced but tough.", 'm'},
{"Novice rogue","A rather shifty individual.", 'm'},
{"Brown mold" , "A strange brown growth on the dungeon floor.", 'n'},
{"Giant brown bat" , "It screeches as it attacks.", 'n'},
{"Novice archer","A nasty little fellow with a bow and arrow.", 'm'},
{"Creeping silver coins" , "A pile of silver coins that moves on"
  " thousands of tiny legs.", 'p'},
{"Snaga" , "He is one of the many weaker 'slave' orcs, often"
  " mistakenly known as a goblin.", 'm'},
{"Rattlesnake" , "It is recognized by the hard-scaled end of its body"
  " that is often rattled to frighten its prey.", 'n'},
{"Cave orc" , "He is often found in huge numbers in deep caves.", 'm'},
{"Wood spider" , "It creeps towards you.", 'n'},
{"Manes" , "It is a minor but aggressive demon.", 'n'},
{"Bloodshot eye" , "A large floating bloodshot eye.", 'n'},
{"Red naga" , "A large red snake with a woman's torso.", 'f'},
{"Red jelly" , "It is a large pulsating mound of red flesh.", 'n'},
{"Giant red frog" , "It looks poisonous.", 'n'},
{"Green icky thing" , "It is a smallish, slimy, icky creature.", 'n'},
{"Zombie kobold" , "It is an animated kobold corpse.  Flesh falls off"
  " in large chunks as it shambles forward.", 'n'},
{"Lost soul" , "It is almost insubstantial.", 'n'},
{"Dark elf","An elven figure with jet black skin and white hair, his"
  " eyes are large and twisted with evil.", 'm'},
{"Night lizard","It is a black lizard with overlapping scales and"
  " a powerful jaw.", 'n'},
{"Mughash the Kobold Lord","Strong and powerful, for a kobold.", 'm'},
{"Wormtongue, Agent of Saruman","He's been spying for Saruman.  He is"
  " a snivelling wretch with no morals and disgusting habits.", 'm'},
{"Lagduf, the Snaga","A captain of a regiment of weaker orcs, Lagduf"
  " keeps his troop in order with displays of excessive violence.", 'm'},
{"Brown yeek" , "It is a strange small humanoid.", 'n'},
{"Novice ranger","An agile hunter, ready and relaxed.", 'm'},
{"Giant salamander","A large black and yellow lizard.  You'd better run away!",
 'n'},
{"Green mold" , "It is a strange growth on the dungeon floor.", 'n'},
{"Skeleton orc" , "It is an animated orc skeleton.", 'n'},
{"Seedy looking human" , "He is covered in scars and looks disreputable.",
 'm'},
{"Lemure" , "It is the larval form of a major demon.", 'n'},
{"Hill orc" , "He is a hardy well-weathered survivor.", 'm'},
{"Bandit" , "He is after your cash!", 'm'},
{"Yeti" , "A large white figure covered in shaggy fur.", 'n'},
{"Bloodshot icky thing" , "It is a slimy, icky creature.", 'n'},
{"Giant grey rat" , "It is a rodent of unusual size.", 'n'},
{"Black harpy" , "A woman's face on the body of a vicious black bird.", 'f'},
{"Orc shaman" , "An orc dressed in skins who gestures wildly.", 'm'},
{"Baby blue dragon","This hatchling dragon is still soft, its"
  " eyes unaccustomed to light and its scales a pale blue.", 'n'},
{"Baby white dragon","This hatchling dragon is still soft, its"
  " eyes unaccustomed to light and its scales a pale white.", 'n'},
{"Baby green dragon","This hatchling dragon is still soft, its"
  " eyes unaccustomed to light and its scales a sickly green.", 'n'},
{"Baby black dragon","This hatchling dragon is still soft, its"
  " eyes unaccustomed to light and its scales a dull black.", 'n'},
{"Baby red dragon","This hatchling dragon is still soft, its"
  " eyes unaccustomed to light and its scales a pale red.", 'n'},
{"Giant red ant" , "It is large and has venomous mandibles.", 'n'},
{"Brodda, the Easterling","A nasty piece of work, Brodda picks"
  " on defenseless women and children.", 'm'},
{"King cobra" , "It is a large snake with a hooded face.", 'n'},
{"Giant spider" , "It is a vast black spider whose bulbous body"
  " is bloated with poison.", 'n'},
{"Dark elven mage","A drow elven figure, dressed all in black, hurling spells"
  " at you.", 'm'},
{"Orfax, Son of Boldor","He's just like daddy!  He knows mighty spells,"
  " but fortunately he is a yeek.", 'm'},
{"Dark elven warrior",
     "A drow elven figure in armour and ready with his sword.", 'm'},
{"Clear mushroom patch" , "Yum!  It looks quite tasty.", 'n'},
{"Grishnakh, the Hill Orc" , "He is a cunning and devious orc with"
  " a chaotic nature.", 'm'},
{"Giant white tick" , "It is moving slowly towards you.", 'n'},
{"Hairy mold" , "It is a strange hairy growth on the dungeon floor.", 'n'},
{"Disenchanter mold" , "It is a strange glowing growth on the dungeon floor.",
 'n'},
{"Pseudo dragon","A small relative of the dragon that inhabits dark caves.",
 'n'},
{"Tengu", "It is a fast-moving demon that blinks quickly in and out"
  " of existence; no other demon matches its teleporting mastery.", 'n'},
{"Creeping gold coins" , "They are shambling forward on thousands of legs.",
 'p'},
{"Wolf" , "It howls and snaps at you.", 'n'},
{"Giant fruit fly" , "A fast-breeding, annoying pest.", 'n'},
{"Panther","A large black cat, stalking you with intent.  It thinks you"
  "'re its next meal.", 'n'},
{"Brigand" , "He is eyeing your backpack.", 'm'},
{"Baby multi-hued dragon","This hatchling dragon is still soft, its"
  " eyes unaccustomed to light and its scales shimmering with a hint"
  " of colour.", 'n'},
{"Hippogriff","A strange hybrid of eagle, lion and horse.  It looks weird.",
 'n'},
{"Orc zombie" , "It is a shambling orcish corpse leaving behind a trail"
  " of flesh.", 'n'},
{"Gnome mage" , "A mage of short stature.", 'm'},
{"Black mamba" , "It has glistening black skin, a sleek body and"
  " highly venomous fangs.", 'n'},
{"White wolf","A large and muscled wolf from the northern wastes. "
  " Its breath is cold and icy and its fur coated in frost.", 'n'},
{"Grape jelly" , "It is a pulsing mound of glowing flesh.", 'n'},
{"Nether worm mass","They are a disgusting pile of dark worms, eating each"
  " other, the floor, the air, you....", 'p'},
{"Golfimbul, the Hill Orc Chief","A leader of a band of raiding orcs,"
  " he picks on hobbits.", 'm'},
{"Master yeek" , "A small humanoid that radiates some power.", 'n'},
{"Priest" , "A robed humanoid dedicated to his god.", 'm'},
{"Dark elven priest","A drow elven figure, dressed all in black, chanting"
  " curses and waiting to deliver your soul to hell.", 'm'},
{"Air spirit" , "A whirlwind of intelligent air.", 'n'},
{"Skeleton human" , "It is an animated human skeleton.", 'n'},
{"Zombie human" , "It is a shambling human corpse dropping chunks of"
  " flesh behind it.", 'n'},
{"Tiger","One of the largest of its species, a sleek orange and black"
  " shape creeps towards you, ready to pounce.", 'n'},
{"Moaning spirit" , "A ghostly apparition that shrieks horribly.", 'n'},
{"Swordsman" , "A warrior of considerable skill.", 'm'},
{"Stegocentipede" , "It is a vast armoured centipede with massive"
  " mandibles and a spiked tail.", 'n'},
{"Spotted jelly" , "A jelly thing.", 'n'},
{"Drider","A dark elf twisted by the goddess Lolth.  A dark elven torso"
  " sits upon the bloated form of a giant spider.", 'n'},
{"Killer brown beetle" , "It is a vicious insect with a tough carapace.", 'n'},
{"Boldor, King of the Yeeks","A great yeek, powerful in magic and"
  " sorcery, but a yeek all the same.", 'm'},
{"Ogre" , "A hideous, smallish giant that is often found near or with orcs.",
  'n'},
{"Creeping mithril coins" , "They are shambling forward on needle-sharp legs.",
 'p'},
{"Illusionist" , "A deceptive spell caster.", 'm'},
{"Druid","A mystic at one with nature.  Om.", 'm'},
{"Black orc" , "He is a large orc with powerful arms and deep black skin.",
 'm'},
{"Ochre jelly" , "A fast moving highly acidic jelly thing, that is eating"
  " away the floor it rests on.", 'n'},
{"Giant flea" , "It makes you itch just to look at it.", 'n'},
{"Ufthak of Cirith Ungol","A strong orc guarding the pass of Cirith"
  " Ungol.  He is mortally afraid of spiders.", 'm'},
{"Giant white dragon fly" , "It is a large fly that drips frost.", 'n'},
{"Blue icky thing","A strange icky creature with rudimentary"
  " intelligence but evil cunning.  It hungers for food, and you look tasty.",
  'n'},
{"Hill giant" , "A ten foot tall humanoid with powerful muscles.", 'n'},
{"Flesh golem" , "A shambling humanoid monster with long scars.", 'n'},
{"Warg" , "It is a large wolf with eyes full of cunning.", 'n'},
{"Giant black louse" , "It makes you itch just to look at it.", 'n'},
{"Lurker","A strange creature that merges with the dungeon floor,"
  " trapping its victims by enveloping them within its perfectly"
  " disguised form.", 'n'},
{"Wererat","A large rat with glowing red eyes.  The wererat is a"
  " disgusting creature, relishing in filth and disease.", 'n'},
{"Black ogre","A massive orc-like figure with black skin and powerful arms.",
 'n'},
{"Magic mushroom patch","Yum!  It looks quite tasty.  But, wait...it seems"
  " to glow with an unusual light.", 'n'},
{"Guardian naga" , "A giant snake-like figure with a woman's torso.", 'n'},
{"Light hound","A brilliant canine form whose light hurts your eyes,"
  " even at this distance.", 'n'},
{"Dark hound","A hole in the air in the shape of a huge hound.  No"
  " light falls upon its form.", 'n'},
{"Half-orc" , "He is a hideous deformed cross-breed with man and"
  " orc, combining man's strength and cunning with orcish evil.", 'm'},
{"Giant tarantula","A giant spider with hairy black and red legs.", 'n'},
{"Giant clear centipede" , "It is about four feet long and carnivorous.", 'n'},
{"Mirkwood spider","A strong and powerful spider from Mirkwood"
  " forest.  Cunning and evil, it seeks to taste your juicy"
  " insides.", 'n'},
{"Frost giant" , "A twelve foot tall giant covered in furs.", 'n'},
{"Griffon", "It is half lion, half eagle.  It flies"
  " menacingly towards you.", 'n'},
{"Homonculous" , "It is a small demonic spirit full of malevolence.", 'n'},
{"Gnome mage","A mage of short stature.", 'm'},
{"Ethereal hound","A pale white hound.  You can see straight through"
  " it.  Pulsing red lines and strange fluorescent light hints at"
  " internal organs best left to the imagination.", 'n'},
{"Clay golem" , "It is a massive statue-like figure made out of"
  " hardened clay.", 'n'},
{"Umber hulk" , "It is like a huge beetle with glaring eyes and"
  " large mandibles capable of slicing through rock.", 'n'},
{"Orc captain" , "An armoured orc with an air of authority.", 'm'},
{"Gelatinous cube" , "It is a strange, vast gelatinous structure"
  " that assumes cubic proportions as it lines all four walls of"
  " the corridors it patrols.  Through its transparent jelly structure"
  " you can see treasures it has engulfed, and a few corpses as well.", 'n'},
{"Giant green dragon fly" , "A vast, foul-smelling dragonfly.", 'n'},
{"Fire giant" , "A glowing fourteen foot tall giant.  Flames drip from its"
  " red skin.", 'n'},
{"Hummerhorn","A giant buzzing wasp, its stinger drips venom.", 'n'},
{"Ulfast, Son of Ulfang","A short and swarthy Easterling.", 'm'},
{"Quasit" , "The chaotic evil master's favourite pet.", 'n'},
{"Imp" , "The lawful evil master's favourite pet.", 'n'},
{"Forest troll" , "He is green skinned and ugly.", 'm'},
{"Nar, the Dwarf","This dwarf became so obsessed by gold that"
  " Morgoth tricked him into betraying his friends.", 'm'},
{"2-headed hydra","A strange reptilian hybrid with two heads, guarding"
  " its hoard.", 'n'},
{"Water spirit" , "A whirlpool of sentient liquid.", 'n'},
{"Giant brown scorpion" , "It is fast and poisonous.", 'n'},
{"Earth spirit" , "It is humanoid in shape and made out of solid rock.", 'n'},
{"Fire spirit" , "It is composed of pure flame.", 'n'},
{"Fire hound","Flames lick at its feet and its tongue is a blade of"
  " fire.  You can feel a furnace heat radiating from the creature.", 'n'},
{"Cold hound","A hound as tall as a man, this creature appears to"
  " be composed of angular planes of ice.  Cold radiates from it and"
  " freezes your breath in the air.", 'n'},
{"Energy hound","Saint Elmo's Fire forms a ghostly halo around this"
  " hound, and sparks sting your fingers as energy builds up in the"
  " air around you.", 'n'},
{"Mimic","A strange creature that disguises itself as discarded objects"
 " to lure unsuspecting adventurers within reach of its venomous claws.", 'n'},
{"Blink dog","A strange magical member of the canine race, its form"
  " seems to shimmer and fade in front of your very eyes.", 'n'},
{"Uruk-Hai" , "He is a cunning orc of power, as tall as a man,"
  " and stronger.  It fears little.", 'm'},
{"Shagrat, the Orc Captain","He is an Uruk of power and great cunning.", 'm'},
{"Gorbag, the Orc Captain","A gruesomely ugly but cunning orc, his"
  " eyes regard you with hatred.  His powerful arms flex menacingly as"
  " he advances.", 'm'},
{"Shambling mound","A pile of rotting vegetation that slides towards"
  " you with a disgusting stench, waking all it nears.", 'n'},
{"Stone giant" , "It is eighteen feet tall and looking at you.", 'n'},
{"Giant black dragon fly","The size of a large bird, this fly drips"
  " caustic acid.", 'n'},
{"Stone golem" , "It is an animated statue.", 'n'},
{"Red mold" , "It is a strange red growth on the dungeon floor; it seems"
  " to burn with flame.", 'n'},
{"Giant gold dragon fly","Large beating wings support this dazzling"
  " insect.  A loud buzzing noise pervades the air.", 'n'},
{"Bolg, Son of Azog","A large and powerful orc.  He looks just like"
  " his daddy.  He is tall and fast, but fortunately blessed"
  " with orcish brains.", 'm'},
{"Phase spider","A spider that never seems quite there.  Everywhere you"
  " look it is just half-seen in the corner of one eye.", 'n'},
{"3-headed hydra","A strange reptilian hybrid with three heads, guarding"
  " its hoard.", 'n'},
{"Earth hound","A beautiful crystalline shape does not disguise the"
  " danger this hound clearly presents.  Your flesh tingles as"
  " it approaches....", 'n'},
{"Air hound","Swirling vapours surround this beast as it floats"
  " towards you, seemingly walking on air.  Noxious gases sting your throat.",
  'n'},
{"Sabre-tooth tiger","A fierce and dangerous cat, its huge tusks and"
  " sharp claws would lacerate even the strongest armour.", 'n'},
{"Water hound","Liquid footprints follow this hound as it pads around"
  " the dungeon.  An acrid smell of acid rises from the dog's pelt.", 'n'},
{"Chimera" , "It is a strange concoction of lion, dragon and goat.  It"
  " looks very odd but very avoidable.", 'n'},
{"Quylthulg" , "It is a strange pulsing mound of flesh.", 'n'},
{"Sasquatch","A tall shaggy, furry humanoid, it could call the"
  " yeti brother.", 'n'},
{"Werewolf" , "It is a huge wolf with eyes that glow with manly intelligence.",
 'n'},
{"Dark elven lord","A drow elven figure in armour and radiating evil power.",
 'm'},
{"Cloud giant" , "It is a twenty foot tall giant wreathed in clouds.", 'n'},
{"Ugluk, the Uruk-Hai","Another of Morgoth's servants, this orc is"
  " strong and cunning.  He is ugly and scarred from many power struggles.",
 'm'},
{"Lugdush, the Uruk-Hai","A strong and cunning orc warrior, Lugdush"
  " sneers as he insults your mother.", 'm'},
{"Blue dragon bat" , "It is a glowing blue bat with a sharp tail.", 'n'},
{"Mimic","A strange creature that disguises itself as discarded objects"
 " to lure unsuspecting adventurers within reach of its venomous claws.", 'n'},
{"Fire vortex","A whirling maelstrom of fire.", 'n'},
{"Water vortex","A caustic spinning tower of water.", 'n'},
{"Cold vortex","A twisting whirlpool of frost.", 'n'},
{"Energy vortex","A shimmering tornado of air, sparks crackle along"
  " its length.", 'n'},
{"Mummified orc" , "It is an orcish figure covered in wrappings.", 'n'},
{"Killer stag beetle" , "It is a giant beetle with vicious claws.", 'n'},
{"Iron golem" , "It is a giant metal statue that moves slowly towards you.",
 'n'},
{"Giant yellow scorpion" , "It is a giant scorpion with a sharp stinger.",
 'n'},
{"Black ooze" , "It is a strangely moving puddle.", 'n'},
{"Hardened warrior" , "A scarred warrior who moves with confidence.", 'm'},
{"Azog, King of the Uruk-Hai","He is also known as the King of Khazad-dum. "
  " His ego is renowned to be bigger than his head.", 'm'},
{"Master rogue","A thief of great power and shifty speed.", 'm'},
{"Red dragon bat" , "It is a sharp-tailed bat, wreathed in fire.", 'n'},
{"Killer blue beetle" , "It is looking for prey.", 'n'},
{"Giant bronze dragon fly","This vast gleaming bronze fly has wings"
  " which beat mesmerically fast.", 'n'},
{"Forest wight" , "It is a ghostly apparition with a humanoid form.", 'n'},
{"Ibun, Son of Mim","One of the last of the petty dwarves.  Ibun is a"
  " tricky sorcerous little being, full of mischief.", 'm'},
{"Khim, Son of Mim","One of the last of the petty dwarves.  Khim is a"
  " tricky sorcerous little being, full of mischief.", 'm'},
{"4-headed hydra","A strange reptilian hybrid with four heads, guarding"
  " its hoard.", 'n'},
{"Mummified human" , "It is a human form encased in mouldy wrappings.", 'n'},
{"Vampire bat","An undead bat that flies at your neck hungrily.", 'n'},
{"Sangahyando of Umbar","A Black Numenorean with a blacker heart.", 'm'},
{"Angamaite of Umbar","A Black Numenorean who hates the men of the west.",
 'm'},
{"Banshee" , "It is a ghostly woman's form that wails mournfully.", 'f'},
{"Pukelman","A stumpy figure carved from stone, with glittering eyes,"
  " this sentinel strides towards you with deadly intent.", 'n'},
{"Dark elven druid","A powerful drow, with mighty nature"
  "-controlling enchantments.", 'm'},
{"Stone troll" , "He is a giant troll with scabrous black skin.", 'm'},
{"Troll priest","A troll who is so bright he knows how to read.", 'm'},
{"Wereworm","A huge wormlike shape dripping acid, twisted by evil"
  " sorcery into a foul monster that breeds on death.", 'n'},
{"Carrion crawler","A hideous centipede covered in slime and with"
  " glowing tentacles around its head.", 'n'},
{"Killer red beetle" , "It is a giant beetle with poisonous mandibles.", 'n'},
{"Giant grey ant lion" , "It is an ant encased in shaggy grey fur.", 'n'},
{"Ulwarth, Son of Ulfang","A short and swarthy Easterling.", 'm'},
{"Displacer beast","It is a huge black panther, clubbed tentacles"
  " sprouting from its shoulders.", 'n'},
{"Giant fire tick" , "It is smoking and burning with great heat.", 'n'},
{"Cave ogre","A giant orc-like figure with an awesomely muscled frame.", 'n'},
{"White wraith" , "It is a tangible but ghostly form made of white fog.", 'n'},
{"Monadic Deva","A lesser angel wearing little more than a loincloth -"
  " its steely skin provides all the protection it needs.", 'n'},
{"Mim, Betrayer of Turin","The last of his race, Mim is a petty"
 " dwarf.  Petty dwarves are strange creatures, powerful in sorcery and"
 " originating in the East.  They were hunted to extinction by high elves.",
 'm'},
{"Killer fire beetle" , "It is a giant beetle wreathed in flames.", 'n'},
{"Creeping adamantite coins","A mass of shining coins slithering"
  " towards you... Quick!  Pick it up and put it in your pocket.", 'p'},
{"Algroth","A powerful troll form.  Venom drips from its needlelike claws.",
 'n'},
{"Vibration hound","A blurry canine form which seems to be moving as"
  " fast as the eye can follow.  You can feel the earth resonating"
  " beneath your feet.", 'n'},
{"Nexus hound","A locus of conflicting points coalesce to form the"
  " vague shape of a huge hound.  Or is it really there?  Anyway, it seems"
  " to be coming towards you....", 'n'},
{"Ogre mage","A hideous ogre wrapped in black sorcerous robes.", 'n'},
{"Lokkak, the Ogre Chieftain","An ogre renowned for acts of"
  " surpassing cruelty, Lokkak quickly became the leader of a large band"
  " of violent ogres.", 'm'},
{"Vampire" , "It is a humanoid with an aura of power.  You notice a"
  " sharp set of front teeth.", 'n'},
{"Gorgimera","The result of evil"
  " experiments, this travesty of nature should never be alive.  It has"
  " 3 heads - gorgon, goat and dragon - all attached to a lion's body.", 'n'},
{"Colbran","A man-shaped form of living lightning, sparks and"
  " shocks crackle all over this madly capering figure.  It leaps and"
  " whirls around and about you....", 'n'},
{"Spirit naga","A wraithly snake-like form with the torso of a"
  " beautiful woman, it is the most powerful of its kind.", 'f'},
{"5-headed hydra","A strange reptilian hybrid with five heads"
  " dripping venom.", 'n'},
{"Black knight" , "He is a figure encased in deep black plate armour;"
  " he looks at you menacingly.", 'm'},
{"Uldor the Accursed","An evil and cunning man from the East.", 'm'},
{"Mage" , "A mage of some power - you can tell by the size of his hat.", 'm'},
{"Mind flayer","A humanoid form with a gruesome head, tentacular mouth,"
  " and piercing eyes.  Claws reach out for you and you feel a"
  " presence invade your mind.", 'n'},
{"Draebor, the Imp" ,"An intensely irritating git of a monster.", 'n'},
{"Basilisk","An evil reptile that preys on unsuspecting travellers. "
  " Its eyes stare deeply at you... your soul starts to wilt... look"
  " away....", 'n'},
{"Ice troll" , "He is a white troll with powerfully clawed hands.", 'm'},
{"Giant purple worm" , "It is a massive worm form, many feet in length. "
  " Its vast maw drips acid and poison.", 'n'},
{"Movanic Deva","A lesser angel protected by an aura of holiness. "
  " Its muscular form looks extremely powerful next to your own frail body.", 'n'},
{"Catoblepas","A strange ox-like form with a huge head but a thin,"
  " weak neck, it looks likes the creation of some deranged alchemist.", 'n'},
{"Mimic","A strange creature that disguises itself as discarded objects"
 " to lure unsuspecting adventurers within reach of its venomous claws.", 'n'},
{"Young blue dragon" , "It has a form that legends are made of.  Its"
  " still-tender scales are a deep blue in hue.  Sparks crackle along its"
  " length.", 'n'},
{"Young white dragon" , "It has a form that legends are made of.  Its"
  " still-tender scales are a frosty white in hue.  Icy blasts of cold"
  " air come from it as it breathes.", 'n'},
{"Young green dragon" , "It has a form that legends are made of.  Its"
  " still-tender scales are a deep green in hue.  Foul gas seeps through"
  " its scales.", 'n'},
{"Young bronze dragon", "It has a form that legends are made of.  Its"
  " still-tender scales are a rich bronze hue, and its shape masks"
  " its true form.", 'n'},
{"Mithril golem","This is a statue of truesilver!  Imagine how much"
  " that would be worth if you could take it home.", 'n'},
{"Shadow drake","It is a dragon-like form wrapped in shadow.  Glowing"
  " red eyes shine out in the dark.", 'n'},
{"Skeleton troll" , "It is a troll skeleton animated by dark dweomers.", 'n'},
{"Manticore" , "It is a winged lion's body with a human torso and a"
  " tail covered in vicious spikes.", 'n'},
{"Giant static ant" , "It is a giant ant that crackles with energy.", 'n'},
{"Giant army ant","An armoured form moving with purpose.  Powerful on"
  " its own, flee when hordes of them march.", 'n'},
{"Grave wight" , "It is a ghostly form with eyes that haunt you.", 'n'},
{"Killer slicer beetle" , "It is a beetle with deadly sharp"
  " cutting mandibles and a rock-hard carapace.", 'n'},
{"Ghost" , "You don't believe in them.", 'n'},
{"Death watch beetle" , "It is a giant beetle that produces a chilling sound.",
 'n'},
{"Ogre shaman" , "It is an ogre wrapped in furs and covered in grotesque"
  " body paints.", 'n'},
{"Nexus quylthulg" , "It is a very unstable, strange pulsing mound of flesh.",
 'n'},
{"Shelob, Spider of Darkness","Shelob is an enormous bloated"
  " spider, rumoured to have been one of the brood of Ungoliant"
  " the Unlight.  Her poison is legendary, as is her ego, which may be"
  " her downfall.  She used to guard the pass through Cirith Ungol, but"
  " has not been seen there for many eons.", 'f'},
{"Ninja" , "A humanoid clothed in black who moves with blinding speed.", 'm'},
{"Memory moss","A mass of green vegetation.  You don't remember"
  " seeing anything like it before....", 'n'},
{"Storm giant" , "It is a twenty-five foot tall giant wreathed in lighting.",
 'n'},
{"Cave troll" , "He is a vicious monster, feared for its ferocity.", 'm'},
{"Half-troll","A huge, ugly, half-human in search of plunder.", 'm'},
{"Mystic","An adept at unarmed combat, the mystic strikes with"
  " stunning power.  He can summon help from nature and is able to focus"
  " his power to ease any pain.", 'm'},
{"Barrow wight" , "It is a ghostly nightmare of a entity.", 'n'},
{"Giant skeleton troll" , "It is the animated form of a massive troll.", 'n'},
{"Chaos drake","A dragon twisted by the forces of chaos.  It seems"
  " first ugly, then fair, as its form shimmers and changes in front"
  " of your eyes.", 'n'},
{"Law drake","This dragon is clever and cunning.  It laughs at your"
  " puny efforts to disturb it.", 'n'},
{"Balance drake","A mighty dragon, the balance drake seeks to maintain"
  " the Cosmic Balance, and despises your feeble efforts to destroy evil.",
 'n'},
{"Ethereal drake","A dragon of elemental power, with control over light"
  " and dark, the ethereal drake's eyes glare with white hatred from"
  " the shadows.", 'n'},
{"Bert the Stone Troll","Big, brawny, powerful and with a taste for"
  " hobbit.  He has friends called Bill and Tom.", 'm'},
{"Bill the Stone Troll","Big, brawny, powerful and with a taste for"
  " hobbit.  He has friends called Bert and Tom.", 'm'},
{"Tom the Stone Troll" ,"Big, brawny, powerful and with a taste for"
  " hobbit.  He has friends called Bert and Bill.", 'm'},
{"Shade","A shadowy form clutches at you from the darkness.  A"
  " powerful undead with a deadly touch.", 'n'},
{"Spectre","A phantasmal shrieking spirit.  Its wail drives the intense"
  " cold of pure evil deep within your body.", 'n'},
{"Water troll" , "He is a troll that reeks of brine.", 'm'},
{"Fire elemental" , "It is a giant inferno of flames.", 'n'},
{"Astral Deva","It is an angel moving very quickly, wielding a holy"
  " war hammer and casting a volley of powerful spells in your direction.",
  'n'},
{"Water elemental" , "It is a giant tempest of water.", 'n'},
{"Invisible stalker" , "It is impossible to define its form but"
  " its violence is legendary.", 'n'},
{"Carrion crawler","A hideous centipede covered in slime and with"
  " glowing tentacles around its head.", 'n'},
{"Master thief","Cool and confident, fast and lithe; protect"
  " your possessions quickly!", 'm'},
{"Ulfang the Black","A short and swarthy Easterling dressed in Black.", 'm'},
{"Lich" , "It is a skeletal form dressed in robes.  It radiates vastly"
  " evil power.", 'n'},
{"Master vampire" , "It is a humanoid form dressed in robes.  Power"
  " emanates from its chilling frame.", 'n'},
{"Giant red scorpion" , "It is a giant red scorpion.  It looks poisonous.",
 'n'},
{"Earth elemental" , "It is a giant form composed of rock with fists"
  " of awesome power.", 'n'},
{"Air elemental" , "It is a giant tornado of winds.", 'n'},
{"Hell hound" , "It is a giant dog that glows with heat.  Flames pour"
  " from its nostrils.", 'n'},
{"Eog golem","A deep grey statue, which is striding towards you with an"
  " all-too-familiar purpose.  Your magic surprisingly feels much"
  " less powerful now.", 'n'},
{"Olog-Hai" , "It is a massive intelligent troll with needle sharp fangs.",
 'n'},
{"Dagashi","A human warrior, moving with lightning speed.", 'm'},
{"Gravity hound","Unfettered by the usual constraints of gravity,"
  " these unnatural creatures are walking on the walls and even"
  " the ceiling!  The earth suddenly feels rather less solid as you"
  " see gravity warp all round the monsters.", 'n'},
{"Acidic cytoplasm","A disgusting animated blob of destruction.  Flee its"
  " gruesome hunger!", 'n'},
{"Inertia hound","Bizarrely, this hound seems to be hardly moving at"
  " all, yet it approaches you with deadly menace.  It makes you tired"
  " just to look at it.", 'n'},
{"Impact hound","A deep blue shape is visible before you, its canine"
  " form strikes you with an almost physical force.  The dungeon"
  " floor buckles as if struck by a powerful blow as it stalks towards you.",
  'n'},
{"Dread","It is a form that screams its presence against the eye. "
  " Death incarnate, its hideous black body seems to struggle"
  " against reality as the universe itself struggles to banish it.", 'n'},
{"Ooze elemental" , "Animated filth, an eyesore of ooze.", 'n'},
{"Smoke elemental" , "Its blackened form crackles with heat.", 'n'},
{"Young black dragon" , "It has a form that legends are made of.  Its"
 " still-tender scales are a darkest black hue.  Acid drips from its body.",
 'n'},
{"Mumak","A massive elephantine form with eyes twisted by madness.", 'n'},
{"Giant red ant lion" , "A giant ant covered in shaggy fur.  Its"
  " powerful jaws glow with heat.", 'n'},
{"Mature white dragon" , "A large dragon, scales gleaming bright white.", 'n'},
{"Xorn" , "A huge creature of the element Earth.  Able to merge with its"
  " element, it has four huge arms protruding from its enormous torso.", 'n'},
{"Shadow","A mighty spirit of darkness of vaguely humanoid form. "
  " Razor-edged claws reach out to end your life as it glides towards"
  " you, seeking to suck the energy from your soul to feed its power.", 'n'},
{"Phantom","An unholy creature of darkness, the aura emanating from"
  " this evil being saps your very soul.", 'n'},
{"Grey wraith" , "A tangible but ghostly form, made of grey fog.  The"
  " air around it feels deathly cold.", 'n'},
{"Young multi-hued dragon" , "It has a form that legends are made"
  " of.  Beautiful scales of shimmering and magical colours cover it.", 'n'},
{"Colossus","An enormous construct resembling a titan made from stone. "
  " It strides purposefully towards you, swinging its slow fists with"
  " earth-shattering power.", 'n'},
{"Young gold dragon", "It has a form that legends are made of.  Its"
  " still-tender scales are a tarnished gold hue, and light is"
  " reflected from its form.", 'n'},
{"Rogrog the Black Troll" ,"A massive and cruel troll of great power,"
  " drool slides caustically down his muscular frame.  Despite his bulk,"
  " he strikes with stunning speed.", 'm'},
{"Mature blue dragon" , "A large dragon, scales tinted deep blue.", 'n'},
{"Mature green dragon" , "A large dragon, scales tinted deep green.", 'n'},
{"Mature bronze dragon","A large dragon with scales of rich bronze.", 'n'},
{"Young red dragon" , "It has a form that legends are made of.  Its"
  " still-tender scales are a deepest red hue.  Heat radiates from its"
  " form.", 'n'},
{"Trapper","A larger cousin of the lurker, this creature traps"
  " unsuspecting victims and paralyzes them, to be slowly digested later.",
  'n'},
{"Bodak","It is a humanoid form composed of flames and hatred.", 'n'},
{"Ice elemental" , "It is a animated statue of ice.  It regards"
  " you disdainfully.", 'n'},
{"Necromancer" , "A gaunt figure, clothed in black robes.", 'm'},
{"Lorgan, Chief of the Easterlings","A mighty warrior from the east,"
  " Lorgan hates everything that he cannot control.", 'm'},
{"Demonologist","A figure twisted by evil standing in robes of deepest"
  " crimson.", 'm'},
{"Mummified troll" , "It is a massive figure clothed in wrappings.  You"
  " are wary of its massive fists.", 'n'},
{"The Queen Ant","She's upset because you hurt her children.", 'f'},
{"Will o' the wisp","A strange ball of glowing light.  It disappears"
  " and reappears and seems to draw you to it.  You seem somehow"
  " compelled to stand still and watch its strange dancing motion.", 'n'},
{"Magma elemental" , "It is a glowing form of molten hate.", 'n'},
{"Black pudding","A lump of rotting black flesh that slurrrrrrrps"
  " across the dungeon floor.", 'n'},
{"Iridescent beetle" , "It is a giant beetle, whose carapace shimmers"
  " with vibrant energies.", 'n'},
{"Nexus vortex","A maelstrom of potent magical energy.", 'n'},
{"Plasma vortex","A whirlpool of intense flame, charring the stones at"
  " your feet.", 'n'},
{"Mature red dragon" , "A large dragon, scales tinted deep red.", 'n'},
{"Mature gold dragon","A large dragon with scales of gleaming"
  " gold.", 'n'},
{"Crystal drake","A dragon of strange crystalline form.  Light"
  " shines through it, dazzling your eyes with spectrums of colour.", 'n'},
{"Mature black dragon" , "A large dragon, with scales of deepest black.", 'n'},
{"Mature multi-hued dragon",
 "A large dragon, scales shimmering many colours.", 'n'},
{"Death knight","It is a humanoid form dressed in armour of an"
  " ancient form.  From beneath its helmet, eyes glow a baleful red"
  " and seem to pierce you like lances of fire.", 'n'},
{"Castamir the Usurper","A Black Numenorean who usurped the throne"
  " of Gondor, he is treacherous and evil.", 'm'},
{"Time vortex","You haven't seen it yet.", 'n'},
{"Shimmering vortex","A strange pillar of shining light that hurts"
 " your eyes.  Its shape changes constantly as it cuts through the air"
 " towards you.  It is like a beacon, waking monsters from their slumber.",
 'n'},
{"Ancient blue dragon" , "A huge draconic form.  Lightning crackles"
  " along its length.", 'n'},
{"Ancient bronze dragon","A huge draconic form enveloped in a cascade"
  " of colour.", 'n'},
{"Beholder" ,"It is a ball-like creature, with one main eye and"
  " twelve smaller eyes on stalks.  It has thousands of fangs - beware!", 'n'},
{"Emperor wight" , "Your life force is torn from your body as this"
  " powerful unearthly being approaches.", 'n'},
{"Planetar","It is an angel, fast and strong.  You are stunned by"
  " its extreme holiness and try to resist all desires to obey it.", 'n'},
{"Vargo, Tyrant of Fire","A raging pillar of fire, Vargo burns every"
  " living thing beyond recognition.", 'n'},
{"Black wraith" , "A figure that seems made of void, its strangely"
  " human shape is cloaked in shadow.  It reaches out at you.", 'n'},
{"Erinyes" , "It is a lesser demon of female form; however, she takes"
  " little time to show its true colours.", 'f'},
{"Nether wraith" , "A form that hurts the eye, death permeates the"
  " air around it.  As it nears you, a coldness saps your soul.", 'n'},
{"Eldrak","A massive troll, larger and stronger than many men"
  " together.  Usually a solitary creature.", 'n'},
{"Ettin","A massive troll of huge strength.  Ettins are stupid but violent.",
 'n'},
{"Waldern, King of Water","A huge water elemental, Waldern is master of"
  " all things liquid.  Wave after wave drowns your frail body.", 'n'},
{"Kavlax the Many-Headed","A large dragon with a selection of heads,"
  " all shouting and arguing as they look for prey, but each with its"
  " own deadly breath weapon.", 'm'},
{"Ancient white dragon" , "A huge draconic form.  Frost covers it from"
  " head to tail.", 'n'},
{"Ancient green dragon" , "A huge draconic form enveloped in clouds"
  " of poisonous vapour.", 'n'},
{"7-headed hydra",
 "A strange reptilian hybrid with seven heads dripping venom.", 'n'},
{"Night mare","A fearsome skeletal horse with glowing eyes, that watch"
  " you with little more than a hatred of all that lives.", 'n'},
{"Vampire lord" , "A foul wind chills your bones as this ghastly"
  " figure approaches.", 'n'},
{"Ancient black dragon" , "A huge draconic form.  Pools of acid melt"
  " the floor around it.", 'n'},
{"Disenchanter worms" , "They are a strange mass of squirming worms.  Magical"
  " energy crackles around these disgusting forms.", 'p'},
{"Rotting quylthulg" , "It is a pulsing flesh mound that reeks of death"
  " and putrefaction.", 'n'},
{"Spirit troll", "A weird troll from the elemental planes.", 'n'},
{"Lesser titan" , "It is a humanoid figure thirty feet tall that gives"
  " off an aura of power and hate.", 'n'},
{"9-headed hydra","A strange reptilian hybrid with nine smouldering heads.",
 'n'},
{"Enchantress","This elusive female spellcaster has a special affinity"
  " for dragons, whom she rarely fights without.", 'f'},
{"Archpriest","An evil priest, dressed all in black.  Deadly spells hit"
  " you at an alarming rate as his black spiked mace rains down blow"
  " after blow on your pitiful frame.", 'm'},
{"Sorceror" , "A human figure in robes, he moves with magically"
  " improved speed, and his hands are ablur with spell casting.", 'm'},
{"Xaren" ,"It is a tougher relative of the Xorn.  Its hide glitters"
  " with metal ores.", 'n'},
{"Giant roc","A vast legendary bird, its iron talons rake the"
  " most impenetrable of surfaces and its screech echoes through the"
  " many winding dungeon corridors.", 'n'},
{"Uvatha the Horseman","A tall black cloaked Ringwraith, he is a master"
  " of horsemanship.  He longs to taste your blood.", 'm'},
{"Minotaur" ,"It is a cross between a human and a bull.", 'n'},
{"Medusa, the Gorgon","One of the original three ugly sisters.  Her"
  " face could sink a thousand ships.  Her scales rattle as she"
  " slithers towards you, venom dripping from her ghastly mouth.", 'f'},
{"Death drake","It is a dragon-like form wrapped in darkness.  You"
  " cannot make out its true form but you sense its evil.", 'n'},
{"Ancient red dragon" , "A huge draconic form.  Wisps of smoke steam"
  " from its nostrils and the extreme heat surrounding it makes you"
  " gasp for breath.", 'n'},
{"Ancient gold dragon","A huge draconic form wreathed in a nimbus of light.",
 'n'},
{"Great crystal drake","A huge crystalline dragon.  Its claws could cut"
  " you to shreds and its teeth are razor sharp.  Strange colours"
  " ripple through it as it moves in the light.", 'n'},
{"Vrock" , "It is a demon with a long neck and raking claws.", 'n'},
{"Death quasit" , "It is a demon of small stature, but its armoured"
  " frame moves with lightning speed and its powers make it a tornado"
  " of death and destruction.", 'n'},
{"Adunaphel the Quiet","A sorceress in life, Adunaphel quickly fell"
  " under Sauron's sway and the power of the rings.", 'f'},
{"Dark elven sorceror","A drow elven figure, dressed in deepest black. "
  " Power seems to crackle from his slender frame.", 'm'},
{"Master lich" , "A skeletal form wrapped in robes.  Powerful magic"
  " crackles along its bony fingers.", 'n'},
{"Hezrou" , "It is a demon of lizard form with cruel-looking jaws.", 'n'},
{"Akhorahil the Blind","A mighty sorcerer King, Akhorahil was blind"
  " in life.  With powerful enchantments, he created jewelled eyes"
  " that enabled him to see better than any ordinary man ever could.", 'm'},
{"Gorlim, Betrayer of Barahir","This once-mighty warrior was so"
  " dominated by Morgoth's power that he became little more than a"
  " mindless creature of evil.", 'm'},
{"Solar" , "Never a more heavenly being have you seen.  The very holiness"
  " of its presence makes you deeply respect it.  Few creatures can"
  " match the powers of a Solar; fewer still live to tell the tale"
  " after attacking one.", 'n'},
{"Glabrezu" ,"It is demon with arms and pincers, its form a true"
  " mockery of life.", 'n'},
{"Ren the Unclean","Ren was an insane eastern king who believed himself"
  " to be the son of a volcano god.  At an early age his sanity"
  " was destroyed by a plague that wiped out his family, and he"
  " never recovered.", 'm'},
{"Nalfeshnee" , "It is a large demon with the head of a giant boar. "
  " Flames run up and down its length.", 'n'},
{"Undead beholder","A huge eyeball that floats in the air.  Black"
  " nether storms rage around its bloodshot pupil and light seems to"
  " bend as it sucks its power from the very air around it.  Your"
  " soul chills as it drains your vitality for its evil enchantments.", 'n'},
{"Dread","It is a form that screams its presence against the eye. "
  " Death incarnate, its hideous black body seems to struggle"
  " against reality as the universe itself struggles to banish it.", 'n'},
{"Mumak","A massive elephantine form with eyes twisted by madness.", 'n'},
{"Ancient multi-hued dragon", "A huge draconic form.  Many colours"
  " ripple down its massive frame.  Few live to see another.", 'n'},
{"Ethereal dragon","A huge dragon emanating from the elemental plains,"
  " the ethereal dragon is a master of light and dark.  Its form"
  " disappears from sight as it cloaks itself in unearthly shadows.", 'n'},
{"Ji Indur Dawndeath","This Ringwraith was a weak-minded sorcerer-king"
  " who fell easily under Sauron's power.", 'm'},
{"Marilith" , "She is a demon of female form with many arms, each"
  " bearing deadly weapons.", 'f'},
{"Quaker, Master of Earth","A huge stone elemental stands before you. "
  " The walls and ceiling are reduced to rubble as Quaker advances.", 'm'},
{"Balor" ,"It is a massive humanoid demon wreathed in flames.", 'n'},
{"Ariel, Queen of Air","A whirlwind of destruction, Ariel, air"
  " elemental sorceress, avoids your blows with her extreme speed.", 'f'},
{"11-headed hydra",
 "A strange reptilian hybrid with eleven smouldering heads.", 'n'},
{"Patriarch","A dark priest of the highest order.  Powerful and evil,"
  " beware his many spells.", 'm'},
{"Dreadmaster","It is an unlife of power almost unequaled.  An affront"
  " to existence, its very touch abuses and disrupts the flow of life,"
  " and its unearthly limbs, of purest black, crush rock and flesh with ease.",
  'n'},
{"Drolem","A constructed dragon, the drolem has massive strength. "
  " Powerful spells weaved during its creation make it a"
  " fearsome adversary.  Its eyes show little intelligence, but it has"
  " been instructed to destroy all it meets.", 'n'},
{"Scatha the Worm","An ancient and wise Dragon.  Scatha has"
  " grown clever over the long years.  His scales are covered with frost,"
  " and his breath sends a shower of ice into the air.", 'm'},
{"Dwar, Dog Lord of Waw","Dwar had a special affinity for dogs in life,"
  " and can still command them at will.  He howls manically as he"
  " reaches out to destroy you.", 'm'},
{"Smaug the Golden","Smaug is one of the Uruloki that still survive, a"
  " fire-drake of immense cunning and intelligence.  His speed through"
  " air is matched by few other dragons and his dragonfire is"
  " what legends are made of.", 'm'},
{"Dracolich","The skeletal form of a once-great dragon, enchanted by"
  " magic most perilous.  Its animated form strikes with speed and"
  " drains life from its prey to satisfy its hunger.", 'n'},
{"Greater titan","A forty foot tall humanoid that shakes the ground as"
  " it walks.  The power radiating from its frame shakes your courage,"
  " its hatred inspired by your defiance.", 'n'},
{"Dracolisk","A mixture of dragon and basilisk, the dracolisk stares at"
  " you with deep piercing eyes, its evil breath burning the ground"
  " where it stands.", 'n'},
{"Death mold","It is the epitome of all that is evil, in a mold. "
  " Its lifeless form draws power from sucking the souls of those"
  " that approach it, a nimbus of pure evil surrounds it.  However, it"
  " can't move....", 'n'},
{"Itangast the Fire Drake","A mighty ancient dragon, Itangast's"
  " form scorches your flesh.  Wisps of smoke curl up from his nostrils"
  " as he regards you with disdain.", 'm'},
{"Glaurung, Father of the Dragons","Glaurung is the father of all"
  " dragons, and was for a long time the most powerful.  Nevertheless, he"
  " still has full command over his brood and can command them"
  " to appear whenever he so wishes.  He is the definition of dragonfire.",
  'm'},
{"Master mystic","A lord of all that is natural, skilled in the"
  " mystic ways.  He is a master of martial arts and is at one with"
  " nature, able to summon help from the wild if need be.", 'm'},
{"Muar, the Balrog","A huge balrog surrounded by raging pillars of"
  " fire, Muar is indeed a terrible opponent.  Wielding a great whip of"
  " fire and a blazing sword, his fury blisters your skin and melts your"
  " flesh!", 'm'},
{"Nightwing","Everywhere colours seem paler and the air chiller.  At"
  " the centre of the cold stands a mighty figure.  Its wings envelop you"
  " in the chill of death as the nightwing reaches out to draw you"
  " into oblivion.  Your muscles sag and your mind loses all will to"
  " fight as you stand in awe of this mighty being.", 'n'},
{"Nether hound","You feel a soul-tearing chill upon viewing this beast,"
  " a ghostly shape of darkness.  You think it may be a dog, but it"
  " makes you feel weaker just to look at it....", 'n'},
{"Time hound","You get a terrible sense of deja vu, or is it a"
  " premonition?  All at once you see a little puppy and a toothless"
  " old dog.  Perhaps you should give up and go to bed.", 'n'},
{"Plasma hound","The very air warps as pure elemental energy stalks"
  " towards you in the shape of a giant hound.  Your hair stands on end"
  " and your palms itch as you sense trouble....", 'n'},
{"Demonic quylthulg", "A pile of pulsing flesh that glows with an"
  " inner hellish fire.  The world itself seems to cry out against it.", 'n'},
{"Great storm wyrm","A vast dragon of power.  Storms and lightning"
  " crash around its titanic form.  Deep blue scales reflect the"
  " flashes and highlight the creature's great muscles.  It regards you"
  " with contempt.", 'n'},
{"Baphomet the Minotaur Lord","A fearsome bull-headed demon,"
  " Baphomet swings a mighty axe as he curses all that defy him.", 'm'},
{"Harowen the Black Hand","He is a master of disguise, an expert"
  " of stealth, a genius at traps, and moves with blinding speed. "
  " Better check your pockets just in case....", 'm'},
{"Hoarmurath of Dir","A Ringwraith powerful in fell sorcery, he yearns"
  " for the life he has lost for a life of everlasting torment.", 'm'},
{"Grand master mystic","He is one of the few true masters of the art,"
  " being extremely skillful in all forms of unarmed combat and"
  " controlling the world's natural creatures with disdainful ease.", 'm'},
{"Khamul the Easterling","A warrior-king of the East.  Khamul is a"
  " powerful opponent, his skill in combat awesome and his form twisted"
  " by evil cunning.", 'm'},
{"Ethereal hound","A pale white hound.  You can see straight through"
  " it.  Pulsing red lines and strange fluorescent light hints at"
  " internal organs best left to the imagination.", 'n'},
{"Great ice wyrm","An immense dragon capable of awesome destruction. "
  " You have never felt such extreme cold, or witnessed such an icy"
  " stare.  Begone quickly or feel its wrath!", 'n'},
{"The Phoenix","A massive glowing eagle bathed in flames.  The searing"
  " heat chars your skin and melts your armour.", 'n'},
{"Nightcrawler","This intensely evil creature bears the form of"
 " a gargantuan black worm.  Its gaping maw is a void of blackness,"
 " acid drips from its steely hide.  It is like nothing you have ever"
 " seen before, and a terrible chill runs down your spine as you face it....",
 'n'},
{"Hand druj","A skeletal hand floating in the air, motionless except for"
  " its flexing fingers.", 'n'},
{"Eye druj","A bloodshot eyeball floating in the air, you'd be forgiven"
  " for assuming it harmless.", 'n'},
{"Skull druj","A glowing skull possessed by sorcerous power.  It need"
  " not move, but merely blast you with mighty magic.", 'n'},
{"Chaos vortex","Void, nothingness, spinning destructively.", 'n'},
{"Aether vortex","An awesome vortex of pure magic, power radiates from its"
  " frame.", 'n'},
{"The Lernean Hydra","A massive legendary hydra.  It has twelve powerful"
  " heads.  Its many eyes stare at you as clouds of smoke and"
  " poisonous vapour rise from its seething form.", 'n'},
{"Thuringwethil","Chief messenger between Sauron and Morgoth, she is"
  " surely the most deadly of her vampire race.  At first she is charming"
  " to meet, but her wings and eyes give away her true form.", 'f'},
{"Great hell wyrm","A vast dragon of immense power.  Fire leaps"
  " continuously from its huge form.  The air around it scalds you. "
  " Its slightest glance burns you, and you truly realize how"
  " insignificant you are.", 'n'},
{"Draconic quylthulg", "It looks like it was once a dragon corpse,"
  " now deeply infected with magical bacteria that make it pulse in a"
  " foul and degrading way.", 'n'},
{"Fundin Bluecloak","He is one of the greatest dwarven priests to walk"
  " the earth.  Fundin has earned a high position in the church, and"
  " his skill with both weapon and spell only justify his position"
  " further.  His combination of both dwarven strength and priestly"
  " wisdom are a true match for any adventurer.", 'm'},
{"Uriel, Angel of Fire","A creature of godly appearance, you dare"
  " not challenge Uriel's supremacy.  Those who stood against him before"
  " are but a memory, cremated by his mastery of elemental"
  " fire.", 'm'},
{"Azriel, Angel of Death","Azriel commands awesome power, his visage"
  " holy enough to shrivel your soul.  You shriek with disbelief as"
  " his mastery of death draws you to your grave.  It is truly beyond"
  " all but the mightiest of warriors to stand against him and live.", 'm'},
{"Ancalagon the Black","'Rushing Jaws' is his name, and death is his"
  " game.  No dragon of the brood of Glaurung can match him.", 'm'},
{"Nightwalker","A huge giant garbed in black, more massive than a titan"
  " and stronger than a dragon.  With terrible blows, it breaks your"
  " armour from your back, leaving you defenseless against its evil"
  " wrath.  It can smell your fear, and you in turn smell the awful"
  " stench of death as this ghastly figure strides towards you menacingly.",
  'n'},
{"Gabriel, the Messenger","Commanding a legion of Solars, Gabriel"
  " will destroy you for your sins.  He will crush you like the"
  " pitiful insignificant being he sees you to be.  Your very soul"
  " will be taken into judgement by his supreme authority as he cleanses the"
  " world of evil.", 'm'},
{"Saruman of Many Colours","Originally known as the White, Saruman"
  " fell prey to Sauron's wiles.  He seeks to emulate him and breeds"
  " orcs and trolls to fight for him.  He searches forever for the One"
  " Ring, to become a mighty Sorcerer-King of the world.", 'm'},
{"Dreadlord","It is a massive form of animated death, its colour"
  " deeper than black.  It drinks in light, and space around it is"
  " twisted and torn by the weight of its evil.  It is unlife and it"
  " knows nothing but the stealing of souls and the stench of death. "
  " Flee its hunger!", 'n'},
{"The Cat Lord","Master of all things feline, the Cat Lord moves"
  " with catlike stealth....Miaow!", 'm'},
{"Jabberwock","'Beware the Jabberwock, my son! / The jaws that bite, the"
 " claws that catch!'  Run and run quickly, death incarnate chases"
 " behind you!", 'n'},
{"Chaos hound","A constantly changing canine form, this hound"
  " rushs towards you as if expecting mayhem and chaos ahead.  It appears"
  " to have an almost kamikaze relish for combat.  You suspect all may"
  " not be as it seems....", 'n'},
{"Great Wyrm of Chaos","A massive dragon of changing form.  As you watch,"
  " it appears first fair and then foul.  Its body is twisted by"
  " chaotic forces as it strives to stay real.  Its very existence"
  " distorts the universe around it.", 'n'},
{"Great Wyrm of Law","A massive dragon of powerful intellect.  It seeks"
  " to dominate the universe and despises all other life.  It sees all"
  " who do not obey it as mere insects to be crushed underfoot.", 'n'},
{"Great Wyrm of Balance","This massive dragon is the mightiest"
  " of dragonkind.  It is thousands of years old and seeks to maintain"
  " the Cosmic Balance.  It sees you as an upstart troublemaker without the"
  " wisdom to control your actions.  It will destroy you.", 'n'},
{"Tselakus, the Dreadlord","This huge affront to existence twists and"
  " tears at the fabric of space.  A master of mighty magic,"
  " Tselakus hungers for your tender flesh.  Darkness itself recoils"
  " from the touch of Tselakus as he leaves a trail of death"
  " and destruction.  Tselakus is a being of sneering"
  " contempt, laughing at your pitiful efforts to defy him.  Mighty"
  " claws rend reality as he annihilates all in his path to your soul!", 'm'},
{"Tiamat, Celestial Dragon of Evil","Usually found guarding the first"
  " plane of Hell, Tiamat is a formidable opponent, her five"
  " heads breathing death to all who stand against her.", 'f'},
{"Black reaver","A humanoid form,  black as night, advancing steadily"
  " and unstoppably.  Flee!", 'n'},
{"Master quylthulg","A pulsating mound of flesh, shining with silver"
  " pulses of throbbing light.", 'n'},
{"Greater draconic quylthulg","A massive mound of scaled flesh,"
  " throbbing and pulsating with multi-hued light.", 'n'},
{"Greater rotting quylthulg","A massive pile of rotting flesh.  A"
  " disgusting stench fills the air as it throbs and writhes.", 'n'},
{"Vecna, the Emperor Lich" ,"He is a highly cunning, extremely"
  " magical being, spoken of in legends.  This ancient shadow of death"
  " wilts any living thing it passes.", 'm'},
{"Omarax the Eye Tyrant","A vast baleful eye floating in the air.  His"
  " gaze seems to shred your soul and his spells crush your will.  He"
  " is ancient, his history steeped in forgotten evils, his"
  " atrocities numerous and sickening.", 'm'},
{"Ungoliant, the Unlight","This enormous, hideous spirit of void is in"
  " the form of a spider of immense proportions.  She is surrounded by"
  " a cloud of Unlight as she sucks in all living light into her"
  " bloated body.  She is always ravenously hungry and would even"
  " eat herself to avoid starvation.  She is rumoured to have a foul"
  " and deadly breath.", 'f'},
{"Aether hound","A shifting, swirling form.  It seems to be all colours"
  " and sizes and shapes, though the dominant form is that of a huge"
  " dog.  You feel very uncertain all of a sudden.", 'n'},
{"The Mouth of Sauron","The Mouth of Sauron is a mighty spell caster. "
  " So old that even he cannot remember his own name, his power and"
  " evil are undeniable.  He believes unshakeably that he is unbeatable"
  " and laughs as he weaves his awesome spells.", 'm'},
{"The Emperor Quylthulg","A huge seething mass of flesh with a"
  " rudimentary intelligence, the Emperor Quylthulg changes colours"
  " in front of your eyes.  Pulsating first one colour then the next,"
  " it knows only it must bring help to protect itself.", 'n'},
{"Qlzqqlzuup, the Lord of Flesh","This disgusting creature squeals"
  " and snorts as it writhes on the floor.  It pulsates with evil."
  "  Its intent is to overwhelm you with monster after monster, until it"
  " can greedily dine on your remains.", 'n'},
{"Murazor, the Witch-King of Angmar","The Chief of the Ringwraiths.  A"
  " fell being of devastating power.  His spells are lethal and his"
  " combat blows crushingly hard.  He moves at speed, and commands"
  " legions of evil to do his bidding.  It is said that he is fated never to"
  " die by the hand of mortal man.", 'm'},
{"Pazuzu, Lord of Air","A winged humanoid from the Planes of Hell,"
  " Pazuzu grins inhumanely at you as he decides your fate.", 'm'},
{"Hell hound" , "It is a giant dog that glows with heat.  Flames pour"
  " from its nostrils.", 'n'},
{"Cantoras, the Skeletal Lord","A legion of evil undead druj animating"
  " the skeleton of a once mighty sorcerer.  His power is devastating"
  " and his speed unmatched in the underworld.  Flee his wrath!", 'm'},
{"The Tarrasque","The Tarrasque is a massive reptile of legend, rumoured"
  " to be unkillable and immune to magic.  Fear its anger, for"
  " its devastation is unmatched!", 'n'},
{"Lungorthin, the Balrog of White Fire","A massive form cloaked in"
  " flame.  Lungorthin stares balefully at you with eyes that smoulder"
  " red.  The dungeon floor where he stands is scorched by the heat of"
  " his body.", 'm'},
{"Draugluin, Sire of All Werewolves","Draugluin provides Sauron with"
  " a fearsome personal guard.  He is an enormous wolf inhabited with"
  " a human spirit.  He is chief of all his kind.", 'm'},
{"Feagwath the Undead Sorceror","A stench of corruption and decay"
  " surrounds this sorcerer, who has clearly risen from the grave"
  " to continue his foul plots and schemes.", 'm'},
{"Carcharoth, the Jaws of Thirst","The first guard of Angband,"
  " Carcharoth, also known as 'The Red Maw', is the largest wolf to"
  " ever walk the earth.  He is highly intelligent and a deadly opponent"
  " in combat.", 'm'},
{"Cerberus, Guardian of Hades","A two-headed hell hound of fearsome"
  " aspect.  Flame burns merrily from its hide as it snarls and roars"
  " its defiance.", 'n'},
{"Gothmog, the High Captain of Balrogs","Gothmog is the Chief Balrog"
  " in Morgoth's personal guard.  He is renowned for slaying Ecthelion"
  " the Warder of the Gates and he has never been defeated in combat. "
  " With his whip of flame and awesome fiery breath he saved his master"
  " from Ungoliant's rage.", 'm'},
{"Sauron, the Sorcerer","He is Morgoth's most powerful servant.  Mighty"
  " in spells and enchantments, he created the One Ring.  His eyes glow"
  " with power and his gaze seeks to destroy your soul.  He has"
  " many servants, and rarely fights without them.", 'm'},
{"Morgoth, Lord of Darkness",
  "He is the Master of the Pits of Angband.  His figure is like"
  " a black mountain crowned with Lightning.  He rages with"
  " everlasting anger, his body scarred by Fingolfin's eight mighty"
  " wounds.  He can never rest from his pain, but seeks forever to"
  " dominate all that is light and good in the world.  He is the origin"
  " of man's fear of darkness and created many foul creatures with his"
  " evil powers.  Orcs, Dragons, and Trolls are his most foul"
  " corruptions, causing much pain and suffering in the world to"
  " please him.  His disgusting visage, twisted with evil, is crowned"
  " with iron, the two remaining Silmarils forever burning him.  Grond,"
  " the mighty Hammer of the Underworld, cries defiance as he"
  " strides towards you to crush you to a pulp!", 'm'},
{"","",' '}			/* space for player ghost -CWS */
};
