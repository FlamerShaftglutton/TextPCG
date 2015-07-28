#include "src/Scripting.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Object
{
	bool visible;
	bool visible_in_short_description;
	bool friendly;
	bool mobile;
	bool playable;
	
	int hitpoints;
	int attack;
	float hit_chance;
	
	std::string description;
	std::string name;
	
	Object()
	{
		visible = false;
		visible_in_short_description = false;
		friendly = false;
		mobile = false;
		playable = false;
		
		hitpoints = 0;
		attack = 0;
		hit_chance = 0.0f;
		
		description = "";
		name = "";
	}
	
	void fill_ObjectMap(ObjectMap& o)
	{
		o.visible = &visible;
		o.visible_in_short_description = &visible_in_short_description;
		o.friendly = &friendly;
		o.mobile = &mobile;
		o.playable = &playable;
		
		o.hitpoints = &hitpoints;
		o.attack = &attack;
		o.hit_chance = &hit_chance;
		
		o.description = &description;
		o.name = &name;
	}
};

void print_sv(ScriptingVariables& sv)
{
	std::cout << "\n\nMain Text: " << *(sv.main_text)
			  << "\n\nPlayer: "
			  << "\n\tVisible: " << *(sv.player.visible)
			  << "\n\tVisible in short description: " << *(sv.player.visible_in_short_description)
			  << "\n\tFriendly: " << *(sv.player.friendly)
			  << "\n\tMobile: " << *(sv.player.mobile)
			  << "\n\tPlayable: " << *(sv.player.playable)
			  << "\n\tHitpoints: " << *(sv.player.hitpoints)
			  << "\n\tAttack: " << *(sv.player.attack)
			  << "\n\tHit Chance: " << *(sv.player.hit_chance)
			  << "\n\tDescription: " << *(sv.player.description)
			  << "\n\tName: " << *(sv.player.name)
			  << "\n\nCaller: "
			  << "\n\tVisible: " << *(sv.caller.visible)
			  << "\n\tVisible in short description: " << *(sv.caller.visible_in_short_description)
			  << "\n\tFriendly: " << *(sv.caller.friendly)
			  << "\n\tMobile: " << *(sv.caller.mobile)
			  << "\n\tPlayable: " << *(sv.caller.playable)
			  << "\n\tHitpoints: " << *(sv.caller.hitpoints)
			  << "\n\tAttack: " << *(sv.caller.attack)
			  << "\n\tHit Chance: " << *(sv.caller.hit_chance)
			  << "\n\tDescription: " << *(sv.caller.description)
			  << "\n\tName: " << *(sv.caller.name)
			  << "\n\nCurrent_Room: "
			  << "\n\tDescription: " << *(sv.current_room.description)
			  << "\n\tShort Description: " << *(sv.current_room.short_description)
			  << "\n\tMinimap Symbol: " << *(sv.current_room.minimap_symbol);
}

int main()
{
	std::ifstream infile("expressions.txt");
	
	ScriptingVariables sv;
	std::string main_text = "";
	sv.main_text = &main_text;
	std::string description = "A dank, smelly arena. Will this place test you, or will you test it?";
	sv.current_room.description = &description;
	std::string short_description = "An arena";
	sv.current_room.short_description = &short_description;
	std::string minimap_symbol = "<fg=yellow><bg=yellow> ";
	sv.current_room.minimap_symbol = &minimap_symbol;
	
	Object o;
	o.visible = true;
	o.visible_in_short_description = true;
	o.friendly = true;
	o.mobile = true;
	o.playable = true;
	o.hitpoints = 100;
	o.attack = 10;
	o.hit_chance = 0.25f;
	o.description = "An ugly humanoid. It's hideous!";
	o.name = "You";
	
	ObjectMap om;
	o.fill_ObjectMap(om);
	
	sv.current_room.objects.push_back(om);
	sv.player = om;
	
	o.playable = false;
	o.hitpoints = 11;
	o.description = "A somewhat short man in a dark gray cloak. He mutters to himself while eyeing you.";
	o.name = "Mysterious Stranger";
	o.fill_ObjectMap(om);
	sv.current_room.objects.push_back(om);
	sv.caller = om;
	
	std::string all_text = "";
	while (!infile.eof())
	{
		std::string line;
		std::getline(infile,line);
		all_text += line + "\n";
	}
	
	ScriptSet ss;
	ss.construct("",all_text,"");
	
	//print_sv(sv);
	
	ss.execute_on_sight(sv);
	
	std::cout << main_text << std::endl;
	
	//print_sv(sv);
	
	return 0;
}