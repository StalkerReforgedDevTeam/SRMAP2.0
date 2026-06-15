class SRZ_RPNameManager
{
	protected static ref SRZ_RPNameManager s_Instance;

	// Name pools (combined for variety)
	protected ref array<string> m_FirstNames = new array<string>();
	protected ref array<string> m_LastNames  = new array<string>();
	
	// Banned words pool
	protected ref array<string> m_BannedWords = new array<string>();
	
	// Banned name combinations (first + last)
	protected ref array<string> m_BannedCombinations = new array<string>();

	// --------------------------------------------------------------------------------------------
	// Singleton instance
	static SRZ_RPNameManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new SRZ_RPNameManager();
		
		return s_Instance;
	}

	// --------------------------------------------------------------------------------------------
	// Constructor
	void SRZ_RPNameManager()
	{
		BuildNamePools();
		BuildBannedWordsList();
		BuildBannedCombinationsList();
	}

	// --------------------------------------------------------------------------------------------
	// Build banned words list (single words)
	protected void BuildBannedWordsList()
	{
		m_BannedWords.InsertAll({
			// Slurs and hate speech
			"nigger", "nigga", "faggot", "fag", "retard", "retarded", "kike", "spic", "chink", "gook",
			"wetback", "beaner", "coon", "darkie", "paki", "raghead", "towelhead", "cracker",
			"honky", "gringo", "tranny", "shemale", "dyke", "lesbo",
			
			// Sexual/vulgar
			"penis", "vagina", "cock", "dick", "pussy", "cunt", "tits", "boobs", "anus", "asshole",
			"butthole", "dildo", "vibrator", "blowjob", "handjob", "cumshot", "creampie", "gangbang",
			"orgy", "rape", "rapist", "molest", "pedophile", "pedo", "incest", "bestiality",
			"whore", "slut", "hoe", "hooker", "prostitute", "pornstar", "stripper",
			
			// Profanity
			"fuck", "fucking", "fucker", "fucked", "shit", "shitty", "bitch", "bastard",
			"arse", "piss", "pissed", "bollocks", "wanker", "twat",
			
			// System/impersonation
			"admin", "administrator", "moderator", "owner", "server", "console", "system",
			"gamemaster", "developer", "dev", "staff", "support", "official",
			
			// Drugs (explicit)
			"cocaine", "heroin", "meth", "methhead", "crackhead", "junkie",
			
			// Trolling/meme
			"urmom", "yourmom", "deez", "ligma", "sugma", "bofa", "candice", "wendys",
			"icup", "icdub", "pen15", "8008", "80085",
			
			// Leet speak variants
			"n1gger", "n1gga", "f4ggot", "r3tard", "p3nis", "d1ck", "puss1", "c0ck",
			"sh1t", "b1tch", "fuk", "phuck", "phuk",
			
			// Common bypasses
			"nigg", "fagt", "rtard", "pen1s", "vag1na", "c0ck", "d1ck", "pvss", "azz"
		});
		
		Print(string.Format("[SRZ_RP] Loaded %1 banned words", m_BannedWords.Count()), LogLevel.NORMAL);
	}
	
	// --------------------------------------------------------------------------------------------
	// Build banned combinations list (full names of infamous people)
	protected void BuildBannedCombinationsList()
	{
		m_BannedCombinations.InsertAll({
			// === DICTATORS & WAR CRIMINALS ===
			"adolf hitler", "adolfhitler", "hitler", "hittler", "hitlar",
			"joseph stalin", "josephstalin", "stalin", "staline",
			"benito mussolini", "mussolini",
			"mao zedong", "maozedong", "mao tse", "maotse",
			"pol pot", "polpot",
			"idi amin", "idiamin",
			"saddam hussein", "saddamhussein", "saddam",
			"muammar gaddafi", "gaddafi", "qaddafi", "gadaffi",
			"kim jong", "kimjong", "kim il", "kimil",
			"osama bin", "osamabin", "bin laden", "binladen", "osama",
			"heinrich himmler", "himmler",
			"joseph goebbels", "goebbels",
			"hermann goering", "goering", "goring",
			"rudolf hess", "rudolfhess",
			"slobodan milosevic", "milosevic",
			"radovan karadzic", "karadzic",
			"ratko mladic", "mladic",
			"bashar assad", "basharassad",
			"augusto pinochet", "pinochet",
			"francisco franco", "franco",
			"hideki tojo", "tojo",
			
			// === TERRORISTS ===
			"osama bin laden", "binladen", "bin ladin",
			"abu bakr", "abubakr", "al baghdadi", "albaghdadi",
			"ayman zawahiri", "zawahiri",
			"anwar awlaki", "awlaki",
			"khalid sheikh", "khalidsheikh",
			"mohammed atta", "mohammedatta",
			"anders breivik", "breivik",
			"timothy mcveigh", "mcveigh",
			"ted kaczynski", "kaczynski", "unabomber",
			
			// === SERIAL KILLERS ===
			"ted bundy", "tedbundy", "bundy",
			"jeffrey dahmer", "jeffreydahmer", "dahmer",
			"john gacy", "johngacy", "gacy",
			"charles manson", "charlesmanson", "manson",
			"ed gein", "edgein", "gein",
			"albert fish", "albertfish",
			"andrei chikatilo", "chikatilo",
			"dennis rader", "dennisrader", "btk killer", "btk",
			"richard ramirez", "richardramirez", "nightstalker",
			"david berkowitz", "berkowitz", "son of sam", "sonofsam",
			"aileen wuornos", "wuornos",
			"gary ridgway", "ridgway", "green river", "greenriver",
			"harold shipman", "shipman",
			"jack the ripper", "jacktheripper", "jack ripper",
			"zodiac killer", "zodiac",
			"henry lee lucas", "henrylucas",
			"ivan milat", "milat",
			"peter sutcliffe", "sutcliffe", "yorkshire ripper",
			"fred west", "fredwest", "rose west", "rosewest",
			"luis garavito", "garavito",
			"pedro lopez", "pedrolopez",
			"alexander pichushkin", "pichushkin",
			
			// === MASS SHOOTERS ===
			"adam lanza", "adamlanza",
			"dylann roof", "dylannroof",
			"nikolas cruz", "nikolascruz",
			"stephen paddock", "paddock",
			"james holmes", "jamesholmes",
			"elliot rodger", "rodger", "elliotrodger",
			"eric harris", "ericharris",
			"dylan klebold", "klebold",
			"seung hui", "seunghui", "cho seung",
			"brenton tarrant", "tarrant",
			
			// === CULT LEADERS ===
			"jim jones", "jimjones", "jonestown",
			"david koresh", "davidkoresh", "koresh",
			"marshall applewhite", "applewhite", "heavens gate",
			"shoko asahara", "asahara", "aum shinrikyo",
			"keith raniere", "raniere", "nxivm",
			
			// === CURRENT POLITICAL FIGURES ===
			"vladimir putin", "vladimirputin", "putin", "pootin", "putler",
			"donald trump", "donaldtrump", "trump", "trumpy",
			"joe biden", "joebiden", "biden",
			"barack obama", "barackobama", "obama",
			"hillary clinton", "hillaryclinton",
			"volodymyr zelensky", "zelensky", "zelenskyy",
			"xi jinping", "xijinping", "jinping",
			"emmanuel macron", "macron",
			"boris johnson", "borisjohnson",
			"angela merkel", "merkel",
			"narendra modi", "modi",
			"jair bolsonaro", "bolsonaro",
			"recep erdogan", "erdogan",
			"benjamin netanyahu", "netanyahu", "bibi",
			"ayatollah khamenei", "khamenei", "khomeini",
			
			// === CELEBRITIES (to prevent impersonation) ===
			"elon musk", "elonmusk",
			"jeff bezos", "jeffbezos", "bezos",
			"mark zuckerberg", "zuckerberg", "zuck",
			"bill gates", "billgates",
			"steve jobs", "stevejobs",
			"kanye west", "kanyewest", "kanye",
			"kim kardashian", "kardashian",
			"taylor swift", "taylorswift",
			"justin bieber", "justinbieber", "bieber",
			"eminem", "slim shady", "slimshady",
			"snoop dogg", "snoopdogg",
			"drake",
			"beyonce", "beyoncé",
			"rihanna",
			"lady gaga", "ladygaga",
			"ariana grande", "arianagrande",
			"dwayne johnson", "dwaynejohnson", "the rock", "therock",
			"keanu reeves", "keanureeves",
			"johnny depp", "johnnydepp",
			"leonardo dicaprio", "dicaprio",
			"tom cruise", "tomcruise",
			"brad pitt", "bradpitt",
			"angelina jolie", "angelinajolie",
			"pewdiepie", "pewds",
			"mrbeast", "mr beast",
			"ninja",
			"shroud",
			"dr disrespect", "drdisrespect",
			"xqc",
			"pokimane",
			"asmongold",
			
			// === CONTROVERSIAL FIGURES ===
			"andrew tate", "andrewtate", "tate",
			"alex jones", "alexjones", "infowars",
			"jordan peterson", "jordanpeterson",
			"ben shapiro", "benshapiro",
			"joe rogan", "joerogan",
			"tucker carlson", "tuckercarlson",
			"harvey weinstein", "weinstein",
			"jeffrey epstein", "epstein", "epstien",
			"ghislaine maxwell", "maxwell",
			"r kelly", "rkelly",
			"bill cosby", "billcosby", "cosby",
			"oj simpson", "ojsimpson",
			
			// === FICTIONAL VILLAINS (edgy naming) ===
			"joker",
			"thanos",
			"darth vader", "darthvader", "vader",
			"voldemort",
			"hannibal lecter", "hanniballecter", "lecter",
			"pennywise",
			"freddy krueger", "krueger",
			"jason voorhees", "voorhees",
			"michael myers",
			"leatherface",
			"jigsaw",
			"pinhead",
			"chucky",
			
			// === NAZI/HATE GROUP REFERENCES ===
			"nazi", "naz1", "n4zi",
			"ss officer", "ssofficer", "waffen ss", "waffenss",
			"gestapo",
			"third reich", "thirdreich", "reich",
			"aryan nation", "aryannation", "aryan",
			"kkk", "ku klux", "kuklux", "klan",
			"white power", "whitepower",
			"heil hitler", "heilhitler", "heil", "sieg heil", "siegheil",
			"1488", "14 88", "88", "fourteen words",
			"proud boy", "proudboy",
			"boogaloo",
			"atomwaffen",
			"azov",
			
			// === RELIGIOUS FIGURES (impersonation) ===
			"jesus christ", "jesuschrist", "jesus",
			"god almighty", "godalmighty",
			"allah",
			"muhammad", "mohammed", "mohamed", "prophet muhammad",
			"buddha",
			"satan", "lucifer", "devil", "beelzebub",
			"antichrist"
		});
		
		Print(string.Format("[SRZ_RP] Loaded %1 banned name combinations", m_BannedCombinations.Count()), LogLevel.NORMAL);
	}

	// --------------------------------------------------------------------------------------------
	// Check if name contains banned words
	protected bool ContainsBannedWord(string name, out string foundWord)
	{
		foundWord = "";
		
		// Convert to lowercase for checking
		string lowerName = name;
		lowerName.ToLower();
		
		// Remove spaces and common substitutions for thorough checking
		string compactName = lowerName;
		compactName.Replace(" ", "");
		compactName.Replace("0", "o");
		compactName.Replace("1", "i");
		compactName.Replace("3", "e");
		compactName.Replace("4", "a");
		compactName.Replace("5", "s");
		compactName.Replace("7", "t");
		compactName.Replace("8", "b");
		compactName.Replace("@", "a");
		compactName.Replace("$", "s");
		compactName.Replace("!", "i");
		compactName.Replace("_", "");
		compactName.Replace("-", "");
		compactName.Replace(".", "");
		
		foreach (string banned : m_BannedWords)
		{
			// Check original lowercase name
			if (lowerName.Contains(banned))
			{
				foundWord = banned;
				return true;
			}
			
			// Check compact version (no spaces, substitutions normalized)
			if (compactName.Contains(banned))
			{
				foundWord = banned;
				return true;
			}
		}
		
		return false;
	}
	
	// --------------------------------------------------------------------------------------------
	// Check if name matches a banned combination
	protected bool MatchesBannedCombination(string name, out string foundCombination)
	{
		foundCombination = "";
		
		// Convert to lowercase
		string lowerName = name;
		lowerName.ToLower();
		lowerName.Trim();
		
		// Create compact version (no spaces)
		string compactName = lowerName;
		compactName.Replace(" ", "");
		compactName.Replace("0", "o");
		compactName.Replace("1", "i");
		compactName.Replace("3", "e");
		compactName.Replace("4", "a");
		compactName.Replace("5", "s");
		compactName.Replace("7", "t");
		compactName.Replace("8", "b");
		compactName.Replace("@", "a");
		compactName.Replace("$", "s");
		compactName.Replace("!", "i");
		compactName.Replace("_", "");
		compactName.Replace("-", "");
		compactName.Replace(".", "");
		
		foreach (string banned : m_BannedCombinations)
		{
			// Create compact version of banned name too
			string compactBanned = banned;
			compactBanned.Replace(" ", "");
			
			// Exact match (with spaces)
			if (lowerName == banned)
			{
				foundCombination = banned;
				return true;
			}
			
			// Compact match (no spaces)
			if (compactName == compactBanned)
			{
				foundCombination = banned;
				return true;
			}
			
			// Contains match - name contains the banned combination
			if (lowerName.Contains(banned))
			{
				foundCombination = banned;
				return true;
			}
			
			if (compactName.Contains(compactBanned))
			{
				foundCombination = banned;
				return true;
			}
		}
		
		return false;
	}

	// --------------------------------------------------------------------------------------------
	// Validate a player-submitted name (1-2 words, appropriate length, no banned words)
	bool ValidatePlayerName(string name, out string errorMsg)
	{
		errorMsg = "";
		
		name.Trim();
		
		if (name.IsEmpty())
		{
			errorMsg = "Name cannot be empty.";
			return false;
		}
		
		// Check length
		if (name.Length() < 2 || name.Length() > 32)
		{
			errorMsg = "Name must be 2-32 characters.";
			return false;
		}
		
		// Check for banned words
		string foundBanned;
		if (ContainsBannedWord(name, foundBanned))
		{
			errorMsg = "Name contains inappropriate content.";
			Print(string.Format("[SRZ_RP] Blocked name '%1' - contains banned word", name), LogLevel.WARNING);
			return false;
		}
		
		// Check for banned combinations (celebrities, criminals, etc.)
		string foundCombination;
		if (MatchesBannedCombination(name, foundCombination))
		{
			errorMsg = "This name is not allowed.";
			Print(string.Format("[SRZ_RP] Blocked name '%1' - matches banned combination", name), LogLevel.WARNING);
			return false;
		}
		
		// Count words (split by spaces)
		array<string> words = new array<string>();
		name.Split(" ", words, true);
		
		// Remove empty entries
		array<string> nonEmptyWords = new array<string>();
		foreach (string word : words)
		{
			word.Trim();
			if (!word.IsEmpty())
				nonEmptyWords.Insert(word);
		}
		
		int wordCount = nonEmptyWords.Count();
		
		if (wordCount == 0)
		{
			errorMsg = "Name cannot be empty.";
			return false;
		}
		
		if (wordCount > 2)
		{
			errorMsg = "Name must be 1-2 words only (nickname OR firstname lastname).";
			return false;
		}
		
		// Check each word has reasonable length
		foreach (string word : nonEmptyWords)
		{
			if (word.Length() < 2)
			{
				errorMsg = "Each word must be at least 2 characters.";
				return false;
			}
		}
		
		// Basic character validation (letters, numbers, basic punctuation)
		for (int i = 0; i < name.Length(); i++)
		{
			string ch = name.Get(i);
			int ascii = ch.ToAscii();
			
			// Allow: letters (A-Z, a-z), numbers (0-9), space, hyphen, apostrophe
			bool isValid = 
				(ascii >= 65 && ascii <= 90) ||   // A-Z
				(ascii >= 97 && ascii <= 122) ||  // a-z
				(ascii >= 48 && ascii <= 57) ||   // 0-9
				(ascii == 32) ||                   // space
				(ascii == 45) ||                   // hyphen
				(ascii == 39);                     // apostrophe
			
			if (!isValid)
			{
				errorMsg = "Name contains invalid characters. Use only letters, numbers, spaces, hyphens, and apostrophes.";
				return false;
			}
		}
		
		return true;
	}

	// --------------------------------------------------------------------------------------------
	// Generate a random name (public for commands and components)
	string GenerateRandomName(int playerId)
	{
		// Check if name pools are populated
		if (m_FirstNames.Count() == 0 || m_LastNames.Count() == 0)
		{
			Print("[SRZ_RP] ERROR: Name pools are empty! Rebuilding...", LogLevel.ERROR);
			BuildNamePools();
		}
		
		// Final safety check
		if (m_FirstNames.Count() == 0 || m_LastNames.Count() == 0)
		{
			Print("[SRZ_RP] CRITICAL ERROR: Name pools failed to build!", LogLevel.ERROR);
			return string.Format("ERROR_PLAYER_%1", playerId);
		}

		// Generate name from pool
		string firstName = m_FirstNames.GetRandomElement();
		string lastName = m_LastNames.GetRandomElement();

		string fullName = string.Format("%1 %2", firstName, lastName);
		Print(string.Format("[SRZ_RP] Generated name for player %1: %2", playerId, fullName), LogLevel.NORMAL);
		
		return fullName;
	}

	// --------------------------------------------------------------------------------------------
	// Apply RP name to character
	void ForceApplyToCharacter(int playerId, string rpName)
	{
		if (!Replication.IsServer())
			return;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;

		SCR_PlayerController pc = SCR_PlayerController.Cast(pm.GetPlayerController(playerId));
		if (!pc)
			return;

		// Store in profile manager
		SRZ_RPNameProfileManager profileMgr = SRZ_RPNameProfileManager.GetInstance();
		if (profileMgr)
		{
			profileMgr.SetNameForPlayer(playerId, rpName);
		}
		
		// Apply to character if spawned
		IEntity entity = pc.GetMainEntity();
		if (entity)
		{
			SRZ_RPNameCharacterComponent charComp = SRZ_RPNameCharacterComponent.Cast(
				entity.FindComponent(SRZ_RPNameCharacterComponent)
			);
			
			if (charComp)
			{
				charComp.UpdateRPName(rpName);
			}
		}
	}

	// --------------------------------------------------------------------------------------------
	// Build name pools
	protected void BuildNamePools()
	{
		// === FIRST NAMES (COMBINED) ===
		m_FirstNames.InsertAll({
			// Originally Bandit
			"Tyomka","Vityuha","Vasyan","Lyoha","Kostyan","Mihas","Arkasha","Petruha","Vetal","Bodya",
			"Yashka","Dimuha","Miha","Tolay","Toha","Pavluha","Kolyan","Fimka","Zhorka","Styopka",
			"Zheka","Sanyok","Grishka","Stepuha","Yuras","Dimon","Mitka","Boryan","Grishko","Shurik",
			"Ilyuha","Vadya","Andryuha","Felka","Vasek","Timoha","Mihal","Vovan",
			// Originally Stalker
			"Slava","Seryoga","Tima","Valera","Sevka","Pashka","Mishka","Vanka","Fima","Roma",
			"Max","Petro","Grisha","Gleb","Misha","Matvey","Rus","Genka","Edik",
			"Slavik","Vadim","Kolya","Dimka","German","Danya","Vasya","Fedka","Romka",
			"Danko","Tolik","Ilya","Fedya","Arthur","Vadik","Zhora","Filka",
			"Zhenka","Sava","Senya","Gena","Egor","Yurko","Valik","Petya","Yasha",
			"Vlad","Danila","Fyodor","Stepa","Semyon","Syoma","Vitka","Sanya","Yara","Yashka",
			"Rostik","Lyoshka","Dima","Vanya","Vitalik","Gosha","Nik","Kostik","Seva","Kostya",
			"Toshka","Borya","Vova","Anton","Yurka","Vitya","Lyonya","Nikita","Dmitro","Artyom",
			"Oleg","Vasko","Lyova","Filya","Mitya","Denis","Timka","Bogdan","Lenka","Borka",
			"Alex","Petka","Venya","Yarik","Stepan","Zhenya","Igoryok","Georg","Vovka","Vaska",
			"Pasha","Yury","Egorka"
		});

		// === LAST NAMES (COMBINED) ===
		m_LastNames.InsertAll({
			// Originally Bandit
			"Tambourine","Corpse","Nothing","Spy","Arsonist","One-eyed","Carp","Punk","Tightwad",
			"Perp","Maniac","Beast","Squeegee","Fierce","Partisan","Cross","Slasher","Kaschey",
			"Dumpling","Bourgeois","Boa","Thief","Hipster","Flamen","Wrinkled","Surgeon","Block",
			"Feather","Rogue","Muddy","Glitch","Parasite","Aesthete","Canary","Moor","Graph",
			"Baton","Blind","Knuckles","Fragged","Attorney","Pirate","Joker","Leary","Hook",
			"Drip","Percher","Cracker","Grater","Artist","Conman","Gagster","Fog","Batiy",
			"Bum","Chucker","Pimp","Prick","Bustard","Dummy","Bull","Razor","Bud","Rotten",
			"Susanin","Frantic","Cripple","Beaver","Snot","Buffalo","Teddy","Despot","Sparrow",
			"Chemist","Hatrack","Grudge","Gray","Jiggers","Messenger","Hangman","Caveman","Monster",
			"Banker","Gravedigger","Leshiy","Dynamite","Genghis","Chapay","Worm","Blimp","Shaved",
			"Pusher","Fist","Mayhem","Bolt","Chisel","Choker","Gangrene","Snake","Holey","Gentile",
			"Pintle","Jack","Ace","Bulldog","Crowbar","Ninja","Death","Drill","Transporter","Stout",
			"Ragged","Jackal","Grabber","Ghoul","Loony","Chronic","Prince","Local","Decadent",
			"Boatswain","Grinder","Dolittle","Patsyuk","Gaffer","Grad","Zmur","Major","Moose",
			"Bedbug","Accordion","Spliff","Corner-cutter","Capital","Lamer","Nickel","Godfather",
			"Beaten","Boar","Coachman","Butcher","Baldy","Banana","Pacifist","Tiranas","Tails",
			"Legalist","Burnt","Hunchback","Yakker","Boss-eyed","Outcast","Vagabond",
			"Piston","Pig","Bummer","Bourbon","Tramp","Chill","Long","Pepper","Panadol",
			"Cranky","Baron","Woodpecker","Tie","Barmaley","Player","Brigand","Rambo","Eared",
			"Terminator","Awl","Spam",
			// Originally Stalker
			"Friday","Commodore","Sharp","Little Man","Piranha","Punk","Boss","Squint-eyed",
			"Spider","Owl","Claw","Sorcerer","Butters","Wrinkled","Shaggy","Dancer","Prometheus",
			"Fireball","Vandal","Crocodile","Blind","Glutton","Boulder","Hook","Drip","Whirlpool",
			"Muskrat","Artist","White","Homer","Whiner","Bull","Rook","Pilgrim","Beetle","Teddy",
			"Skiff","Despot","Hangman","Comatose","Caveman","Sultan","Green","Nerve","Leshiy",
			"Kagor","Pusher","Turkey","Goose","Important","Deserter","Death","Reverend","Warrior",
			"Romantic","Ghoul","Loony","Chronic","Prince","Professor","Moose","Bedbug","Wind",
			"Tarantula","Softy","Pastor","Colonel","Aviator","Gnat","Docent","Boar","Healer",
			"Padishah","Outcast","Gloomy","Vagabond","Piston","Fatty","Sniper","Cleaner","Backbone",
			"Magnate","Baron","Operator","Woodpecker","Barmaley","Phantom","Mechanic","Hog","Spy",
			"Vacuum","Perp","Hood","Nozzle","Invincible","Armorer","Luckster","Prophet","Rusty",
			"Tourist","Cosmos","Coffin","Hipster","Scholar","Paranoid","Smoker","Knuckles","Pirate",
			"Trap","Ice","Cossack","Whip","Master","Goblin","Dealer","Rocket","Bitter","Rotten",
			"Stranger","Cripple","Chance","Favorite","Flint","Deputy","Monster","Bayonet","Garlic",
			"Grenade","Cat","Bulldog","Ninja","Motor","Drill","Trigger","Jackal","Grabber","Poet",
			"Decadent","Rocker","Karma","Head","Neptune","Major","Whiz","Stern","Cider","Buffoon",
			"Heartthrob","Dude","Carbide","Narcissus","Godfather","Beaten","Agent","Guest","Coachman",
			"Banana","Tails","Yakker","Cynic","Bubble","Lord","Courier","Skinny","Long",
			"Law","Pepper","Chill","Panadol","Hybrid","Native","Brave","Tie","Player","Ferret",
			"Pilot","Mutant","Cobra","Terminator","Tambourine","Iron","Dinosaur","Fireman","Raven",
			"Gremlin","Pianist","Chopper","Black","Cross","Slasher","Postman","Surgeon","Feather",
			"Gunpowder","Oldtimer","Glitch","Comb","Martian","Canary","Rabbit","Shaman","Mountaineer",
			"Conman","Immortal","Voland","Isotope","Dolphin","Razor","Brick","Susanin","Frantic",
			"Mammoth","Viking","Bureaucrat","Doctor","Chemist","Dogman","Captain","Messenger",
			"Dynamite","Worm","Bundle","Dry","Choker","Bolt","Chisel","Bard","Lock","Microbe",
			"Porter","Ace","Electrician","Transporter","Stout","Ragged","Secretary","Local",
			"Boatswain","Vulture","Hero","Gladiator","Smartass","Buddy","Ascetic","Cross-eyed",
			"Icarus","Lamer","Visitor","Nickel","Anaconda","Blessed","Butcher","Root","King",
			"Loader","Legalist","Burnt","Nocturnal","Bourbon","Bummer","Quack","Chilly","Rambo",
			"Pumpkin","Carpenter","Spam","Corpse","Locksmith","Stone","Indian","Frame","Beast",
			"Shrimp","Partisan","Barbarian","Accountant","Fang","Kaschey","Dumpling","Thief",
			"Ambassador","Flamen","Rogue","Muddy","Forester","Clown","Mason","Footballer","Bone",
			"Traveler","Aesthete","Caesar","Baton","Attorney","Moped","Dock","Leary",
			"Geologist","Ranger","Horse","Paddle","Machine","Grater","Athlete","Madera",
			"Ventilator","Witcher","Bustard","Robot","Target","Quiet","Tanker","Diplomat",
			"Skeleton","Parachute","Axe","Miner","Scythe","Casanova","Sparrow","Dandy","Marmot",
			"Gramps","Gray","Kutuzov","Bear","Clumsy","Banker","Gravedigger","Cursor",
			"Fist","Mayhem","Falcon","Telegraphist","Priest","Apostle","Dad","Gentile","Samurai",
			"Snake","Spirit","Jack","Lantern","Loser","Crowbar","Carrier","Cockroach","Thorn",
			"Ant","Trailblazer","Reactor","Loyal","Wrangler","Grinder","Sheriff","Hireling","Pagan",
			"Grad","Hoary","Gaffer","Zmur","Red","Capital","Pipe","Simpleton","Boot","Student",
			"Anomaly","Pacifist","Baldy","Diver","Hunchback","Merchant","Driver",
			"Cabbage","Beekeeper","Cranky","Dexter","Stooped","Lucky","Gorynich","Awl"
		});
	}
}